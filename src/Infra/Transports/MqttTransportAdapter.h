#pragma once

#include <vector>
#include "Infra/Transports/IMessageClient.h"
#include "Infra/Mqtt/MqttClientHandler.h"


class MqttTransportAdapter : public IMessageClient {
public:
    MqttTransportAdapter(const char *device_name,
                         MqttSettings mqttSettings,
                         const std::vector<Capability *> &capabilities,
                         const std::vector<Property *> &properties);

    void setup() override;
    void handle() override;

    void sendState(CapabilityState state) override;
    void sendMessage(const char *topic, String payload) override;

    bool isPowerOn() override;
    bool isConnected() override;

    
    bool subscribeDeviceCommand(const String &device_id);
    void subscribe(const char *topic) override;

private:
    MqttClientHandler mqtt;
};
