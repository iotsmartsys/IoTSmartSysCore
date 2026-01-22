#pragma once

#include "Contracts/Settings/ISettingsParser.h"

namespace iotsmartsys::platform::espressif
{
    class EspIdfSettingsParser final : public iotsmartsys::core::settings::ISettingsParser
    {
    public:
        EspIdfSettingsParser() = default;
        ~EspIdfSettingsParser() override = default;

        iotsmartsys::core::common::StateResult parse(const char *json, iotsmartsys::core::settings::Settings &out) override;

    private:
    };
} // namespace iotsmartsys::platform::espressif