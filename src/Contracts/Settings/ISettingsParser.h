// Contracts/Settings/ISettingsParser.h
#pragma once

#include "esp_err.h"
#include "Settings.h"

namespace iotsmartsys::core::settings
{
    // Erros específicos do parser (mantendo esp_err_t)
    static constexpr esp_err_t ESP_ERR_SETTINGS_PARSE_INVALID_JSON   = ESP_ERR_INVALID_ARG;
    static constexpr esp_err_t ESP_ERR_SETTINGS_PARSE_MISSING_FIELD  = ESP_ERR_INVALID_STATE;
    static constexpr esp_err_t ESP_ERR_SETTINGS_PARSE_OUT_OF_RANGE   = ESP_ERR_INVALID_SIZE;

    class ISettingsParser
    {
    public:
        virtual ~ISettingsParser() = default;

        // Parseia JSON (string) e preenche Settings.
        // Retorna ESP_OK ou um código de erro.
        virtual esp_err_t parse(const char *json, iotsmartsys::core::settings::Settings &out) = 0;
    };
} // namespace iotsmartsys::core::settings