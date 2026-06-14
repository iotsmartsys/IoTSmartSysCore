#include "Contracts/Connections/WiFiManager.h"
#include "Config/WifiCredentials.h"
#include "Contracts/Connectivity/ConnectivityGate.h"
#include <algorithm>
#include <cstring>
#include <ctime>
#include <vector>

namespace iotsmartsys::core
{
    namespace
    {
        constexpr std::time_t kMinValidEpoch = 1704067200; // 2024-01-01 00:00:00 UTC

        struct WifiConnectCandidate
        {
            const char *ssid{nullptr};
            const char *password{nullptr};
            uint8_t bssid[6]{};
            int32_t channel{0};
            int32_t rssi{-127};
        };

        bool sameSsid(const String &left, const char *right)
        {
            return right && left == right;
        }

        std::size_t configuredCredentialCount(const WiFiConfig &cfg)
        {
            if (iotsmartsys::config::hasWifiCredentials())
            {
                return iotsmartsys::config::kWifiCredentialCount;
            }

            if (cfg.credentialCount > 0)
            {
                return cfg.credentialCount;
            }

            return (cfg.ssid && cfg.ssid[0]) ? 1 : 0;
        }

        const char *configuredSsid(const WiFiConfig &cfg, std::size_t index)
        {
            if (iotsmartsys::config::hasWifiCredentials())
            {
                return iotsmartsys::config::kWifiCredentials[index].ssid;
            }

            if (cfg.credentialCount > 0)
            {
                return cfg.credentials[index].ssid;
            }

            return cfg.ssid;
        }

        const char *configuredPassword(const WiFiConfig &cfg, std::size_t index)
        {
            if (iotsmartsys::config::hasWifiCredentials())
            {
                return iotsmartsys::config::kWifiCredentials[index].password;
            }

            if (cfg.credentialCount > 0)
            {
                return cfg.credentials[index].password;
            }

            return cfg.password;
        }
    }

    WiFiManager::WiFiManager(iotsmartsys::core::ILogger &log)
        : _log(log), _timeProvider(nullptr)
    {
    }

    void WiFiManager::begin(const WiFiConfig &cfg)
    {
        _timeProvider = &iotsmartsys::core::Time::get();

        _cfg = cfg;
        _attempt = 0;
        _associated = false;
        _gotIp = false;
        _state = WiFiState::Connecting;
        _nextActionAtMs = 0;
        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                           iotsmartsys::core::ConnectivityGate::IP_READY |
                           iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
        }

