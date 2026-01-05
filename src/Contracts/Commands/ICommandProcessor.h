#pragma once

#include "Contracts/Events/DeviceCommand.h"

namespace iotsmartsys::core
{
    class ICommandProcessor
    {
    public:
        virtual ~ICommandProcessor() = default;

        virtual bool process(const DeviceCommand &command) = 0;
    };
}