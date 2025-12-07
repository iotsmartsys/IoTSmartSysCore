#pragma once
#include <Arduino.h>
#include <vector>

class CapabilityState
{
public:
    String capability_name;
    String device_id;
    String value;
    String type;

    CapabilityState() {}
    CapabilityState(String capability_name, String value, String type)
        : capability_name(capability_name), value(value), type(type) {}
    CapabilityState(String device_id, String capability_name, String value, String type)
        : device_id(device_id), capability_name(capability_name), value(value), type(type) {}

    String toJson()
    {
        String payload = "{ \"device_id\":\"" + String(device_id) + "\",\"capability_name\":\"" + String(capability_name) + "\",\"value\":\"" + String(value) + "\",\"type\":\"" + String(type) + "\"}";

        return payload;
    }
};