#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <ctime>

#include "Contracts/Logging/ILogger.h"
#include "Contracts/Providers/Time.h"
#include "Contracts/Providers/ITimeProvider.h"

#include <WiFi.h>

#include "Contracts/Settings/Settings.h"

namespace iotsmartsys::core
{

    struct WiFiConfig
    {
        const char *ssid = nullptr;
        const char *password = nullptr;
        struct Credential
        {
            const char *ssid = nullptr;
            const char *password = nullptr;
        };
        Credential credentials[3]{};
        std::size_t credentialCount = 0;

        // retry policy
        uint32_t initialBackoffMs = 1000;
        uint32_t maxBackoffMs = 60000;
        uint32_t jitterMs = 250;
        uint32_t connectTimeoutMs = 15000;
        uint32_t dhcpTimeoutMs = 45000;
        uint8_t maxFastRetries = 5;
        uint32_t reconnectMinUptimeMs = 3000;
        uint32_t roamCheckIntervalMs = 30000;
        int8_t roamRssiThreshold = -75;
        bool meshRoaming = true;

        bool persistent = false;
        bool autoReconnect = false;

        bool loadFromSettings(const core::settings::Settings &s)
        {
            if (!s.isValidWifiConfig())
                return false;

            credentialCount = 0;
            const auto append = [this](const core::settings::WifiProfileConfig &profile)
            {
                if (!profile.isValid() || credentialCount >= 3)
                {
                    return;
                }

                credentials[credentialCount].ssid = profile.ssid.c_str();
                credentials[credentialCount].password = profile.password.c_str();
                credentialCount++;
            };

            if (s.wifi.profile == "tertiary")
            {
                append(s.wifi.tertiary);
                append(s.wifi.primary);
                append(s.wifi.secondary);
            }
            else if (s.wifi.profile == "secondary")
            {
                append(s.wifi.secondary);
                append(s.wifi.primary);
                append(s.wifi.tertiary);
            }
            else
            {
                append(s.wifi.primary);
                append(s.wifi.secondary);
                append(s.wifi.tertiary);
            }

            if (credentialCount == 0)
            {
                ssid = s.wifi.ssid.c_str();
                password = s.wifi.password.c_str();
            }
            else
            {
                ssid = credentials[0].ssid;
                password = credentials[0].password;
            }
            return true;
        }
    };

    enum class WiFiState : uint8_t
    {
        Idle,
        Connecting,
        Connected,
        BackoffWaiting
    };

    class WiFiManager
    {
    public:
        explicit WiFiManager(iotsmartsys::core::ILogger &log);

        void begin(const WiFiConfig &cfg);
        void handle(); // chama no loop

        bool isConnected() const;
        WiFiState currentState() const;
        std::vector<std::string> getAvailableSSIDs();
        const char *getIpAddress() const;
        const char *getMacAddress() const;
        const char *getSsid() const;
        const char *getSignalStrength() const;
        int32_t getRssi() const;
        uint32_t getLastDisconnectedAtMs() const;
        uint8_t getLastDisconnectReason() const;
        uint32_t getConnectionCount() const;

    private:
        void startConnect();
        void scheduleRetry();
        uint32_t computeBackoffMs() const;
        bool selectBestAccessPoint();
        void startTimeSync();
        bool isSystemTimeValid() const;

        void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
        static const char *statusToString(wl_status_t status);
        static const char *disconnectReasonToString(uint8_t reason);
        WiFiEventId_t _eventId{};

    private:
        iotsmartsys::core::ILogger &_log;
        WiFiConfig _cfg{};

        WiFiState _state{WiFiState::Idle};
        std::string _ipAddress;
        std::string _macAddress;
        std::string _ssid;
        std::string _signalStrength;

        uint32_t _attempt{0};
        uint32_t _nextActionAtMs{0};
        uint32_t _lastRoamCheckMs{0};
        uint16_t _credentialAttemptIndex{0};
        uint8_t _lastDisconnectReason{0};
        uint8_t _dhcpWaitExtensions{0};
        uint8_t _targetBssid[6]{};
        int32_t _targetChannel{0};
        int32_t _targetRssi{-127};
        std::string _targetSsid;
        std::string _targetPassword;
        bool _hasTargetBssid{false};

        uint32_t _connectedAtMs{0};
        uint32_t _lastDisconnectedAtMs{0};
        uint32_t _connectionCount{0};
        bool _associated{false};
        bool _gotIp{false};
        iotsmartsys::core::ITimeProvider *_timeProvider{nullptr};
        bool _ntpSyncStarted{false};
        bool _ntpSyncLogged{false};

    };

} // namespace iotsmartsys::app
