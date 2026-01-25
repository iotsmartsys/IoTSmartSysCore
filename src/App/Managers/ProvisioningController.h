#pragma once

#include <cstdint>

#include "Config/BuildConfig.h"
#include "Contracts/Logging/Log.h"
#include "Contracts/Connections/WiFiManager.h"
#include "Core/Provisioning/ProvisioningManager.h"
#include "Platform/Espressif/Providers/DeviceIdentityProvider.h"

#if IOTSMARTSYS_PROVISIONING_ENABLED && defined(BLE_PROVISIONING_CHANNEL_ENABLE) && (BLE_PROVISIONING_CHANNEL_ENABLE != 0)
#include "Platform/Espressif/Provisioning/BleProvisioningChannel.h"
#endif
#if IOTSMARTSYS_PROVISIONING_ENABLED && defined(WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE) && (WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE != 0)
#include "Platform/Arduino/Provisioning/WebPortalProvisioningChannel.h"
#endif

namespace iotsmartsys::app
{
    class ProvisioningController
    {
    public:
        ProvisioningController(core::ILogger &logger,
                               core::WiFiManager &wifi,
                               platform::espressif::providers::DeviceIdentityProvider &deviceIdentityProvider);

        void begin();
        void handle();
        bool isActive() const;

    private:
        void setupProvisioning();

        static constexpr uint32_t kProvisioningRestartDelayMs = 3000;

        core::ILogger &logger_;
        core::WiFiManager &wifi_;
        platform::espressif::providers::DeviceIdentityProvider &deviceIdentityProvider_;
        core::provisioning::ProvisioningManager *provManager_{nullptr};
#if IOTSMARTSYS_PROVISIONING_ENABLED && defined(BLE_PROVISIONING_CHANNEL_ENABLE) && (BLE_PROVISIONING_CHANNEL_ENABLE != 0)
        core::provisioning::BleProvisioningChannel *bleChannel_{nullptr};
#endif
#if IOTSMARTSYS_PROVISIONING_ENABLED && defined(WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE) && (WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE != 0)
        core::provisioning::WebPortalProvisioningChannel *webPortalChannel_{nullptr};
#endif
        bool inConfigMode_{false};
    };
} // namespace iotsmartsys::app
