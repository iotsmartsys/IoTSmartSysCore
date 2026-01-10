#pragma once

#include <string>

namespace iotsmartsys::core
{

    /// @brief Provedor de identidade do dispositivo.
    class IDeviceIdentityProvider
    {
    public:
        virtual ~IDeviceIdentityProvider() = default;

        /// @brief Obtém o nome único do dispositivo.
        /// @return Nome do dispositivo.
        virtual std::string getDeviceName() const = 0;

        /// @brief Obtém o identificador único do dispositivo.
        /// @return Identificador único do dispositivo.
        virtual std::string getDeviceUniqueId() const = 0;
    };

} // namespace iotsmartsys::core::provisioning