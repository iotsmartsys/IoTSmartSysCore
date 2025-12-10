#pragma once
#include <Arduino.h>
#include "MqttConfig.h"

struct MqttSettings
{
    MqttConfig primary;
    MqttConfig secondary;
    String announce_topic{"smarthome/discovery"};
    String command_topic{"device/{device_id}/command"};
    String notify_topic{"device/state"};

    bool hasSecondary() const
    {
        return !secondary.host.isEmpty() && secondary.port != 0;
    }

    // Example device/{device_id}/command
    String getCommandTopicForDevice(const String &device_id) const
    {
        String topic = command_topic;
        if (topic.indexOf("{device_id}") != -1)
        {
            topic.replace("{device_id}", device_id);
        }
        return topic;
    }
};
