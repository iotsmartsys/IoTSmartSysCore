#pragma once

#include "Contracts/Parsers/ICommandParser.h"
#include "Contracts/Logging/ILogger.h"
#include <string>
#include <cstddef>

namespace iotsmartsys::platform::espressif
{
    class EspIdfCommandParser : public iotsmartsys::core::ICommandParser
    {
    public:
        EspIdfCommandParser(iotsmartsys::core::ILogger &logger);
        iotsmartsys::core::DeviceCommand *parseCommand(const char *jsonPayload, size_t payloadLen) override;

    private:
        iotsmartsys::core::ILogger &_logger;
        const char *skipWhitespace(const char *p, const char *end) const;
        const char *findKeyValueStart(const char *json, const char *end, const char *key) const;
        const char *parseJsonString(const char *p, const char *end, std::string &out) const;
        void unescapeJsonStringInPlace(std::string &s) const;
        bool tryExtractJsonStringField(const char *json, size_t len, const char *key, std::string &out) const;
    };
}
