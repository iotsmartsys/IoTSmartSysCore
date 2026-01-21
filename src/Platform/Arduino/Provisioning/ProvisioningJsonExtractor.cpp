#include "Platform/Arduino/Provisioning/ProvisioningJsonExtractor.h"

namespace iotsmartsys::core::provisioning
{
    const char *ProvisioningJsonExtractor::skipWhitespace(const char *p)
    {
        while (p && *p)
        {
            const char c = *p;
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
                break;
            ++p;
        }
        return p;
    }

    // Finds the occurrence of:  "key"   :
    // Returns pointer positioned right after the ':' (whitespace not consumed after ':')
    const char *ProvisioningJsonExtractor::findKey(const char *json, const char *key)
    {
        if (!json || !key)
            return nullptr;

        // Build pattern: "key"
        // We'll do a simple scan for '"' then compare key
        const size_t keyLen = strlen(key);
        const char *p = json;

        while ((p = strchr(p, '\"')) != nullptr)
        {
            ++p; // after opening quote
            // Compare key
            if (strncmp(p, key, keyLen) == 0 && p[keyLen] == '\"')
            {
                const char *afterQuote = p + keyLen + 1;
                afterQuote = skipWhitespace(afterQuote);
                if (afterQuote && *afterQuote == ':')
                {
                    return afterQuote + 1; // after ':'
                }
            }
            // continue scanning from current position
            ++p;
        }

        return nullptr;
    }

    // Parses a JSON string beginning at p (expects p points to opening quote)
    // Returns pointer to char after closing quote on success, nullptr on failure
    const char *ProvisioningJsonExtractor::parseJsonString(const char *p, String &out)
    {
        p = skipWhitespace(p);
        if (!p || *p != '\"')
            return nullptr;

        ++p; // skip opening quote
        String tmp;
        tmp.reserve(64);

        while (*p)
        {
            const char c = *p;

            if (c == '\"')
            {
                // end of string
                out = tmp;
                unescapeJsonStringInPlace(out);
                return p + 1;
            }

            if (c == '\\')
            {
                // keep escape sequence as-is, we'll unescape later
                if (*(p + 1) == '\0')
                    return nullptr;
                tmp += c;
                tmp += *(p + 1);
                p += 2;
                continue;
            }

            tmp += c;
            ++p;
        }

        return nullptr; // unterminated string
    }

    void ProvisioningJsonExtractor::unescapeJsonStringInPlace(String &s)
    {
        // Minimal unescape: \" \\ \/ \n \r \t
        String out;
        out.reserve(s.length());

        for (size_t i = 0; i < s.length(); i++)
        {
            const char c = s[i];
            if (c == '\\' && i + 1 < s.length())
            {
                const char n = s[i + 1];
                switch (n)
                {
                case '\"':
                    out += '\"';
                    break;
                case '\\':
                    out += '\\';
                    break;
                case '/':
                    out += '/';
                    break;
                case 'n':
                    out += '\n';
                    break;
                case 'r':
                    out += '\r';
                    break;
                case 't':
                    out += '\t';
                    break;
                default:
                    // unknown escape, keep literal
                    out += n;
                    break;
                }
                i++; // skip next
            }
            else
            {
                out += c;
            }
        }

        s = out;
    }

    bool ProvisioningJsonExtractor::tryExtractJsonStringField(const String &json, const char *key, String &out)
    {
        return tryExtractJsonStringField(json.c_str(), key, out);
    }

    bool ProvisioningJsonExtractor::tryExtractJsonStringField(const char *json, const char *key, String &out)
    {
        const char *p = findKey(json, key);
        if (!p)
            return false;

        p = skipWhitespace(p);
        // We only support string values for now: "value"
        if (!p || *p != '\"')
            return false;

        String value;
        const char *end = parseJsonString(p, value);
        if (!end)
            return false;

        out = value;
        return true;
    }

    bool ProvisioningJsonExtractor::tryParse(const String &json, ProvisioningJsonFields &out)
    {
        // Basic sanity: must look like an object
        const String trimmed = String(json);
        const char *p = skipWhitespace(trimmed.c_str());
        if (!p || *p != '{')
            return false;

        // Extract known fields (all optional except ssid which the caller validates)
        (void)tryExtractJsonStringField(trimmed, "ssid", out.ssid);
        (void)tryExtractJsonStringField(trimmed, "password", out.password);
        (void)tryExtractJsonStringField(trimmed, "device_api_key", out.deviceApiKey);
        (void)tryExtractJsonStringField(trimmed, "basic_auth", out.basicAuth);
        (void)tryExtractJsonStringField(trimmed, "device_api_url", out.deviceApiUrl);

        // If we didn't find anything, consider invalid JSON for our expected shape
        if (out.ssid.length() == 0 &&
            out.password.length() == 0 &&
            out.deviceApiKey.length() == 0 &&
            out.basicAuth.length() == 0 &&
            out.deviceApiUrl.length() == 0)
        {
            return false;
        }

        return true;
    }

} // namespace iotsmartsys::core::provisioning