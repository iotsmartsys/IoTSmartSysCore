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
        CapabilityStateChanged(const char *capability_name, const char *value, const char *type)
            : capability_name(capability_name), value(value), type(type) {}
        CapabilityStateChanged(const char *device_id, const char *capability_name, const char *value, const char *type)
            : device_id(device_id), capability_name(capability_name), value(value), type(type) {}

        // Convenience overloads to accept std::string without breaking callers
        CapabilityStateChanged(const std::string &capability_name, const std::string &value, const std::string &type)
            : CapabilityStateChanged(capability_name.c_str(), value.c_str(), type.c_str()) {}

        CapabilityStateChanged(const std::string &device_id, const std::string &capability_name, const std::string &value, const std::string &type)
            : CapabilityStateChanged(device_id.c_str(), capability_name.c_str(), value.c_str(), type.c_str()) {}

        std::string toJson()
        {
            std::string payload = "{ \"device_id\":\"" + device_id + "\",\"capability_name\":\"" + capability_name + "\",\"value\":\"" + value + "\",\"type\":\"" + type + "\"}";

            return payload;
        }
    };

} // namespace iotsmartsys::core