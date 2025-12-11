#pragma once

#include <string>

namespace iotsmartsys::core
{

    class ICapabilityState
    {
    public:
        std::string capability_name;
        std::string device_id;
        std::string value;
        std::string type;

        ICapabilityState() {}
        ICapabilityState(std::string capability_name, std::string value, std::string type)
            : capability_name(capability_name), value(value), type(type) {}
        ICapabilityState(std::string device_id, std::string capability_name, std::string value, std::string type)
            : device_id(device_id), capability_name(capability_name), value(value), type(type) {}

        std::string toJson()
        {
            std::string payload = "{ \"device_id\":\"" + device_id + "\",\"capability_name\":\"" + capability_name + "\",\"value\":\"" + value + "\",\"type\":\"" + type + "\"}";

            return payload;
        }
    };

} // namespace iotsmartsys::core