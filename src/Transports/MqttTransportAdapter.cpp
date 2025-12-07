#include "Transports/MqttTransportAdapter.h"

MqttTransportAdapter::MqttTransportAdapter(const char *device_name,
                                           MqttSettings mqttSettings,
                                           const std::vector<Capability *> &capabilities,
                                           const std::vector<Property *> &properties)
    : mqtt(device_name, mqttSettings, capabilities, properties) {}

void MqttTransportAdapter::setup() { mqtt.setup(); }

void MqttTransportAdapter::handle() { mqtt.handle(); }

void MqttTransportAdapter::sendState(CapabilityState state) { mqtt.sendState(state); }

void MqttTransportAdapter::sendMessage(const char *topic, String payload) { mqtt.sendMqttMessage(topic, payload); }

bool MqttTransportAdapter::isPowerOn() { return mqtt.isPowerOn(); }

bool MqttTransportAdapter::isConnected() { return mqtt.isConnected(); }

bool MqttTransportAdapter::subscribeDeviceCommand(const String &device_id) {
    return mqtt.subscribeDeviceCommand(device_id);
}
void MqttTransportAdapter::subscribe(const char *topic) { mqtt.subscribe(topic); }
