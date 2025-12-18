// Contracts/Providers/ISettingsProvider.h
#pragma once

#include "esp_err.h"
#include "Contracts/Settings/Settings.h"

namespace iotsmartsys::core::providers
{
    class ISettingsProvider
    {
    public:
        virtual ~ISettingsProvider() = default;

        // Carrega settings; retorna ESP_ERR_NVS_NOT_FOUND se ainda não existe.
        virtual esp_err_t load(iotsmartsys::core::settings::Settings &out) = 0;

        // Salva settings (persistência).
        virtual esp_err_t save(const iotsmartsys::core::settings::Settings &settings) = 0;
        
        // Apaga settings (persistência).
        virtual esp_err_t erase() = 0;

        // Verifica se settings existem.
        virtual bool exists() = 0;
    };
} // namespace iotsmartsys::core::providers