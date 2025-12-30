#pragma once

#include "Contracts/Parsers/ICommandParser.h"
#include "Contracts/Logging/ILogger.h"

namespace iotsmartsys::platform::espressif
{
    class EspIdfCommandParser : public iotsmartsys::core::ICommandParser
    {
    public:
        EspIdfCommandParser(iotsmartsys::core::ILogger &logger);
        iotsmartsys::core::DeviceCommand *parseCommand(const char *jsonPayload, size_t payloadLen) override;

    private:
        iotsmartsys::core::ILogger &_logger;
    };
}
