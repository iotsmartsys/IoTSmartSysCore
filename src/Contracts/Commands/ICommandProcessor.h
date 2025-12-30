#pragma once

#include "Contracts/Events/DeviceCommand.h"

namespace iotsmartsys::core
{
    class ICommandProcessor
    {
    public:
        virtual ~ICommandProcessor() = default;

        virtual void process(const DeviceCommand &command) = 0;
    };
}