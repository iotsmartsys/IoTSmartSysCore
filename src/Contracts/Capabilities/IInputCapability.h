#pragma once

#include "ICapability.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::core
{
    class IInputCapability : public ICapability
    {
    public:
        IInputCapability(IInputHardwareAdapter &input_hardware_adapter,
                         ICapabilityEventSink *event_sink,
                         std::string type,
                         std::string value)
            : ICapability(event_sink, type, value), inputHardwareAdapter(input_hardware_adapter) {}

        IInputCapability(IInputHardwareAdapter &input_hardware_adapter,
                         ICapabilityEventSink *event_sink,
                         std::string capability_name,
                         std::string type,
                         std::string value)
            : ICapability(event_sink, capability_name, type, value), inputHardwareAdapter(input_hardware_adapter) {}

        virtual ~IInputCapability() = default;

        virtual void setup() = 0;
        virtual void handle() = 0;

    protected:
        IInputHardwareAdapter &inputHardwareAdapter;
    };
} // namespace iotsmartsys::core
