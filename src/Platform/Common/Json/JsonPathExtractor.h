#pragma once

#include <string>
#include <cstddef>

namespace iotsmartsys::platform::common::json
{
    // Minimal JSON extractor tailored for the project's needs.
    // - No external deps
    // - Supports objects, strings, integers, booleans
    // - No arrays, no floats, no unicode escapes
    class JsonPathExtractor
    {
    public:
        JsonPathExtractor(const char *json, size_t len);

        // Getters. Return true if field exists and was parsed into out.
        bool getString(const char *path, std::string &out) const;
        bool getInt(const char *path, int &out) const;
        bool getBool(const char *path, bool &out) const;

        // Optional: retrieve raw object range for a path (begin offset, end offset)
        bool getObjectRange(const char *path, size_t &begin, size_t &end) const;

    private:
        const char *m_begin;
        const char *m_end;

        const char *skipWhitespace(const char *p, const char *end) const;
        const char *findKeyValueStart(const char *json, const char *end, const char *key) const;
        const char *parseJsonString(const char *p, const char *end, std::string &out) const;
        void unescapeJsonStringInPlace(std::string &s) const;
        bool tryExtractJsonStringField(const char *json, size_t len, const char *key, std::string &out) const;

        // Helpers used internally
        const char *findKeyInObject(const char *objStart, const char *objEnd, const char *key, const char **valueStart, const char **valueEnd) const;
    };
}
