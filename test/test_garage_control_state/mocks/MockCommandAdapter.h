#pragma once

#include <string>
#include <string.h>
#include "Contracts/Adapters/ICommandHardwareAdapter.h"
#include "Contracts/Adapters/IHardwareCommand.h"
#include "Contracts/Capabilities/ICapabilityType.h"

namespace iotsmartsys::test::mocks
{

    class MockCommandAdapter : public iotsmartsys::core::ICommandHardwareAdapter
    {
    public:
        bool powered = false;
        int pulseCount = 0;

        void setup() override {}
        void handle() override {}
        long lastStateReadMillis() const override { return 0; }

        bool applyCommand(const iotsmartsys::core::IHardwareCommand &command) override
        {
            bool wasOn = command.isEqualTo(POWER_ON_COMMAND);
            powered = wasOn;
            if (wasOn)
            {
                ++pulseCount;
            }
            return true;
        }

        bool applyCommand(const char *value) override
        {
            bool wasOn = (strcmp(value, POWER_ON_COMMAND) == 0);
            powered = wasOn;
            if (wasOn)
            {
                ++pulseCount;
            }
            return true;
        }

        std::string getStateValue() override { return powered ? POWER_ON_COMMAND : POWER_OFF_COMMAND; }
        iotsmartsys::core::IHardwareState getState() override { return iotsmartsys::core::IHardwareState(); }
    };

} // namespace iotsmartsys::test::mocks
