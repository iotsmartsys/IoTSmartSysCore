#pragma once

#include "Contracts/Events/DeviceCommand.h"

namespace iotsmartsys::core
{
    class ICommandParser
    {
    public:
        virtual iotsmartsys::core::DeviceCommand *parseCommand(const char *jsonPayload, size_t payloadLen) = 0;
        // Optional per-call diagnostic policy; existing parsers remain source compatible.
        virtual iotsmartsys::core::DeviceCommand *parseCommand(const char *jsonPayload, size_t payloadLen, bool logDetails)
        {
            (void)logDetails;
            return parseCommand(jsonPayload, payloadLen);
        }
    };

}