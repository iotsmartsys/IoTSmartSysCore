#pragma once

#include "Contracts/Settings/Settings.h"

namespace iotsmartsys::core::settings
{
    class IReadOnlySettingsProvider
    {
    public:
        virtual ~IReadOnlySettingsProvider() = default;
        virtual bool hasCurrent() const = 0;
        virtual bool copyCurrent(Settings &out) const = 0;
    };
} // namespace iotsmartsys::core::settings 