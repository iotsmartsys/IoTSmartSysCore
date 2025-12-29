// Contracts/Providers/ISettingsProvider.h
#pragma once

#include "Contracts/Common/StateResult.h"
#include "Contracts/Settings/Settings.h"

namespace iotsmartsys::core::providers
{
    class ISettingsProvider
    {
    public:
        virtual ~ISettingsProvider() = default;

        /// @brief Carrega as configurações de settings.
        /// @param out Referência para o objeto Settings onde as configurações carregadas serão armazenadas.
        /// @return StateResult indicando o resultado da operação.
        virtual iotsmartsys::core::common::StateResult load(iotsmartsys::core::settings::Settings &out) = 0;

        /// @brief Salva as configurações de settings.
        /// @param settings Referência para o objeto Settings que contém as configurações a serem salvas.
        /// @return StateResult indicando o resultado da operação.
        virtual iotsmartsys::core::common::StateResult save(const iotsmartsys::core::settings::Settings &settings) = 0;

        /// @brief Salva apenas as configurações de Wi-Fi.
        /// @param wifi Referência para o objeto WifiConfig que contém as configurações de Wi-Fi a serem salvas.
        /// @return StateResult indicando o resultado da operação.
        virtual iotsmartsys::core::common::StateResult saveWiFiOnly(const iotsmartsys::core::settings::WifiConfig &wifi) = 0;

        /// @brief Apaga as configurações de settings (persistência).
        /// @return StateResult indicando o resultado da operação.
        virtual iotsmartsys::core::common::StateResult erase() = 0;

        /// @brief Verifica se as configurações existem.
        /// @return true se as configurações existem, false caso contrário.
        virtual bool exists() = 0;
    };
} // namespace iotsmartsys::core::providers