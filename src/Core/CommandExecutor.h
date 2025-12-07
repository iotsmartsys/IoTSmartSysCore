#pragma once

#include <Arduino.h>
#include <vector>
#include "Capabilities/Capability.h"
#include "Capabilities/CapabilityCommand.h"
#include "Mqtt/MqttClientHandler.h"

void applyCommandToCapabilities(
        const CapabilityCommand &command,
        std::vector<Capability *> &capabilities,
        bool &power_on,
        MqttClientHandler *mqttHandler);

