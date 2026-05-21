#include "EspIdfCommandParser.h"
#include "Contracts/Commands/CommandTypes.h"

#include <string>
#include <cstring>

namespace iotsmartsys::platform::espressif
{
    EspIdfCommandParser::EspIdfCommandParser(iotsmartsys::core::ILogger &logger)
        : _logger(logger)
    {
    }

    const char *EspIdfCommandParser::skipWhitespace(const char *p, const char *end) const
    {
        while (p && p < end)
        {
            const char c = *p;
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
                break;
            ++p;
        }
        return p;
    }

    const char *EspIdfCommandParser::findKeyValueStart(const char *json, const char *end, const char *key) const
    {
        if (!json || !key)
            return nullptr;

        const size_t keyLen = std::strlen(key);
        const char *p = json;

        while (p && p < end)
        {
            // find opening quote
            const void *qv = std::memchr(p, '"', (size_t)(end - p));
            if (!qv)
                return nullptr;
            const char *q = (const char *)qv;
            q++; // after opening quote

            if (q + (ptrdiff_t)keyLen < end && std::strncmp(q, key, keyLen) == 0 && (q + keyLen) < end && q[keyLen] == '"')
            {
                const char *after = q + keyLen + 1;
                after = skipWhitespace(after, end);
                if (after && after < end && *after == ':')
                {
                    after++;
                    return skipWhitespace(after, end);
                }
            }

            p = q;
        }

        return nullptr;
    }

    const char *EspIdfCommandParser::parseJsonString(const char *p, const char *end, std::string &out) const
    {
        p = skipWhitespace(p, end);
        if (!p || p >= end || *p != '"')
            return nullptr;

        ++p; // skip opening quote
        std::string tmp;
        tmp.reserve(64);

        bool escaped = false;
        while (p < end)
        {
            const char c = *p++;

            if (escaped)
            {
                // Keep escapes in a normalized form; we'll unescape afterwards.
                tmp.push_back('\\');
                tmp.push_back(c);
                escaped = false;
                continue;
            }

            if (c == '\\')
            {
                escaped = true;
                continue;
            }

            if (c == '"')
            {
                out = tmp;
                unescapeJsonStringInPlace(out);
                return p;
            }

            tmp.push_back(c);
        }

        return nullptr; // unterminated
    }

    void EspIdfCommandParser::unescapeJsonStringInPlace(std::string &s) const
    {
        std::string out;
        out.reserve(s.size());

        for (size_t i = 0; i < s.size(); i++)
        {
            const char c = s[i];
            if (c == '\\' && i + 1 < s.size())
            {
                const char n = s[i + 1];
                switch (n)
                {
                case '"':
                    out.push_back('"');
                    break;
                case '\\':
                    out.push_back('\\');
                    break;
                case '/':
                    out.push_back('/');
                    break;
                case 'n':
                    out.push_back('\n');
                    break;
                case 'r':
                    out.push_back('\r');
                    break;
                case 't':
                    out.push_back('\t');
                    break;
                default:
                    out.push_back(n);
                    break;
                }
                i++;
            }
            else
            {
                out.push_back(c);
            }
        }

        s.swap(out);
    }

    bool EspIdfCommandParser::tryExtractJsonStringField(const char *json, size_t len, const char *key, std::string &out) const
    {
        if (!json || len == 0 || !key)
            return false;

        const char *end = json + len;
        const char *p = findKeyValueStart(json, end, key);
        if (!p)
            return false;

        // Only string values are supported here.
        if (p >= end || *p != '"')
            return false;

        std::string value;
        const char *next = parseJsonString(p, end, value);
        (void)next;
        if (!next)
            return false;

        out = value;
        return true;
    }

