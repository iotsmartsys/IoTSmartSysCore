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

namespace iotsmartsys::core
{
    struct ICommandCapability : public ICapability
    {
    public:
        ICommandCapability(ICommandHardwareAdapter &hardware_adapator,
                           ICapabilityEventSink *event_sink,
                           std::string capability_name,
                           std::string type,
                           std::string value)
            : ICapability(event_sink, capability_name, type, value), command_hardware_adapter(hardware_adapator) {}

        ICommandCapability(ICommandHardwareAdapter &hardware_adapator,
                           ICapabilityEventSink *event_sink,
                           std::string type,
                           std::string value)
            : ICapability(event_sink, type, value), command_hardware_adapter(hardware_adapator) {}

        virtual ~ICommandCapability() {}

        void applyCommand(CapabilityCommand command)
        {
            logger.info("COMMAND", "Applying command: capability=%s value=%s",
                        command.capability_name.c_str(),
                        command.value.c_str());
            command_hardware_adapter.applyCommand(command.value);
            updateState(command.value);
        }

        virtual void setup() override
        {
            command_hardware_adapter.setup();
            updateState(command_hardware_adapter.getState());
        }

        virtual void handle() override
        {
            if (command_hardware_adapter.getState() != value)
            {
                updateState(command_hardware_adapter.getState());
            }
        }

    protected:
        ICommandHardwareAdapter &command_hardware_adapter;

    private:
    };
}