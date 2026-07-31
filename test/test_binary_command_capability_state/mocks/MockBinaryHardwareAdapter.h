#pragma once

#include <string>
#include <cstring>
#include "Contracts/Adapters/ICommandHardwareAdapter.h"

namespace iotsmartsys::test::mocks
{
    // Configurable command hardware adapter: lets tests force rejection of
    // applyCommand() or a read-back mismatch, to exercise the
    // BinaryCommandCapability restore/persist protocol's failure paths.
    class MockBinaryHardwareAdapter : public iotsmartsys::core::ICommandHardwareAdapter
    {
    public:
        explicit MockBinaryHardwareAdapter(const char *initialValue) : state(initialValue) {}

        void setup() override { setupCalls++; }
        void handle() override {}
        long lastStateReadMillis() const override { return 0; }

        bool applyCommand(const iotsmartsys::core::IHardwareCommand &command) override
        {
            return applyCommand(command.getCommand().c_str());
        }

        bool applyCommand(const char *value) override
        {
            applyCommandCalls++;
            if (!acceptCommands)
                return false;

            std::string newState = value;
            if (newState == "toggle")
            {
                // Minimal off/on flip, sufficient for the off/on-vocabulary
                // capabilities exercised by these tests.
                newState = (state == "on") ? "off" : "on";
            }

            if (!confirmMismatch)
            {
                state = newState;
            }
            // when confirmMismatch is true, the command is "accepted" but the
            // hardware state deliberately does not move to the requested value.
            return true;
        }

        std::string getStateValue() override { return state; }
        iotsmartsys::core::IHardwareState getState() override { return iotsmartsys::core::IHardwareState(); }

        std::string state;
        bool acceptCommands{true};
        bool confirmMismatch{false};
        int applyCommandCalls{0};
        int setupCalls{0};
    };

} // namespace iotsmartsys::test::mocks
