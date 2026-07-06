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

        bool sameBssid(const uint8_t *left, const uint8_t *right)
        {
            return left && right && std::memcmp(left, right, 6) == 0;
        }

        void copyBssid(uint8_t (&dst)[6], const uint8_t *src)
        {
            if (src)
            {
                std::memcpy(dst, src, 6);
            }
            else
            {
                std::memset(dst, 0, 6);
            }
        }

        bool sameSsid(const String &left, const char *right)
        {
            return right && left == right;
        }

        bool sameSsid(const std::string &left, const char *right)
        {
            return right && left == right;
        }

        std::size_t configuredCredentialCount(const WiFiConfig &cfg)
        {
            if (cfg.credentialCount > 0)
            {
                return cfg.credentialCount;
            }

            if (cfg.ssid && cfg.ssid[0])
            {
                return 1;
            }

            return iotsmartsys::config::hasWifiCredentials() ? iotsmartsys::config::kWifiCredentialCount : 0;
        }

        const char *configuredSsid(const WiFiConfig &cfg, std::size_t index)
        {
            if (cfg.credentialCount > 0)
            {
                return cfg.credentials[index].ssid;
            }

            if (cfg.ssid && cfg.ssid[0])
            {
                return cfg.ssid;
            }

            return iotsmartsys::config::hasWifiCredentials() ? iotsmartsys::config::kWifiCredentials[index].ssid : nullptr;
        }

        const char *configuredPassword(const WiFiConfig &cfg, std::size_t index)
        {
            if (cfg.credentialCount > 0)
            {
                return cfg.credentials[index].password;
            }

            if (cfg.ssid && cfg.ssid[0])
            {
                return cfg.password;
            }

            return iotsmartsys::config::hasWifiCredentials() ? iotsmartsys::config::kWifiCredentials[index].password : nullptr;
        }

        const char *passwordForConfiguredSsid(const WiFiConfig &cfg, const std::string &ssid)
        {
            const std::size_t credentialCount = configuredCredentialCount(cfg);
            for (std::size_t i = 0; i < credentialCount; ++i)
            {
                const char *candidateSsid = configuredSsid(cfg, i);
                if (sameSsid(ssid, candidateSsid))
                {
                    return configuredPassword(cfg, i);
                }
            }
            return nullptr;
        }

        bool isConfiguredSsid(const WiFiConfig &cfg, const std::string &ssid)
        {
            return passwordForConfiguredSsid(cfg, ssid) != nullptr;
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
                    if (_lastRssiRoamAttemptMs != 0 && now - _lastRssiRoamAttemptMs < _cfg.roamCooldownMs)
                    {
                        _log.warn("WIFI", "RSSI=%d below threshold=%d, but roam cooldown is active (%lu/%lu ms). Keeping current connection.",
                                  rssi,
                                  _cfg.roamRssiThreshold,
                                  (unsigned long)(now - _lastRssiRoamAttemptMs),
                                  (unsigned long)_cfg.roamCooldownMs);
                        break;
                    }

                    _lastRssiRoamAttemptMs = now;
                    _log.warn("WIFI", "RSSI=%d below threshold=%d. Evaluating better AP...",
                              rssi,
                              _cfg.roamRssiThreshold);
                    if (evaluateBetterAccessPoint(rssi))
                    {
                        _log.warn("WIFI", "Switching AP due to better RSSI.");
                        WiFi.disconnect(false, false);
                        delay(100);
                        startConnect();
                    }
                    else
                    {
                        _log.warn("WIFI", "RSSI below threshold, but no better AP found. Keeping current connection.");
                    }
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
        if (_usePreparedTargetOnce && _hasTargetBssid)
        {
            _usePreparedTargetOnce = false;
            _log.info("WIFI", "Using prepared roam target for reconnect.");
        }
        else
        {
            _usePreparedTargetOnce = false;
            selectBestAccessPoint();
        }
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

        const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());
        updateAccessPointCache(now);

        std::vector<WifiConnectCandidate> candidates;
        for (const auto &ap : _apCache)
        {
            if (now - ap.lastSeenMs > _cfg.roamScanCacheMaxAgeMs || !sameSsid(ap.ssid, targetSsid))
            {
                continue;
            }

            WifiConnectCandidate candidate{};
            candidate.ssid = targetSsid;
            candidate.password = targetPassword;
            std::memcpy(candidate.bssid, ap.bssid, sizeof(candidate.bssid));
            candidate.channel = ap.channel;
            candidate.rssi = ap.rssi;
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

        return _hasTargetBssid;
    }

    void WiFiManager::updateAccessPointCache(uint32_t nowMs)
    {
        const int n = WiFi.scanNetworks(false, false, false, 120);
        if (n <= 0)
        {
            _log.warn("WIFI", "WiFi scan found no APs while updating AP cache.");
            WiFi.scanDelete();
            return;
        }

        for (int i = 0; i < n; ++i)
        {
            const std::string ssid = WiFi.SSID(i).c_str();
            if (!isConfiguredSsid(_cfg, ssid))
            {
                continue;
            }

            uint8_t *bssid = WiFi.BSSID(i);
            if (!bssid)
            {
                continue;
            }

            auto existing = std::find_if(_apCache.begin(), _apCache.end(), [&](const ScannedAccessPoint &ap)
                                         { return ap.ssid == ssid && sameBssid(ap.bssid, bssid); });

            if (existing == _apCache.end())
            {
                ScannedAccessPoint ap{};
                ap.ssid = ssid;
                copyBssid(ap.bssid, bssid);
                ap.channel = WiFi.channel(i);
                ap.rssi = WiFi.RSSI(i);
                ap.lastSeenMs = nowMs;
                _apCache.push_back(ap);
            }
            else
            {
                existing->channel = WiFi.channel(i);
                existing->rssi = WiFi.RSSI(i);
                existing->lastSeenMs = nowMs;
            }
        }

        WiFi.scanDelete();

        _apCache.erase(std::remove_if(_apCache.begin(), _apCache.end(), [&](const ScannedAccessPoint &ap)
                                      { return nowMs - ap.lastSeenMs > _cfg.roamScanCacheMaxAgeMs; }),
                       _apCache.end());
    }

    bool WiFiManager::evaluateBetterAccessPoint(int32_t currentRssi)
    {
        const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());
        const std::string currentSsid = WiFi.SSID().c_str();
        uint8_t currentBssid[6]{};
        copyBssid(currentBssid, WiFi.BSSID());

        _log.info("WIFI", "Current AP SSID=%s BSSID=%s RSSI=%ld threshold=%d",
                  currentSsid.c_str(),
                  WiFi.BSSIDstr().c_str(),
                  (long)currentRssi,
                  _cfg.roamRssiThreshold);

        updateAccessPointCache(now);

        const ScannedAccessPoint *best = nullptr;
        int32_t bestImprovement = 0;

        for (const auto &ap : _apCache)
        {
            if (now - ap.lastSeenMs > _cfg.roamScanCacheMaxAgeMs || !isConfiguredSsid(_cfg, ap.ssid))
            {
                continue;
            }

            if (sameBssid(ap.bssid, currentBssid))
            {
                _log.info("WIFI", "Candidate SSID=%s BSSID=%02X:%02X:%02X:%02X:%02X:%02X RSSI=%ld ignored: same BSSID/current AP",
                          ap.ssid.c_str(),
                          ap.bssid[0], ap.bssid[1], ap.bssid[2],
                          ap.bssid[3], ap.bssid[4], ap.bssid[5],
                          (long)ap.rssi);
                continue;
            }

            const int32_t improvement = ap.rssi - currentRssi;
            if (improvement < _cfg.roamMinImprovementDb)
            {
                _log.info("WIFI", "Candidate SSID=%s BSSID=%02X:%02X:%02X:%02X:%02X:%02X RSSI=%ld ignored: improvement %ld dBm < %d dBm",
                          ap.ssid.c_str(),
                          ap.bssid[0], ap.bssid[1], ap.bssid[2],
                          ap.bssid[3], ap.bssid[4], ap.bssid[5],
                          (long)ap.rssi,
                          (long)improvement,
                          _cfg.roamMinImprovementDb);
                continue;
            }

            if (ap.rssi < _cfg.roamCandidateMinRssi)
            {
                _log.info("WIFI", "Candidate SSID=%s BSSID=%02X:%02X:%02X:%02X:%02X:%02X RSSI=%ld ignored: below candidate minimum=%d",
                          ap.ssid.c_str(),
                          ap.bssid[0], ap.bssid[1], ap.bssid[2],
                          ap.bssid[3], ap.bssid[4], ap.bssid[5],
                          (long)ap.rssi,
                          _cfg.roamCandidateMinRssi);
                continue;
            }

            _log.info("WIFI", "Candidate SSID=%s BSSID=%02X:%02X:%02X:%02X:%02X:%02X RSSI=%ld accepted: improvement=%ld dBm",
                      ap.ssid.c_str(),
                      ap.bssid[0], ap.bssid[1], ap.bssid[2],
                      ap.bssid[3], ap.bssid[4], ap.bssid[5],
                      (long)ap.rssi,
                      (long)improvement);

            if (!best || ap.rssi > best->rssi)
            {
                best = &ap;
                bestImprovement = improvement;
            }
        }

        if (!best)
        {
            _log.warn("WIFI", "No better AP found. Keeping current connection.");
            return false;
        }

        const char *password = passwordForConfiguredSsid(_cfg, best->ssid);
        if (!password)
        {
            _log.warn("WIFI", "Best AP SSID=%s has no configured password. Keeping current connection.", best->ssid.c_str());
            return false;
        }

        _targetSsid = best->ssid;
        _targetPassword = password;
        std::memcpy(_targetBssid, best->bssid, sizeof(_targetBssid));
        _targetChannel = best->channel;
        _targetRssi = best->rssi;
        _hasTargetBssid = true;
        _usePreparedTargetOnce = true;

        _log.warn("WIFI", "Better AP found: SSID=%s BSSID=%02X:%02X:%02X:%02X:%02X:%02X RSSI=%ld improvement=%ld dBm",
                  _targetSsid.c_str(),
                  _targetBssid[0], _targetBssid[1], _targetBssid[2],
                  _targetBssid[3], _targetBssid[4], _targetBssid[5],
                  (long)_targetRssi,
                  (long)bestImprovement);
        return true;
    }

    void WiFiManager::logDnsDiagnostics()
    {
        const IPAddress dns0 = WiFi.dnsIP(0);
        const IPAddress dns1 = WiFi.dnsIP(1);
        const String dns0Text = dns0.toString();
        const String dns1Text = dns1.toString();

        _log.info("WIFI", "DNS_SERVER index=0 ip=%s", dns0Text.c_str());
        _log.info("WIFI", "DNS_SERVER index=1 ip=%s", dns1Text.c_str());

        IPAddress resolvedIp;
        const int resolved = WiFi.hostByName("api.iotsmartsys.tech", resolvedIp);
        const String resolvedText = resolvedIp.toString();
        _log.info("WIFI", "DNS_TEST api.iotsmartsys.tech -> %s result=%d",
                  resolvedText.c_str(),
                  resolved);
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
            _connectionCount++;
            startTimeSync();
            logDnsDiagnostics();

            gate.setBits(iotsmartsys::core::ConnectivityGate::WIFI_CONNECTED |
                         iotsmartsys::core::ConnectivityGate::IP_READY);
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            _associated = false;
            _gotIp = false;
            _lastDisconnectReason = info.wifi_sta_disconnected.reason;
            _lastDisconnectedAtMs = (_timeProvider ? _timeProvider->nowMs() : millis());
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

    int32_t WiFiManager::getRssi() const
    {
        return isConnected() ? WiFi.RSSI() : 0;
    }

    uint32_t WiFiManager::getLastDisconnectedAtMs() const
    {
        return _lastDisconnectedAtMs;
    }

    uint8_t WiFiManager::getLastDisconnectReason() const
    {
        return _lastDisconnectReason;
    }

    uint32_t WiFiManager::getConnectionCount() const
    {
        return _connectionCount;
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
