#pragma once

#include <string>

namespace iotsmartsys::core
{
    class DeviceCommand
    {
    public:
        const char *capability_name;
        const char *device_id;
        const char *value;
    };

}