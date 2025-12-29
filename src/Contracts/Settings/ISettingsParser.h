// Contracts/Settings/ISettingsParser.h
#pragma once

#include "Contracts/Common/StateResult.h"
#include "Settings.h"

namespace iotsmartsys::core::settings
{
    // Erros específicos do parser (usando StateResult do core)
    static constexpr iotsmartsys::core::common::StateResult SETTINGS_PARSE_INVALID_JSON   = iotsmartsys::core::common::StateResult::InvalidArg;
    static constexpr iotsmartsys::core::common::StateResult SETTINGS_PARSE_MISSING_FIELD  = iotsmartsys::core::common::StateResult::InvalidState;
    static constexpr iotsmartsys::core::common::StateResult SETTINGS_PARSE_OUT_OF_RANGE   = iotsmartsys::core::common::StateResult::Overflow;

    class ISettingsParser
    {
    public:
        virtual ~ISettingsParser() = default;

        // Parseia JSON (string) e preenche Settings.
        // Retorna Error::Ok ou um código de erro.
        virtual iotsmartsys::core::common::StateResult parse(const char *json, iotsmartsys::core::settings::Settings &out) = 0;
    };
} // namespace iotsmartsys::core::settings