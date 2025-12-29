#pragma once

#include <string>

namespace iotsmartsys::ota
{

    struct ManifestInfo
    {
    public:
        std::string module;
        std::string env;
        std::string version;
        std::string urlPath; // path do .bin
        std::string fullUrl; // URL final completa
        std::string checksumType;
        std::string checksumValue;
        size_t size = 0;
        bool mandatory = false;
        bool valid = false;

        bool isValid() const
        {
            return valid;
        }

        int compare(const std::string &actualVersion) const
        {
            long long remote = parseVersionNumber(this->version);
            long long current = parseVersionNumber(actualVersion);

            if (remote < current)
                return -1;
            if (remote > current)
                return 1;
            else
                return 0;
        }

    private:
        long long parseVersionNumber(const std::string &v) const
        {
            std::string digits;
            digits.reserve(v.length());
            for (size_t i = 0; i < v.length(); ++i)
            {
                char c = v[i];
                if (c >= '0' && c <= '9')
                {
                    digits += c;
                }
            }
            if (digits.length() == 0)
                return 0;
            return atoll(digits.c_str());
        }
    };
} // namespace iotsmartsys::ota