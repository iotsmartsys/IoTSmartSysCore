#include "Contracts/Connections/WiFiManager.h"
#include "Contracts/Connectivity/ConnectivityGate.h"

namespace iotsmartsys::core
{

#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
    static constexpr uint32_t kDisconnectDebounceMs = 2000;
#endif

    WiFiManager::WiFiManager(iotsmartsys::core::ILogger &log)
        : _log(log), _timeProvider(nullptr)
    {
    }

    void WiFiManager::begin(const WiFiConfig &cfg)
    {
        _timeProvider = &iotsmartsys::core::Time::get();

        _cfg = cfg;
        _attempt = 0;
        _gotIp = false;
        _state = WiFiState::Idle;
        _nextActionAtMs = 0;
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
        _disconnectSinceMs = 0;
#endif

        // garante que o estado latched de conectividade começa limpo
        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                           iotsmartsys::core::ConnectivityGate::IP_READY |
                           iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
        }

#ifdef ESP32
        _eventId = WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t)
                                { this->onWiFiEvent(event); });
#endif

        WiFi.mode(WIFI_STA);
        // WiFi.persistent(_cfg.persistent);

#ifdef ESP32
        WiFi.setAutoReconnect(_cfg.autoReconnect);
#endif

        startConnect();
    }

    bool WiFiManager::isConnected() const
    {
#ifdef ESP32
        return _gotIp && WiFi.status() == WL_CONNECTED;
#else
        if (WiFi.localIP() != IPAddress(0, 0, 0, 0))
        {
            return true;
        }
        return WiFi.isConnected();
#endif
    }

    WiFiState WiFiManager::currentState() const
    {
        return _state;
    }

    void WiFiManager::handle()
    {
        const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());

        switch (_state)
        {
        case WiFiState::Idle:
            // nada
            break;

        case WiFiState::Connecting:
            if (isConnected())
            {
                _state = WiFiState::Connected;
                _connectedAtMs = now;
                _attempt = 0;
                _ssid = WiFi.SSID().c_str();
                _macAddress = WiFi.macAddress().c_str();
                _ipAddress = WiFi.localIP().toString().c_str();
                _signalStrength = String(WiFi.RSSI()).c_str();
                _log.info("WIFI", "Connected. IP=%s", _ipAddress.c_str());
                // Signal the connectivity gate for non-ESP32 platforms as well
                {
                    auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                    gate.setBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                                 iotsmartsys::core::ConnectivityGate::IP_READY);
                    // Debug: log current gate bits after setting

                    _log.debug("WIFI", "ConnectivityGate bits after set = 0x%08x", gate.bits());
                }
            }
            else if (_nextActionAtMs != 0 && now >= _nextActionAtMs)
            {
                scheduleRetry();
            }
            break;

        case WiFiState::BackoffWaiting:
            if (now >= _nextActionAtMs)
                startConnect();
            break;

        case WiFiState::Connected:
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
            if (WiFi.status() != WL_CONNECTED)
            {
                if (_disconnectSinceMs == 0)
                {
                    _disconnectSinceMs = now;
                }

                if ((now - _disconnectSinceMs) < kDisconnectDebounceMs)
                {
                    break;
                }
            }
            else
            {
                _disconnectSinceMs = 0;
            }
#endif
            if (!isConnected())
            {
                // evita ficar reconectando freneticamente se a rede está instável
                if (now - _connectedAtMs < _cfg.reconnectMinUptimeMs)
                {
                    _log.warn("WIFI", "Flapping detected; delaying reconnect");
                }
                // Clear connectivity bits so other components know network is not ready
                {
                    auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                    gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                                   iotsmartsys::core::ConnectivityGate::IP_READY |
                                   iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
                    // Debug: log current gate bits after clear
                    _log.debug("WIFI", "ConnectivityGate bits after clear = 0x%08x", gate.bits());
                }
                scheduleRetry();
            }
            break;
        }
    }

    void WiFiManager::startConnect()
    {
        if (!_cfg.ssid || !_cfg.ssid[0])
        {
            _log.error("WIFI", "SSID not set");
            return;
        }

        _gotIp = false;

        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                           iotsmartsys::core::ConnectivityGate::IP_READY |
                           iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
        }

        _state = WiFiState::Connecting;

        // timeout “soft”: se passar, entra em retry (sem bloquear)
        const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());
        _nextActionAtMs = now + 5000; // 5s

        _log.info("WIFI", "Connecting to SSID=%s (attempt=%lu)", _cfg.ssid, (unsigned long)(_attempt + 1));

        WiFi.disconnect(true); // limpa estado anterior
        WiFi.begin(_cfg.ssid, _cfg.password);
    }

    void WiFiManager::scheduleRetry()
    {
        _attempt++;

        const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());
        const uint32_t backoff = computeBackoffMs();

        _nextActionAtMs = now + backoff;
        _state = WiFiState::BackoffWaiting;

        _log.warn("WIFI", "Retry in %lu ms (attempt=%lu)", (unsigned long)backoff, (unsigned long)_attempt);
    }

    uint32_t WiFiManager::computeBackoffMs() const
    {
        uint32_t base = _cfg.initialBackoffMs;

        uint8_t exp = (_attempt <= _cfg.maxFastRetries) ? _attempt : _cfg.maxFastRetries;
        for (uint8_t i = 0; i < exp; ++i)
        {
            if (base > (_cfg.maxBackoffMs / 2))
            {
                base = _cfg.maxBackoffMs;
                break;
            }
            base *= 2;
        }
        if (base > _cfg.maxBackoffMs)
            base = _cfg.maxBackoffMs;

        uint32_t jitter = 0;
        if (_cfg.jitterMs)
        {
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
            // ESP8266: use SDK random generator
            jitter = (uint32_t)(os_random() % (_cfg.jitterMs + 1));
#else
            jitter = (uint32_t)(esp_random() % (_cfg.jitterMs + 1));
#endif
        }

        return base + jitter;
    }

#ifdef ESP32
    void WiFiManager::onWiFiEvent(WiFiEvent_t event)
    {
        auto &gate = iotsmartsys::core::ConnectivityGate::instance();

        switch (event)
        {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            gate.setBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED);
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            _gotIp = true;

            gate.setBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                         iotsmartsys::core::ConnectivityGate::IP_READY);
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            _gotIp = false;

            gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                           iotsmartsys::core::ConnectivityGate::IP_READY |
                           iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
            break;

        default:
            break;
        }
    }
#endif

    std::vector<std::string> WiFiManager::getAvailableSSIDs()
    {
        std::vector<std::string> ssids;

        int n = WiFi.scanNetworks();
        if (n <= 0)
        {
            _log.info("WIFI", "No networks found");
            return ssids;
        }

        for (int i = 0; i < n; ++i)
        {
            ssids.push_back(WiFi.SSID(i).c_str());
        }

        WiFi.scanDelete(); // limpa resultados do scan

        return ssids;
    }

    const char *WiFiManager::getSsid() const
    {
        return _ssid.c_str();
    }

    const char *WiFiManager::getIpAddress() const
    {
        return _ipAddress.c_str();
    }

    const char *WiFiManager::getMacAddress() const
    {
        return _macAddress.c_str();
    }

    const char *WiFiManager::getSignalStrength() const
    {
        return _signalStrength.c_str();
    }

} // namespace iotsmartsys::app
