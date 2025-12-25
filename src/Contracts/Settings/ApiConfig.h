#pragma once
#include <string>

namespace iotsmartsys::core::settings
{

    struct ApiConfig
    {
        std::string url;
        std::string key;
        std::string basic_auth;

        bool hasChanged(const ApiConfig &other) const
        {
            return (key != other.key || basic_auth != other.basic_auth || url != other.url);
        }
    };

} // namespace iotsmartsys::core::settings