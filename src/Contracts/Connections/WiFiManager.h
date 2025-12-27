#pragma once

#include <cstdint>
#include <vector>

#include "Contracts/Logging/ILogger.h"
#include "Contracts/Providers/Time.h"
#include "Contracts/Providers/ITimeProvider.h"

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

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

    class WiFiManager
    {
    public:
        explicit WiFiManager(iotsmartsys::core::ILogger &log);

        void begin(const WiFiConfig &cfg);
        void handle(); // chama no loop

        bool isConnected() const;
        const char *stateName() const;
        std::vector<std::string> getAvailableSSIDs();

    private:
        enum class State : uint8_t
        {
            Idle,
            Connecting,
            Connected,
            BackoffWaiting
        };

        void startConnect();
        void scheduleRetry();
        uint32_t computeBackoffMs() const;

#ifdef ESP32
        void onWiFiEvent(WiFiEvent_t event);
        WiFiEventId_t _eventId{};
#endif

    private:
        iotsmartsys::core::ILogger &_log;
        WiFiConfig _cfg{};

        State _state{State::Idle};

        uint32_t _attempt{0}; 
        uint32_t _nextActionAtMs{0};

        uint32_t _connectedAtMs{0};
        bool _gotIp{false};
        iotsmartsys::core::ITimeProvider *_timeProvider{nullptr};
    };

} // namespace iotsmartsys::app