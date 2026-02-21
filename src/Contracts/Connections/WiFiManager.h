#pragma once

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

        // retry policy
        uint32_t initialBackoffMs = 1000;
        uint32_t maxBackoffMs = 60000;
        uint32_t jitterMs = 250;
        uint8_t maxFastRetries = 5;
        uint32_t reconnectMinUptimeMs = 3000;

        bool persistent = false;
        bool autoReconnect = false;

        bool loadFromSettings(const core::settings::Settings &s)
        {
            if (!s.isValidWifiConfig())
                return false;

            ssid = s.wifi.ssid.c_str();
            password = s.wifi.password.c_str();
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

    private:
        void startConnect();
        void scheduleRetry();
        uint32_t computeBackoffMs() const;
        void startTimeSync();
        bool isSystemTimeValid() const;

        void onWiFiEvent(WiFiEvent_t event);
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

        uint32_t _connectedAtMs{0};
        bool _gotIp{false};
        iotsmartsys::core::ITimeProvider *_timeProvider{nullptr};
        bool _ntpSyncStarted{false};
        bool _ntpSyncLogged{false};

    };

} // namespace iotsmartsys::app
