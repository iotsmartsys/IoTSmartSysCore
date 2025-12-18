#pragma once
#include <string>

namespace iotsmartsys::core::settings
{

    struct ApiConfig
    {
        std::string key;
        std::string basic_auth;
    };

} // namespace iotsmartsys::core::settings