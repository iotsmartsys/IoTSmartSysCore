#pragma once

#include "Contracts/Providers/IDeviceIdentityProvider.h"

namespace iotsmartsys::platform::espressif::providers
{
    /// @brief Provedor de identidade do dispositivo baseado em Arduino.
    class DeviceIdentityProvider : public iotsmartsys::core::IDeviceIdentityProvider
    {
    public:
        /// @brief Obtém o nome único do dispositivo.
        /// @return Nome do dispositivo.
        std::string getDeviceID() const override;

        /// @brief Obtém o identificador único do dispositivo.
        /// @return Identificador único do dispositivo.
        std::string getDeviceUniqueId() const override;
    };

} // namespace iotsmartsys::core