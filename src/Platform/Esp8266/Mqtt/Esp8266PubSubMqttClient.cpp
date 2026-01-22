#ifdef ESP8266

#include "Esp8266PubSubMqttClient.h"

// Se você quiser evitar "global", dá pra usar singleton interno.
// Aqui vamos usar um ponteiro estático simples para o thunk.
static iotsmartsys::platform::esp8266::Esp8266PubSubMqttClient *g_self = nullptr;

namespace iotsmartsys::platform::esp8266
{
    Esp8266PubSubMqttClient::Esp8266PubSubMqttClient(iotsmartsys::core::ILogger &log)
        : _logger(log),
          _mqtt(_netPlain) // vamos trocar o client em begin() se precisar
    {
        g_self = this;
    }

    bool Esp8266PubSubMqttClient::parseUri(const char *uri, ParsedUri &out)
    {
        if (!uri)
            return false;

        const char *p = uri;
        out = ParsedUri{};

        // scheme
        const bool isMqtts = (std::strncmp(p, "mqtts://", 8) == 0);
        const bool isMqtt = (std::strncmp(p, "mqtt://", 7) == 0);
        if (!isMqtts && !isMqtt)
            return false;

        out.tls = isMqtts;
        p += isMqtts ? 8 : 7;

        // host até ':' ou '/' ou fim
        const char *hostStart = p;
        while (*p && *p != ':' && *p != '/')
            ++p;
        if (p == hostStart)
            return false;

        out.host.assign(hostStart, (size_t)(p - hostStart));

        // port opcional
        if (*p == ':')
        {
            ++p;
            uint32_t port = 0;
            while (*p >= '0' && *p <= '9')
            {
                port = port * 10u + (uint32_t)(*p - '0');
                ++p;
            }
            if (port == 0 || port > 65535)
                return false;
            out.port = (uint16_t)port;
        }
        else
        {
            out.port = out.tls ? 8883 : 1883;
        }

        return true;
    }

    void Esp8266PubSubMqttClient::applyTlsDefaults()
    {
        // Começo simples e robusto: não valida CA (não depende de RTC/NTP)
        // Depois você evolui para fingerprint/CA via settings.
        _netSecure.setInsecure();
        // Opcional: reduzir chance de travar em rede ruim
        _netSecure.setTimeout(8);
    }

    bool Esp8266PubSubMqttClient::begin(const iotsmartsys::core::TransportConfig &cfg)
    {
        _cfg = cfg;

        ParsedUri u;
        if (!parseUri(cfg.uri, u))
        {
            _logger.warn("MQTT", "ESP8266 IMqttClient: invalid uri='%s'", cfg.uri ? cfg.uri : "(null)");
            return false;
        }

        _useTls = u.tls;
        _hostStr = u.host;
        _port = u.port;

        _clientIdStr = cfg.clientId ? cfg.clientId : "";
        _userStr = cfg.username ? cfg.username : "";
        _passStr = cfg.password ? cfg.password : "";
        _subTopicStr = cfg.subscribeTopic ? cfg.subscribeTopic : "";
        _pubTopicStr = cfg.publishTopic ? cfg.publishTopic : "";

        if (_useTls)
        {
            applyTlsDefaults();
            _mqtt.setClient(_netSecure);
        }
        else
        {
            _mqtt.setClient(_netPlain);
        }

        // Ajustes “safe defaults”
        _mqtt.setServer(_hostStr.c_str(), _port);
        _mqtt.setKeepAlive(cfg.keepAliveSec ? cfg.keepAliveSec : 30);
        _mqtt.setSocketTimeout(6); // evita ficar travado demais
        _mqtt.setBufferSize(1024);
        _mqtt.setCallback(&Esp8266PubSubMqttClient::onPubSubMessageThunk);

        _connected = false;

        _logger.info("MQTT", "ESP8266 IMqttClient ready: tls=%d host=%s port=%u clientId=%s",
                     (int)_useTls, _hostStr.c_str(), (unsigned)_port, _clientIdStr.c_str());

        return true;
    }

