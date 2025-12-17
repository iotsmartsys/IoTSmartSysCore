#pragma once

#include "ICommandCapability.h"

namespace iotsmartsys::core
{
    class SwitchPlugCapability : public ICommandCapability
    {
    public:
        SwitchPlugCapability(std::string capability_name, ICommandHardwareAdapter &hardwareAdapter);

        void setup() override;
        void handle() override;

        void toggle();
        void turnOn();
        void turnOff();
        bool isOn() const;
        void executeCommand(const std::string &state);

    private:
        void power(const std::string &state);
    };

} // namespace iotsmartsys::core
