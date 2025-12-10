#pragma once
#include <Arduino.h>
#include <vector>

class PropertyState
{
public:
    String property_name;
    String device_id;
    String value;
    const String type = "property";

    PropertyState() {}
    PropertyState(String device_id, String property_name, String value)
        : device_id(device_id), property_name(property_name), value(value) {}

    String toJson()
    {
        String payload = "{ \"device_id\":\"" + String(device_id) + "\",\"property_name\":\"" + String(property_name) + "\",\"value\":\"" + String(value) + "\",\"type\":\"" + type + "\"}";

        return payload;
    }
};