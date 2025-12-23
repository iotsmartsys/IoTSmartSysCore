#pragma once

#include "Contracts/Events/DeviceCommand.h"

namespace iotsmartsys::core
{
    class ICommandParser
    {
    public:
    virtual bool parseCommand(const char *jsonPayload, size_t payloadLen, DeviceCommand &outCmd) = 0;
    };

}