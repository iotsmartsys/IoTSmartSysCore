#pragma once

#include "ICapability.h"

namespace iotsmartsys::core
{
    class SwitchCapability : public ICapability
    {
    public:
        SwitchCapability(std::string capability_name, IHardwareAdapter &hardwareAdapter);

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
