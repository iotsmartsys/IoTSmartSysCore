#pragma once

#include "Contracts/Events/DeviceCommand.h"

namespace iotsmartsys::core
{
    class ICommandParser
    {
    public:
        virtual iotsmartsys::core::DeviceCommand *parseCommand(const char *jsonPayload, size_t payloadLen) = 0;
    };

}