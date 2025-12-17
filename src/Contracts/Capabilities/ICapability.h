#pragma once

#include <vector>
#include "Contracts/Events/CapabilityStateChanged.h"
#include "Contracts/Events/ICapabilityEventSink.h"
#include "Contracts/Events/CapabilityCommand.h"
#include "ICapabilityType.h"
#include "Contracts/Adapters/IHardwareAdapter.h"
#include "Contracts/Providers/ITimeProvider.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Logging/Log.h"
#include "Contracts/Providers/Time.h"

namespace iotsmartsys::core
{
    struct ICapability
    {
    public:
        ICapability(std::string type, std::string value)
            : capability_name(""), type(type), value(value) {}
        ICapability(std::string capability_name, std::string type, std::string value)
            : capability_name(capability_name), type(type), value(value) {}

        ICapability(IHardwareAdapter *hardware_adapator,
                    std::string capability_name,
                    std::string type,
                    std::string value)
            : hardware_adapator(hardware_adapator), capability_name(capability_name), type(type), value(value) {}

        ICapability(IHardwareAdapter *hardware_adapator,
                    std::string type,
                    std::string value)
            : hardware_adapator(hardware_adapator), capability_name(""), type(type), value(value) {}

        virtual ~ICapability() {}

        std::string capability_name;
        std::string type;
        std::string value;

        // void applyCommand(CapabilityCommand command)
        // {
        //     if (hardware_adapator)
        //     {
        //         hardware_adapator->applyCommand(command.value);
        //         updateState(command.value);
        //     }
        // }

        void updateState(std::string value)
        {
            this->value = value;
            this->changed = true;
        }

        CapabilityStateChanged readState()
        {
            this->changed = false;
            return CapabilityStateChanged(capability_name, value, type);
        }

        bool hasChanged()
        {
            return changed;
        }

        virtual void handle()
        {
        }

        virtual void setup()
        {
        }

        void applyRenamedName(std::string device_id)
        {
            this->capability_name = device_id + "_" + this->type;
        }

        void rename(std::string new_capability_name)
        {
            this->capability_name = new_capability_name;
        }

    protected:
        core::ILogger &logger = core::Log::get();
        core::ITimeProvider &timeProvider = core::Time::get();
        IHardwareAdapter *hardware_adapator;

    private:
        bool changed = false;
    };
}