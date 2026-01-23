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
        virtual std::string getDeviceID() const = 0;

        /// @brief Obtém o identificador único do dispositivo.
        /// @return Identificador único do dispositivo.
        virtual std::string getDeviceUniqueId() const = 0;

        /// @brief Obtem o modelo do Chip/Board
        /// @return Modelo do Chip/Board
        virtual std::string getDeviceModel() const = 0;
    };

} // namespace iotsmartsys::core