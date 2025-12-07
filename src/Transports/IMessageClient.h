#pragma once

#include <Arduino.h>
#include <vector>
#include "Capabilities/CapabilityState.h"

class IMessageClient {
public:
    virtual ~IMessageClient() = default;

    virtual void setup() = 0;
    virtual void handle() = 0;

    virtual void sendState(CapabilityState state) = 0;
    virtual void sendMessage(const char *topic, String payload) = 0;

    virtual bool isPowerOn() = 0;
    virtual bool isConnected() = 0;

    virtual void subscribe(const char *topic) = 0;
};

