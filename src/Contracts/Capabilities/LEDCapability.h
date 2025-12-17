#pragma once

#include "ICommandCapability.h"

namespace iotsmartsys::core
{
    class LEDCapability : public ICommandCapability
    {
    public:
        LEDCapability(std::string capability_name, ICommandHardwareAdapter &hardwareAdapter);

        void setup() override;
        void handle() override;

        void toggle();
        void turnOn();
        void turnOff();
        bool isOn() const;
        void executeCommand(const std::string &state);
        void blink(unsigned long intervalMs);

    private:
        void power(const std::string &state);
        unsigned long blinkInterval{0};
        unsigned long lastToggleTs{0};
        bool blinking{false};
    };

} // namespace iotsmartsys::core
