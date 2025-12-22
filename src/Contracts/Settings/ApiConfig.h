#pragma once
#include <string>

namespace iotsmartsys::core::settings
{

    struct ApiConfig
    {
        std::string key;
        std::string basic_auth;

        bool hasChanged(const ApiConfig &other) const
        {
            return (key != other.key || basic_auth != other.basic_auth);
        }
    };

} // namespace iotsmartsys::core::settings