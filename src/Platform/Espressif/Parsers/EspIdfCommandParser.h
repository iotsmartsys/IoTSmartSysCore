#pragma once

#include "Contracts/Parsers/ICommandParser.h"

namespace iotsmartsys::platform::espressif
{
    class EspIdfCommandParser : public iotsmartsys::core::ICommandParser
    {
    public:
        bool parseCommand(const char *jsonPayload, size_t payloadLen, iotsmartsys::core::DeviceCommand &outCmd) override;
    };
}
