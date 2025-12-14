#pragma once

#include <Arduino.h>
#include "Contracts/Providers/ITimeProvider.h"

namespace iotsmartsys::platform::arduino
{

    class ArduinoTimeProvider : public core::ITimeProvider
    {
    public:
        std::uint64_t nowMs() const override
        {
            return static_cast<std::uint64_t>(::millis());
        }
    };

} // namespace iotsmartsys::platform::arduino