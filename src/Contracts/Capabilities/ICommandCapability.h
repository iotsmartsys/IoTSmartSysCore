#pragma once

#include <vector>
#include "Contracts/Events/CapabilityStateChanged.h"
#include "Contracts/Events/ICapabilityEventSink.h"
#include "Contracts/Events/CapabilityCommand.h"
#include "ICapabilityType.h"
#include "Contracts/Adapters/ICommandHardwareAdapter.h"
#include "Contracts/Providers/ITimeProvider.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Logging/Log.h"
#include "Contracts/Providers/Time.h"
#include "ICapability.h"
#include "Contracts/Interpreters/IHardwareCommandInterpreter.h"

namespace iotsmartsys::core
{
    struct ICommandCapability : public ICapability
    {
    public:
        ICommandCapability(ICommandHardwareAdapter &hardware_adapator,
                           ICapabilityEventSink *event_sink,
                           const char *capability_name,
                           const char *type,
                           const char *value)
            : ICapability(event_sink, capability_name, type, value), command_hardware_adapter(hardware_adapator) {}

        // Backwards-compatible overloads accepting std::string
        ICommandCapability(ICommandHardwareAdapter &hardware_adapator,
                           ICapabilityEventSink *event_sink,
                           const std::string &capability_name,
                           const std::string &type,
                           const std::string &value)
            : ICommandCapability(hardware_adapator, event_sink, capability_name.c_str(), type.c_str(), value.c_str()) {}

        ICommandCapability(ICommandHardwareAdapter &hardware_adapator,
                           ICapabilityEventSink *event_sink,
                           const char *type,
                           const char *value)
            : ICapability(event_sink, type, value), command_hardware_adapter(hardware_adapator) {}

        ICommandCapability(ICommandHardwareAdapter &hardware_adapator,
                           ICapabilityEventSink *event_sink,
                           const std::string &type,
                           const std::string &value)
            : ICommandCapability(hardware_adapator, event_sink, type.c_str(), value.c_str()) {}

        virtual ~ICommandCapability() {}

        // Override to allow safe runtime-query from base ICapability
        // without using dynamic_cast (RTTI disabled).
        virtual ICommandCapability *asCommandCapability() override
        {
            return this;
        }

        virtual void applyCommand(CapabilityCommand command)
        {            

            if (command_interpreter)
            {
                IHardwareCommand hwCommand = command_interpreter->interpretCommand(command);
                command_hardware_adapter.applyCommand(hwCommand);
                return;
            }

            command_hardware_adapter.applyCommand(command.value);
        }

        virtual void setup() override
        {
            command_hardware_adapter.setup();
            updateState(command_hardware_adapter.getStateValue());
        }

        virtual void handle() override
        {
            if (command_hardware_adapter.getStateValue() != value)
            {
                updateState(command_hardware_adapter.getStateValue());
            }
        }

        virtual void setCommandInterpreter(IHardwareCommandInterpreter *interpreter)
        {
            command_interpreter = interpreter;
        }

    protected:
        ICommandHardwareAdapter &command_hardware_adapter;
        IHardwareCommandInterpreter *command_interpreter{nullptr};

    private:
    };
}