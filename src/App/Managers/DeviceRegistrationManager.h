#pragma once

#include <cstdint>
#include <string>

#include "Contracts/Connections/WiFiManager.h"
#include "Contracts/Providers/IDeviceIdentityProvider.h"
#include "Contracts/Settings/SettingsManager.h"

namespace iotsmartsys::app
{
    class DeviceRegistrationManager
    {
    public:
        DeviceRegistrationManager(core::ILogger &logger,
                                  core::settings::SettingsManager &settingsManager,
                                  core::WiFiManager &wifi,
                                  core::IDeviceIdentityProvider &deviceIdentityProvider);

        void handle();
        bool isRegistered() const { return registered_; }

    private:
        bool tryRegister(const core::settings::Settings &settings, const std::string &deviceId);
        std::string resolveRegistrationUrl(const std::string &apiUrl) const;
        static std::string buildPayload(const std::string &deviceId,
                                        const std::string &macAddress,
                                        const std::string &ipAddress);
        static std::string escapeJson(const std::string &value);
        bool markRegisteredInCache(const core::settings::Settings &settingsSnapshot);
        void scheduleRetry(uint32_t nowMs);

        core::ILogger &logger_;
        core::settings::SettingsManager &settingsManager_;
        core::WiFiManager &wifi_;
        core::IDeviceIdentityProvider &deviceIdentityProvider_;

        bool registered_{false};
        bool missingHttpClientLogged_{false};
        bool invalidApiLogged_{false};
        uint8_t failures_{0};
        uint32_t nextAttemptAtMs_{0};
    };
} // namespace iotsmartsys::app