    bool EspIdfCommandParser::tryExtractJsonArgs(const char *json, size_t len, std::vector<std::pair<std::string, std::string>> &out) const
    {
        out.clear();

        if (!json || len == 0)
            return false;

        const char *end = json + len;
        const char *p = findKeyValueStart(json, end, "args");
        if (!p)
            return false;

        p = skipWhitespace(p, end);
        if (!p || p >= end || *p != '[')
            return false;

        ++p;
        bool inString = false;
        bool escaped = false;
        int objectDepth = 0;
        const char *objectStart = nullptr;

        while (p < end)
        {
            const char c = *p;

            if (inString)
            {
                if (escaped)
                {
                    escaped = false;
                }
                else if (c == '\\')
                {
                    escaped = true;
                }
                else if (c == '"')
                {
                    inString = false;
                }
                ++p;
                continue;
            }

            if (c == '"')
            {
                inString = true;
            }
            else if (c == '{')
            {
                if (objectDepth == 0)
                    objectStart = p;
                objectDepth++;
            }
            else if (c == '}')
            {
                if (objectDepth > 0)
                {
                    objectDepth--;
                    if (objectDepth == 0 && objectStart)
                    {
                        std::string key;
                        std::string value;
                        const size_t objectLen = (size_t)((p + 1) - objectStart);
                        if (tryExtractJsonStringField(objectStart, objectLen, "key", key) &&
                            tryExtractJsonStringField(objectStart, objectLen, "value", value))
                        {
                            out.emplace_back(key, value);
                        }
                        objectStart = nullptr;
                    }
                }
            }
            else if (c == ']' && objectDepth == 0)
            {
                return true;
            }

            ++p;
        }

        return false;
    }

    iotsmartsys::core::DeviceCommand *EspIdfCommandParser::parseCommand(const char *jsonPayload, size_t payloadLen)
    {
        if (!jsonPayload || payloadLen == 0)
        {
           _logger.error("Failed to parse JSON payload: empty payload.");
            return nullptr;
        }

        std::string capabilityName;
        std::string deviceId;
        std::string value;
        std::string type;
        std::string args1;
        std::string args1value;
        std::vector<std::pair<std::string, std::string>> args;

        const bool okCap = tryExtractJsonStringField(jsonPayload, payloadLen, "capability_name", capabilityName);
        const bool okDev = tryExtractJsonStringField(jsonPayload, payloadLen, "device_id", deviceId);
        const bool okVal = tryExtractJsonStringField(jsonPayload, payloadLen, "value", value);
        const bool okTyp = tryExtractJsonStringField(jsonPayload, payloadLen, "type", type);
        const bool okArgs1 = tryExtractJsonStringField(jsonPayload, payloadLen, "args1", args1);
        const bool okArgs1Value = tryExtractJsonStringField(jsonPayload, payloadLen, "args1value", args1value);
        const bool okArgs = tryExtractJsonArgs(jsonPayload, payloadLen, args);

        _logger.info("CMD", "Parsed command capability='%s' device_id='%s' value='%s' type='%s' args_count=%u args1='%s' args1value='%s'.",
                      okCap ? capabilityName.c_str() : "(missing)",
                      okDev ? deviceId.c_str() : "(missing)",
                      okVal ? value.c_str() : "(missing)",
                      okTyp ? type.c_str() : "(missing)",
                      okArgs ? (unsigned)args.size() : 0,
                      okArgs1 ? args1.c_str() : "(missing)",
                      okArgs1Value ? args1value.c_str() : "(missing)");

        if (!okCap || !okDev || !okVal)
        {
            _logger.error("CMD", "Failed to parse JSON: missing required fields (cap=%s dev=%s val=%s typ=%s).",
                          okCap ? "ok" : "missing",
                          okDev ? "ok" : "missing",
                          okVal ? "ok" : "missing",
                          okTyp ? "ok" : "missing");
            return nullptr;
        }

        iotsmartsys::core::DeviceCommand *outCmd = new iotsmartsys::core::DeviceCommand();
        outCmd->capability_name = capabilityName.c_str();
        outCmd->device_id = deviceId.c_str();
        outCmd->value = value.c_str();
        outCmd->type = okTyp ? type.c_str() : iotsmartsys::core::CommandTypeStrings::CAPABILITY_STR;
        outCmd->args = args;

        return outCmd;
    }
}
