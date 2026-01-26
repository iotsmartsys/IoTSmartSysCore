#include "Platform/Common/Json/JsonPathExtractor.h"

#include <cstring>

namespace iotsmartsys::platform::common::json
{
    JsonPathExtractor::JsonPathExtractor(const char *json, size_t len)
        : m_begin(json), m_end(json + len)
    {
    }

    const char *JsonPathExtractor::skipWhitespace(const char *p, const char *end) const
    {
        while (p < end && (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t'))
            ++p;
        return p;
    }

    // parse a quoted JSON string from p (pointing at opening '"')
    const char *JsonPathExtractor::parseJsonString(const char *p, const char *end, std::string &out) const
    {
        out.clear();
        if (p >= end || *p != '"')
            return nullptr;
        ++p; // skip '"'
        while (p < end)
        {
            char c = *p++;
            if (c == '"')
            {
                return p; // return position after closing quote
            }
            if (c == '\\')
            {
                if (p >= end)
                    return nullptr;
                char esc = *p++;
                switch (esc)
                {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                default:
                    // unsupported escape, fail gracefully
                    return nullptr;
                }
            }
            else
            {
                out.push_back(c);
            }
        }
        return nullptr; // unterminated string
    }

    void JsonPathExtractor::unescapeJsonStringInPlace(std::string &s) const
    {
        // parseJsonString already unescapes; this is kept for API completeness
        (void)s;
    }

    // find a key inside an object; returns pointer to value start and its end via valueEnd
    const char *JsonPathExtractor::findKeyInObject(const char *objStart, const char *objEnd, const char *key, const char **valueStart, const char **valueEnd) const
    {
        const char *p = skipWhitespace(objStart, objEnd);
        if (p >= objEnd || *p != '{')
            return nullptr;
        ++p; // skip '{'
        while (true)
        {
            p = skipWhitespace(p, objEnd);
            if (p >= objEnd)
                return nullptr;
            if (*p == '}')
            {
                return nullptr; // end of object, not found
            }
            // Expecting string key
            if (*p != '"')
                return nullptr;
            // parse key
            std::string foundKey;
            const char *afterKey = parseJsonString(p, objEnd, foundKey);
            if (!afterKey)
                return nullptr;
            p = skipWhitespace(afterKey, objEnd);
            if (p >= objEnd || *p != ':')
                return nullptr;
            ++p; // skip ':'
            p = skipWhitespace(p, objEnd);

            // value could be object/string/bool/number
            const char *valStart = p;
            const char *valEnd = p;

            if (*p == '{')
            {
                // find matching '}' by counting braces
                int depth = 0;
                while (p < objEnd)
                {
                    if (*p == '{') depth++;
                    else if (*p == '}')
                    {
                        depth--;
                        if (depth == 0)
                        {
                            valEnd = p + 1;
                            break;
                        }
                    }
                    else if (*p == '"')
                    {
                        // skip inner strings to avoid confusing braces inside strings
                        std::string tmp;
                        const char *after = parseJsonString(p, objEnd, tmp);
                        if (!after) return nullptr;
                        p = after;
                        continue;
                    }
                    ++p;
                }
                if (valEnd == valStart)
                    return nullptr;
            }
            else if (*p == '"')
            {
                std::string tmp;
                const char *after = parseJsonString(p, objEnd, tmp);
                if (!after) return nullptr;
                valEnd = after;
            }
            else
            {
                // bool or number or literal
                // read until comma or closing brace
                while (p < objEnd && *p != ',' && *p != '}')
                    ++p;
                valEnd = p;
            }

            // compare keys
            if (foundKey == key)
            {
                if (valueStart) *valueStart = valStart;
                if (valueEnd) *valueEnd = valEnd;
                return valStart;
            }

            // skip comma if present
            p = skipWhitespace(valEnd, objEnd);
            if (p < objEnd && *p == ',')
            {
                ++p;
                continue;
            }
            // otherwise loop continues and will hit '}' or error
        }
        return nullptr;
    }

    const char *JsonPathExtractor::findKeyValueStart(const char *json, const char *end, const char *key) const
    {
        // json should point to an object
        const char *vstart = nullptr;
        const char *vend = nullptr;
        if (findKeyInObject(json, end, key, &vstart, &vend))
            return vstart;
        return nullptr;
    }

    bool JsonPathExtractor::tryExtractJsonStringField(const char *json, size_t len, const char *key, std::string &out) const
    {
        JsonPathExtractor ext(json, len);
        const char *vstart = ext.findKeyValueStart(json, json + len, key);
        if (!vstart) return false;
        // must be string
        if (*vstart != '"') return false;
        const char *after = ext.parseJsonString(vstart, json + len, out);
        return after != nullptr;
    }

    bool JsonPathExtractor::getString(const char *path, std::string &out) const
    {
        // split path by dots
        // iterate nesting
        const char *p = m_begin;
        const char *end = m_end;
        std::string key;
        const char *curObjStart = m_begin;
        const char *curObjEnd = m_end;

        const char *seg = path;
        while (true)
        {
            // extract next segment
            const char *dot = strchr(seg, '.');
            if (dot)
                key.assign(seg, dot - seg);
            else
                key.assign(seg);

            const char *valStart = nullptr;
            const char *valEnd = nullptr;
            if (!findKeyInObject(curObjStart, curObjEnd, key.c_str(), &valStart, &valEnd))
                return false;

            if (!dot)
            {
                // final key - must be string
                if (!valStart || *valStart != '"') return false;
                const char *after = parseJsonString(valStart, curObjEnd, out);
                return after != nullptr;
            }
            else
            {
                // intermediate - must be object
                if (!valStart || *valStart != '{') return false;
                curObjStart = valStart;
                curObjEnd = valEnd;
                seg = dot + 1;
                continue;
            }
        }
        return false;
    }

    bool JsonPathExtractor::getInt(const char *path, int &out) const
    {
        std::string sval;
        if (!getString(path, sval))
        {
            // maybe number literal (unquoted)
            // find value start for final key
            // split path
            const char *seg = path;
            std::string key;
            const char *curObjStart = m_begin;
            const char *curObjEnd = m_end;
            while (true)
            {
                const char *dot = strchr(seg, '.');
                if (dot)
                    key.assign(seg, dot - seg);
                else
                    key.assign(seg);

                const char *valStart = nullptr;
                const char *valEnd = nullptr;
                if (!findKeyInObject(curObjStart, curObjEnd, key.c_str(), &valStart, &valEnd))
                    return false;

                if (!dot)
                {
                    // final key - valStart should point to number or '-'
                    if (!valStart) return false;
                    const char *q = skipWhitespace(valStart, curObjEnd);
                    // parse integer
                    bool neg = false;
                    if (*q == '-') { neg = true; ++q; }
                    if (q >= curObjEnd || (*q < '0' || *q > '9')) return false;
                    long val = 0;
                    while (q < curObjEnd && *q >= '0' && *q <= '9')
                    {
                        val = val * 10 + (*q - '0');
                        ++q;
                    }
                    out = neg ? -((int)val) : (int)val;
                    return true;
                }
                else
                {
                    if (!valStart || *valStart != '{') return false;
                    curObjStart = valStart;
                    curObjEnd = valEnd;
                    seg = dot + 1;
                    continue;
                }
            }
        }
        // sval obtained, parse as int
        char *endptr = nullptr;
        long v = strtol(sval.c_str(), &endptr, 10);
        if (!endptr || *endptr != '\0') return false;
        out = (int)v;
        return true;
    }

    bool JsonPathExtractor::getBool(const char *path, bool &out) const
    {
        // try to extract as unquoted literal true/false
        const char *seg = path;
        std::string key;
        const char *curObjStart = m_begin;
        const char *curObjEnd = m_end;
        while (true)
        {
            const char *dot = strchr(seg, '.');
            if (dot)
                key.assign(seg, dot - seg);
            else
                key.assign(seg);

            const char *valStart = nullptr;
            const char *valEnd = nullptr;
            if (!findKeyInObject(curObjStart, curObjEnd, key.c_str(), &valStart, &valEnd))
                return false;

            if (!dot)
            {
                if (!valStart) return false;
                const char *q = skipWhitespace(valStart, curObjEnd);
                // check for true/false
                if ((q + 4) <= curObjEnd && strncmp(q, "true", 4) == 0)
                {
                    out = true;
                    return true;
                }
                if ((q + 5) <= curObjEnd && strncmp(q, "false", 5) == 0)
                {
                    out = false;
                    return true;
                }
                return false;
            }
            else
            {
                if (!valStart || *valStart != '{') return false;
                curObjStart = valStart;
                curObjEnd = valEnd;
                seg = dot + 1;
                continue;
            }
        }
        return false;
    }

    bool JsonPathExtractor::getObjectRange(const char *path, size_t &begin, size_t &end) const
    {
        // find object for path and return offsets
        const char *seg = path;
        std::string key;
        const char *curObjStart = m_begin;
        const char *curObjEnd = m_end;
        while (true)
        {
            const char *dot = strchr(seg, '.');
            if (dot)
                key.assign(seg, dot - seg);
            else
                key.assign(seg);

            const char *valStart = nullptr;
            const char *valEnd = nullptr;
            if (!findKeyInObject(curObjStart, curObjEnd, key.c_str(), &valStart, &valEnd))
                return false;

            if (!dot)
            {
                if (!valStart || *valStart != '{') return false;
                begin = (size_t)(valStart - m_begin);
                end = (size_t)(valEnd - m_begin);
                return true;
            }
            else
            {
                if (!valStart || *valStart != '{') return false;
                curObjStart = valStart;
                curObjEnd = valEnd;
                seg = dot + 1;
                continue;
            }
        }
        return false;
    }
}
