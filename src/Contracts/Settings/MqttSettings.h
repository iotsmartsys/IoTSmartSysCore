#pragma once

#include <string>
#include "MqttConfig.h"

namespace iotsmartsys::core::settings
{

    struct MqttSettings
    {
        MqttConfig primary;
        MqttConfig secondary;
        std::string announce_topic{"smarthome/discovery"};
        std::string command_topic{"device/{device_id}/command"};
        std::string notify_topic{"device/state"};

        bool hasSecondary() const
        {
            return !secondary.host.empty();
        }

        // Example device/{device_id}/command
        std::string getCommandTopicForDevice(const std::string &device_id) const
        {
            std::string topic = command_topic;
            if (topic.find("{device_id}") != std::string::npos)
            {
                topic.replace(topic.find("{device_id}"), 12, device_id);
            }
            return topic;
        }
    };

} // namespace iotsmartsys::core::settings
