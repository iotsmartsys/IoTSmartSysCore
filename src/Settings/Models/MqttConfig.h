#pragma once
#include <Arduino.h>

struct MqttConfig
{
    String host;
    int port{1883};
    String user;
    String password;
    String protocol;
    int ttl{0};

    unsigned long getReconnectIntervalMs() const
    {
        return ttl * 60000UL;
    }
};