    void Esp8266PubSubMqttClient::start()
    {
        if (_mqtt.connected())
        {
            _connected = true;
            return;
        }

        if (_clientIdStr.empty())
        {
            _logger.warn("MQTT", "ESP8266 start(): clientId is empty");
        }

        // PubSubClient: connect(..., cleanSession) só existe no overload com Will.
        const bool ok = _mqtt.connect(
            _clientIdStr.c_str(),
            _userStr.empty() ? nullptr : _userStr.c_str(),
            _passStr.empty() ? nullptr : _passStr.c_str(),
            nullptr, 0, false, nullptr,
            _cfg.cleanSession);

        if (ok)
        {
            _connected = true;

            // Sub padrão (se tiver)
            if (!_subTopicStr.empty())
            {
                _mqtt.subscribe(_subTopicStr.c_str());
            }

            if (_onConnected)
            {
                iotsmartsys::core::TransportConnectedView info{};
                info.broker = _hostStr.c_str();
                info.clientId = _clientIdStr.c_str();
                info.keepAliveSec = _cfg.keepAliveSec;
                _onConnected(_onConnectedUser, info);
            }

            _logger.info("MQTT", "ESP8266 connected");
        }
        else
        {
            _connected = false;
            _logger.warn("MQTT", "ESP8266 connect failed (state=%d)", _mqtt.state());
        }
    }

    void Esp8266PubSubMqttClient::stop()
    {
        if (_mqtt.connected())
        {
            _mqtt.disconnect();
        }

        const bool wasConnected = _connected;
        _connected = false;

        if (wasConnected && _onDisconnected)
        {
            _onDisconnected(_onDisconnectedUser);
        }
    }

    void Esp8266PubSubMqttClient::handle()
    {
        const bool nowConnected = _mqtt.connected();

        if (_connected && !nowConnected)
        {
            _connected = false;
            if (_onDisconnected)
                _onDisconnected(_onDisconnectedUser);
        }

        // loop precisa rodar mesmo “quase sempre”
        _mqtt.loop();

        if (!_connected && _mqtt.connected())
        {
            _connected = true;
            if (_onConnected)
            {
                iotsmartsys::core::TransportConnectedView info{};
                _onConnected(_onConnectedUser, info);
            }
        }
    }

    bool Esp8266PubSubMqttClient::isConnected() const
    {
        return _mqtt.connected();
    }

    bool Esp8266PubSubMqttClient::publish(const char *topic, const void *payload, std::size_t len, bool retain)
    {
        if (!topic || !payload)
            return false;
        return _mqtt.publish(topic, (const uint8_t *)payload, (unsigned int)len, retain);
    }

    bool Esp8266PubSubMqttClient::republish(const iotsmartsys::core::TransportMessageView &msg)
    {
        return publish(msg.topic, msg.payload, msg.payloadLen, msg.retain);
    }

    bool Esp8266PubSubMqttClient::subscribe(const char *topic)
    {
        if (!topic || !*topic)
            return false;
        return _mqtt.subscribe(topic);
    }

    void Esp8266PubSubMqttClient::setOnMessage(iotsmartsys::core::TransportOnMessageFn cb, void *user)
    {
        _onMsg = cb;
        _onMsgUser = user;
    }

    void Esp8266PubSubMqttClient::setOnConnected(iotsmartsys::core::TransportOnConnectedFn cb, void *user)
    {
        _onConnected = cb;
        _onConnectedUser = user;
    }

    void Esp8266PubSubMqttClient::setOnDisconnected(iotsmartsys::core::TransportOnDisconnectedFn cb, void *user)
    {
        _onDisconnected = cb;
        _onDisconnectedUser = user;
    }

    void Esp8266PubSubMqttClient::onPubSubMessageThunk(char *topic, uint8_t *payload, unsigned int length)
    {
        if (g_self)
            g_self->onPubSubMessage(topic, payload, length);
    }

    void Esp8266PubSubMqttClient::onPubSubMessage(char *topic, uint8_t *payload, unsigned int length)
    {
        if (!_onMsg)
            return;

        iotsmartsys::core::TransportMessageView msg{};
        msg.topic = topic;
        msg.payload = reinterpret_cast<const char *>(payload);
        msg.payloadLen = (std::size_t)length;
        msg.retain = false; // PubSubClient não expõe retain no callback

        _onMsg(_onMsgUser, msg);
    }

} // namespace iotsmartsys::platform::esp8266

#endif
