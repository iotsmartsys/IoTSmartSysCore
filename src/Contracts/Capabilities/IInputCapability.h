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
                         const char *type,
                         const char *value)
            : ICapability(event_sink, type, value), inputHardwareAdapter(input_hardware_adapter) {}

        IInputCapability(IInputHardwareAdapter &input_hardware_adapter,
                         ICapabilityEventSink *event_sink,
                         const char *capability_name,
                         const char *type,
                         const char *value)
            : ICapability(event_sink, capability_name, type, value), inputHardwareAdapter(input_hardware_adapter) {}

        // Backwards-compatible overloads that accept std::string
        IInputCapability(IInputHardwareAdapter &input_hardware_adapter,
                         ICapabilityEventSink *event_sink,
                         const std::string &type,
                         const std::string &value)
            : IInputCapability(input_hardware_adapter, event_sink, type.c_str(), value.c_str()) {}

        IInputCapability(IInputHardwareAdapter &input_hardware_adapter,
                         ICapabilityEventSink *event_sink,
                         const std::string &capability_name,
                         const std::string &type,
                         const std::string &value)
            : IInputCapability(input_hardware_adapter, event_sink, capability_name.c_str(), type.c_str(), value.c_str()) {}

        virtual ~IInputCapability() = default;

        virtual void setup() = 0;
        virtual void handle() = 0;

    protected:
        IInputHardwareAdapter &inputHardwareAdapter;
    };
} // namespace iotsmartsys::core
