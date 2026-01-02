#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"

namespace iotsmartsys::core
{
    class LEDCapability : public BinaryCommandCapability
    {
    public:
        LEDCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);
        // Backwards-compatible overload
        LEDCapability(const std::string &capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
            : LEDCapability(capability_name.c_str(), hardwareAdapter, event_sink) {}

        void handle() override;

        void toggle();
        void turnOn();
        void turnOff();
        bool isOn() const;
        void executeCommand(const char *state);
        void blink(unsigned long intervalMs);

    private:
        void power(const char *state);
        unsigned long blinkInterval{0};
        unsigned long lastToggleTs{0};
        bool blinking{false};
    };

} // namespace iotsmartsys::core