        _eventId = WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
                                { this->onWiFiEvent(event, info); });

        WiFi.mode(WIFI_STA);
        WiFi.persistent(_cfg.persistent);
        WiFi.setSleep(false);
        WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
        WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
        WiFi.setAutoReconnect(_cfg.autoReconnect);

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
                _credentialAttemptIndex = 0;
                _ssid = WiFi.SSID().c_str();
                _macAddress = WiFi.macAddress().c_str();
                _ipAddress = WiFi.localIP().toString().c_str();
                _signalStrength = String(WiFi.RSSI()).c_str();
                _lastRoamCheckMs = now;
                _log.info("WIFI", "Connected. SSID=%s IP=%s BSSID=%s RSSI=%d",
                          _ssid.c_str(),
                          _ipAddress.c_str(),
                          WiFi.BSSIDstr().c_str(),
                          WiFi.RSSI());
                {
                    auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                    gate.setBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                                 iotsmartsys::core::ConnectivityGate::IP_READY);

                }
            }
            else if (_associated && _nextActionAtMs != 0 && now >= _nextActionAtMs)
            {
                if (_dhcpWaitExtensions == 0)
                {
                    _dhcpWaitExtensions++;
                    _nextActionAtMs = now + _cfg.dhcpTimeoutMs;
                    _log.warn("WIFI", "Associated with AP but still waiting for IP. Extending DHCP wait by %lu ms. BSSID=%s RSSI=%d",
                              (unsigned long)_cfg.dhcpTimeoutMs,
                              WiFi.BSSIDstr().c_str(),
                              WiFi.RSSI());
                }
                else
                {
                    _log.warn("WIFI", "DHCP timeout after association. Retrying WiFi connection. BSSID=%s RSSI=%d",
                              WiFi.BSSIDstr().c_str(),
                              WiFi.RSSI());
                    scheduleRetry();
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
            if (_ntpSyncStarted && !_ntpSyncLogged && isSystemTimeValid())
            {
                _ntpSyncLogged = true;
                _log.info("WIFI", "NTP synchronized. System time is valid for TLS.");
            }

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
                    
                }
                scheduleRetry();
            }

            if (_cfg.meshRoaming && now - _lastRoamCheckMs >= _cfg.roamCheckIntervalMs)
            {
                _lastRoamCheckMs = now;
                const int8_t rssi = WiFi.RSSI();
                _signalStrength = String(rssi).c_str();
                if (rssi <= _cfg.roamRssiThreshold)
                {
                    _log.warn("WIFI", "RSSI=%d below threshold=%d. Re-selecting best mesh AP.",
                              rssi,
                              _cfg.roamRssiThreshold);
                    WiFi.disconnect(false, false);
                    delay(100);
                    startConnect();
                }
            }
            break;
        }
    }

    void WiFiManager::startConnect()
    {
        if (configuredCredentialCount(_cfg) == 0)
        {
            _log.error("WIFI", "SSID not set");
            return;
        }

        _associated = false;
        _gotIp = false;
        _dhcpWaitExtensions = 0;

        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                           iotsmartsys::core::ConnectivityGate::IP_READY |
                           iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
        }

        _state = WiFiState::Connecting;

        // timeout “soft”: se passar, entra em retry (sem bloquear)
        const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());
        _nextActionAtMs = now + _cfg.connectTimeoutMs;

        _log.info("WIFI", "Connecting to WiFi (attempt=%lu credentials=%lu)",
                  (unsigned long)(_attempt + 1),
                  (unsigned long)configuredCredentialCount(_cfg));

        WiFi.disconnect(false, false);
        delay(50);
        selectBestAccessPoint();
        _nextActionAtMs = (_timeProvider ? _timeProvider->nowMs() : millis()) + _cfg.connectTimeoutMs;
        if (_hasTargetBssid)
        {
            _log.info("WIFI", "Selected SSID=%s BSSID=%02X:%02X:%02X:%02X:%02X:%02X channel=%ld RSSI=%ld",
                      _targetSsid.c_str(),
                      _targetBssid[0], _targetBssid[1], _targetBssid[2],
                      _targetBssid[3], _targetBssid[4], _targetBssid[5],
                      (long)_targetChannel,
                      (long)_targetRssi);
            WiFi.begin(_targetSsid.c_str(), _targetPassword.c_str(), _targetChannel, _targetBssid);
        }
        else
        {
            const char *ssid = _targetSsid.empty() ? configuredSsid(_cfg, 0) : _targetSsid.c_str();
            const char *password = _targetPassword.empty() ? configuredPassword(_cfg, 0) : _targetPassword.c_str();
            _log.warn("WIFI", "No scanned AP found. Falling back to generic connect for SSID=%s.", ssid ? ssid : "");
            WiFi.begin(ssid, password);
        }
    }

    void WiFiManager::scheduleRetry()
    {
        _attempt++;
        _credentialAttemptIndex++;

        const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());
        const uint32_t backoff = computeBackoffMs();

        _nextActionAtMs = now + backoff;
        _state = WiFiState::BackoffWaiting;

        const wl_status_t status = WiFi.status();
        _log.warn("WIFI", "Retry in %lu ms (attempt=%lu status=%s reason=%u/%s)",
                  (unsigned long)backoff,
                  (unsigned long)_attempt,
                  statusToString(status),
                  (unsigned)_lastDisconnectReason,
                  disconnectReasonToString(_lastDisconnectReason));
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
            jitter = (uint32_t)(esp_random() % (_cfg.jitterMs + 1));
        }

        return base + jitter;
    }

    bool WiFiManager::selectBestAccessPoint()
    {
        _hasTargetBssid = false;
        _targetChannel = 0;
        _targetRssi = -127;
        _targetSsid.clear();
        _targetPassword.clear();
        std::memset(_targetBssid, 0, sizeof(_targetBssid));

        const std::size_t credentialCount = configuredCredentialCount(_cfg);
        if (credentialCount == 0)
        {
            return false;
        }

        const std::size_t fallbackIndex = _credentialAttemptIndex % credentialCount;
        const char *targetSsid = configuredSsid(_cfg, fallbackIndex);
        const char *targetPassword = configuredPassword(_cfg, fallbackIndex);

        _targetSsid = targetSsid ? targetSsid : "";
        _targetPassword = targetPassword ? targetPassword : "";

        const int n = WiFi.scanNetworks(false, false, false, 120);
        if (n <= 0)
        {
            WiFi.scanDelete();
            return false;
        }

        std::vector<WifiConnectCandidate> candidates;
        for (int i = 0; i < n; ++i)
        {
            if (!sameSsid(WiFi.SSID(i), targetSsid))
            {
                continue;
            }

            uint8_t *bssid = WiFi.BSSID(i);
            if (!bssid)
            {
                continue;
            }

            WifiConnectCandidate candidate{};
            candidate.ssid = targetSsid;
            candidate.password = targetPassword;
            std::memcpy(candidate.bssid, bssid, sizeof(candidate.bssid));
            candidate.channel = WiFi.channel(i);
            candidate.rssi = WiFi.RSSI(i);
            candidates.push_back(candidate);
        }

        if (!candidates.empty())
        {
            std::sort(candidates.begin(), candidates.end(), [](const WifiConnectCandidate &a, const WifiConnectCandidate &b)
                      { return a.rssi > b.rssi; });

            const WifiConnectCandidate &selected = candidates[0];
            _targetSsid = selected.ssid;
            _targetPassword = selected.password ? selected.password : "";
            std::memcpy(_targetBssid, selected.bssid, sizeof(_targetBssid));
            _targetChannel = selected.channel;
            _targetRssi = selected.rssi;
            _hasTargetBssid = true;
        }

        WiFi.scanDelete();
        return _hasTargetBssid;
    }

    void WiFiManager::onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
    {
        auto &gate = iotsmartsys::core::ConnectivityGate::instance();

        switch (event)
        {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            _associated = true;
            _lastDisconnectReason = 0;
            _log.info("WIFI", "Associated with AP.");
            _nextActionAtMs = (_timeProvider ? _timeProvider->nowMs() : millis()) + _cfg.dhcpTimeoutMs;
            gate.setBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED);
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            _associated = true;
            _gotIp = true;
            _lastDisconnectReason = 0;
            startTimeSync();

            gate.setBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                         iotsmartsys::core::ConnectivityGate::IP_READY);
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            _associated = false;
            _gotIp = false;
            _lastDisconnectReason = info.wifi_sta_disconnected.reason;
            _log.warn("WIFI", "Disconnected. reason=%u/%s status=%s",
                      (unsigned)_lastDisconnectReason,
                      disconnectReasonToString(_lastDisconnectReason),
                      statusToString(WiFi.status()));

            gate.clearBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                           iotsmartsys::core::ConnectivityGate::IP_READY |
                           iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
            break;

        default:
            break;
        }
    }

    const char *WiFiManager::statusToString(wl_status_t status)
    {
        switch (status)
        {
        case WL_IDLE_STATUS:
            return "IDLE";
        case WL_NO_SSID_AVAIL:
            return "NO_SSID";
        case WL_SCAN_COMPLETED:
            return "SCAN_COMPLETED";
        case WL_CONNECTED:
            return "CONNECTED";
        case WL_CONNECT_FAILED:
            return "CONNECT_FAILED";
        case WL_CONNECTION_LOST:
            return "CONNECTION_LOST";
        case WL_DISCONNECTED:
            return "DISCONNECTED";
        default:
            return "UNKNOWN";
        }
    }

    const char *WiFiManager::disconnectReasonToString(uint8_t reason)
    {
        if (reason == 0)
        {
            return "NONE";
        }

        const char *name = WiFi.disconnectReasonName(static_cast<wifi_err_reason_t>(reason));
        return (name && name[0] != '\0') ? name : "UNKNOWN";
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

    void WiFiManager::startTimeSync()
    {
        configTzTime("UTC0", "pool.ntp.org", "time.nist.gov", "time.google.com");
        _ntpSyncStarted = true;
        _ntpSyncLogged = false;
        _log.info("WIFI", "Starting NTP sync...");
    }

    bool WiFiManager::isSystemTimeValid() const
    {
        const std::time_t now = std::time(nullptr);
        return now >= kMinValidEpoch;
    }

} // namespace iotsmartsys::app
