#ifdef ESP32
#include "EspIdfMqttClient.h"
#include <cstring>

namespace iotsmartsys::platform::espressif
{
    EspIdfMqttClient::EspIdfMqttClient(iotsmartsys::core::ILogger &log)
        : _logger(log)
    {
    }

    EspIdfMqttClient::~EspIdfMqttClient()
    {
        if (_client)
        {
            esp_mqtt_client_stop(_client);
            esp_mqtt_client_destroy(_client);
            _client = nullptr;
        }
    }

    bool EspIdfMqttClient::begin(const iotsmartsys::core::MqttConfig &cfg)
    {
        _logger.info("[MQTT DBG] EspIdfMqttClient::begin()...");
        if (_client)
            return true;

        _logger.info("[MQTT DBG] Preparing esp_mqtt_client_config_t...");

        esp_mqtt_client_config_t c{};
        c.uri = cfg.uri;
        c.client_id = cfg.clientId;
        c.username = cfg.username;
        c.password = cfg.password;
        c.keepalive = cfg.keepAliveSec;
        c.disable_clean_session = cfg.cleanSession ? 0 : 1;
        _logger.debug("[MQTT DBG] Config: uri='%s' client_id='%s' username='%s' keepalive=%d clean_session=%d",
                           c.uri ? c.uri : "(null)",
                           c.client_id ? c.client_id : "(null)",
                           c.username ? c.username : "(null)",
                           c.keepalive,
                           c.disable_clean_session);

        _logger.info("[MQTT DBG] esp_mqtt_client_init()...");
        _client = esp_mqtt_client_init(&c);
        if (!_client) {
            _logger.error("[MQTT DBG] esp_mqtt_client_init() returned NULL");
            return false;
        }

        _logger.info("[MQTT DBG] esp_mqtt_client_register_event()...");
        esp_err_t r = esp_mqtt_client_register_event(_client, MQTT_EVENT_ANY,
                       (esp_event_handler_t)&EspIdfMqttClient::eventHandlerBridge,
                       this);
        if (r != ESP_OK) {
            _logger.error("[MQTT DBG] register_event failed: %d\n", (int)r);
            // continue but warn
        } else {
            _logger.info("[MQTT DBG] register_event OK");
        }

        return true;
    }

    void EspIdfMqttClient::start()
    {
        if (_client)
            esp_mqtt_client_start(_client);
    }

    void EspIdfMqttClient::stop()
    {
        if (_client)
            esp_mqtt_client_stop(_client);
        _connected = false;
    }

    bool EspIdfMqttClient::isConnected() const
    {
        return _connected;
    }

    bool EspIdfMqttClient::publish(const char *topic, const void *payload, std::size_t len, bool retain)
    {
        if (!_client)
            return false;
        // qos=0
        int msg_id = esp_mqtt_client_publish(_client, topic, (const char *)payload, (int)len, 0, retain ? 1 : 0);
        return msg_id >= 0;
    }

    bool EspIdfMqttClient::subscribe(const char *topic)
    {
        if (!_client)
            return false;
        int msg_id = esp_mqtt_client_subscribe(_client, topic, 0);
        return msg_id >= 0;
    }

    void EspIdfMqttClient::setOnMessage(iotsmartsys::core::MqttOnMessageFn cb, void *user)
    {
        _onMsg = cb;
        _onMsgUser = user;
    }
    void EspIdfMqttClient::setOnConnected(iotsmartsys::core::MqttOnConnectedFn cb, void *user)
    {
        _onConnected = cb;
        _onConnectedUser = user;
    }

    void EspIdfMqttClient::setOnDisconnected(iotsmartsys::core::MqttOnDisconnectedFn cb, void *user)
    {
        _onDisconnected = cb;
        _onDisconnectedUser = user;
    }

    esp_err_t EspIdfMqttClient::eventHandlerThunk(esp_mqtt_event_handle_t event)
    {
        auto *self = static_cast<EspIdfMqttClient *>(event->user_context);
        return self ? self->onEvent(event) : ESP_OK;
    }

    esp_err_t EspIdfMqttClient::eventHandlerBridge(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        // event_data is actually an esp_mqtt_event_handle_t when called by the mqtt client
        esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
        if (!event)
            return ESP_OK;

        // ensure user_context inside the event points to our object (handler_arg)
        event->user_context = handler_arg;

        return EspIdfMqttClient::eventHandlerThunk(event);
    }

    esp_err_t EspIdfMqttClient::onEvent(esp_mqtt_event_handle_t event)
    {
        switch (event->event_id)
        {
        case MQTT_EVENT_CONNECTED:
            _connected = true;
            if (_onConnected)
                _onConnected(_onConnectedUser);
            break;
        case MQTT_EVENT_DISCONNECTED:
            _connected = false;
            if (_onDisconnected)
                _onDisconnected(_onDisconnectedUser);
            break;
        case MQTT_EVENT_DATA:
        {
            if (_onMsg)
            {
                // topic/payload não são null-terminated!
                // você pode passar como view (len) e deixar o Router interpretar.
                iotsmartsys::core::MqttMessageView mv{};
                mv.topic = event->topic; // cuidado: não é NUL-terminated, mas dá pra usar com len se quiser copiar
                mv.payload = event->data;
                mv.payloadLen = (std::size_t)event->data_len;
                mv.retain = event->retain;
                _onMsg(_onMsgUser, mv);
            }
            break;
        }
        default:
            break;
        }
        return ESP_OK;
    }

} // namespace iotsmartsys::platform::esp32
#endif
