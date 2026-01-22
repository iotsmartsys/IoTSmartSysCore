#include "Contracts/Connections/WiFiManager.h"

#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "Contracts/Connectivity/ConnectivityGate.h"

namespace iotsmartsys::core
{
    WiFiManager::WiFiManager(iotsmartsys::core::ILogger &log)
        : _log(log)
    {
        _timeProvider = &iotsmartsys::core::Time::get();
    }

    void WiFiManager::begin(const WiFiConfig &cfg)
    {
        _cfg = cfg;
        _attempt = 0;
        _gotIp = false;
        _state = WiFiState::Idle;
        _nextActionAtMs = 0;
        _connectedAtMs = 0;
        _disconnectSinceMs = 0;

        _ipAddress.clear();
        _macAddress = WiFi.macAddress().c_str();
        _ssid.clear();
        _signalStrength.clear();

        // Zera bits no início (igual ESP32)
        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                           iotsmartsys::core::ConnectivityGate::IP_READY |
                           iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
        }

        WiFi.mode(WIFI_STA);
        WiFi.persistent(_cfg.persistent);
        WiFi.setAutoReconnect(_cfg.autoReconnect);

        // --- EVENT HANDLERS (equivalente ao WiFi.onEvent do ESP32) ---
        _hConnected = WiFi.onStationModeConnected([this](const WiFiEventStationModeConnected &)
                                                  {
                                                      auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                                                      gate.setBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED);
                                                      _log.debug("WIFI", "Event: STA_CONNECTED. gate=0x%08x", gate.bits());
                                                  });

        _hGotIp = WiFi.onStationModeGotIP([this](const WiFiEventStationModeGotIP &evt)
                                          {
                                              (void)evt;
                                              _gotIp = true;

                                              // atualiza strings úteis imediatamente
                                              _ssid = WiFi.SSID().c_str();
                                              _macAddress = WiFi.macAddress().c_str();
                                              _ipAddress = WiFi.localIP().toString().c_str();
                                              _signalStrength = String(WiFi.RSSI()).c_str();

                                              auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                                              gate.setBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                                                           iotsmartsys::core::ConnectivityGate::IP_READY);

                                              _log.info("WIFI", "Event: GOT_IP. IP=%s", _ipAddress.c_str());
                                              _log.debug("WIFI", "gate=0x%08x", gate.bits());
                                          });

        _hDisconnected = WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected &)
                                                        {
                                                            _gotIp = false;

                                                            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                                                            gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                                                                           iotsmartsys::core::ConnectivityGate::IP_READY |
                                                                           iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);

                                                            _log.warn("WIFI", "Event: DISCONNECTED. gate=0x%08x", gate.bits());
                                                        });

        // Começa tentativa imediatamente (como no ESP32)
        startConnect();
    }

    bool WiFiManager::isConnected() const
    {
        return _gotIp && WiFi.status() == WL_CONNECTED;
    }

    WiFiState WiFiManager::currentState() const
    {
        return _state;
    }

    void WiFiManager::handle()
    {
        const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());

        // Polling de segurança (cinto) — garante coerência mesmo se evento falhar
        const bool connectedNow = (WiFi.status() == WL_CONNECTED) && (WiFi.localIP()[0] != 0);
        if (connectedNow && !_gotIp)
        {
            _gotIp = true;
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            gate.setBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                         iotsmartsys::core::ConnectivityGate::IP_READY);
        }
        else if (!connectedNow && _gotIp)
        {
            _gotIp = false;
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                           iotsmartsys::core::ConnectivityGate::IP_READY |
                           iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
        }

        switch (_state)
        {
        case WiFiState::Idle:
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

                // Garante bits setados (mesmo que GOT_IP já tenha setado)
                {
                    auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                    gate.setBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                                 iotsmartsys::core::ConnectivityGate::IP_READY);
                    _log.debug("WIFI", "ConnectivityGate after set = 0x%08x", gate.bits());
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
            if (!isConnected())
            {
                if (now - _connectedAtMs < _cfg.reconnectMinUptimeMs)
                {
                    _log.warn("WIFI", "Flapping detected; delaying reconnect");
                }

                // Clear bits (igual ESP32)
                {
                    auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                    gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                                   iotsmartsys::core::ConnectivityGate::IP_READY |
                                   iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
                    _log.debug("WIFI", "ConnectivityGate after clear = 0x%08x", gate.bits());
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

        // Clear bits antes de tentar (igual ESP32)
        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                           iotsmartsys::core::ConnectivityGate::IP_READY |
                           iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
        }

        _state = WiFiState::Connecting;

        const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());
        _nextActionAtMs = now + 30000; // 30s no ESP8266 (mais realista que 5s)

        _log.info("WIFI", "Connecting to SSID=%s (attempt=%lu)", _cfg.ssid, (unsigned long)(_attempt + 1));

        // não limpa credenciais (evita desgaste), só reinicia sessão
        WiFi.disconnect(false);
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
            jitter = (uint32_t)random(0, (long)_cfg.jitterMs + 1);

        return base + jitter;
    }

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

        WiFi.scanDelete();
        return ssids;
    }

    const char *WiFiManager::getSsid() const { return _ssid.c_str(); }
    const char *WiFiManager::getIpAddress() const { return _ipAddress.c_str(); }
    const char *WiFiManager::getMacAddress() const { return _macAddress.c_str(); }
    const char *WiFiManager::getSignalStrength() const { return _signalStrength.c_str(); }

} // namespace iotsmartsys::core

#endif