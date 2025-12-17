#include "Time.h"

#include <Arduino.h>

namespace iotsmartsys::core
{
    ITimeProvider *Time::_provider = nullptr;

    // Default provider that uses Arduino millis() when no provider is set yet.
    namespace
    {
        class DefaultTimeProvider : public ITimeProvider
        {
        public:
            std::uint64_t nowMs() const override
            {
                return static_cast<std::uint64_t>(millis());
            }
        } defaultProvider;
    }

    ITimeProvider &Time::get()
    {
        return (_provider != nullptr) ? *_provider : static_cast<ITimeProvider &>(defaultProvider);
    }
}
