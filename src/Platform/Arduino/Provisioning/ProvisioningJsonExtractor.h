#pragma once

#include <Arduino.h>

namespace iotsmartsys::core::provisioning
{
    struct ProvisioningJsonFields
    {
        String ssid;
        String password;
        String deviceApiKey;
        String basicAuth;
        String deviceApiUrl;
    };

    class ProvisioningJsonExtractor
    {
    public:
        static bool tryParse(const String &json, ProvisioningJsonFields &out);

    private:
        static bool tryExtractJsonStringField(const char *json, const char *key, String &out);
        static bool tryExtractJsonStringField(const String &json, const char *key, String &out);

        // Minimal string unescape for JSON values (\" \\ \/ \n \r \t)
        static void unescapeJsonStringInPlace(String &s);

        static const char *skipWhitespace(const char *p);
        static const char *findKey(const char *json, const char *key);
        static const char *parseJsonString(const char *p, String &out);
    };
} // namespace iotsmartsys::core::provisioning