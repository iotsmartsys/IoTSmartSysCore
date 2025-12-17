#pragma once

#include <string>

namespace iotsmartsys::core
{
    class CapabilityStateChanged
    {
    public:
        std::string capability_name;
        std::string device_id;
        std::string value;
        std::string type;

        CapabilityStateChanged() {}
        CapabilityStateChanged(std::string capability_name, std::string value, std::string type)
            : capability_name(capability_name), value(value), type(type) {}
        CapabilityStateChanged(std::string device_id, std::string capability_name, std::string value, std::string type)
            : device_id(device_id), capability_name(capability_name), value(value), type(type) {}

        std::string toJson()
        {
            std::string payload = "{ \"device_id\":\"" + device_id + "\",\"capability_name\":\"" + capability_name + "\",\"value\":\"" + value + "\",\"type\":\"" + type + "\"}";

            return payload;
        }
    };

} // namespace iotsmartsys::core