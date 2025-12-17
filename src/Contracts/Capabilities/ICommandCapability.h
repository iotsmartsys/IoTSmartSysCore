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
        ICommandCapability(ICommandHardwareAdapter *hardware_adapator,
                           std::string capability_name,
                           std::string type,
                           std::string value)
            : ICapability(command_hardware_adapater, capability_name, type, value), command_hardware_adapater(hardware_adapator) {}

        ICommandCapability(ICommandHardwareAdapter *hardware_adapator,
                           std::string type,
                           std::string value)
            : ICapability(command_hardware_adapater, "", type, value), command_hardware_adapater(hardware_adapator) {}

        virtual ~ICommandCapability() {}

        void applyCommand(CapabilityCommand command)
        {
            if (command_hardware_adapater)
            {
                command_hardware_adapater->applyCommand(command.value);
                updateState(command.value);
            }
        }

        virtual void setup() override
        {
        }

        virtual void handle() override
        {
        }

    protected:
        ICommandHardwareAdapter *command_hardware_adapater;

    private:
        bool changed = false;
    };
}