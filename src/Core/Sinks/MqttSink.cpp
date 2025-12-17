#include "MqttSink.h"

namespace iotsmartsys::core
{

    MqttSink::MqttSink(IMqttClient &mqttClient)
        : mqttClient(mqttClient)
    {
    }

    void MqttSink::onStateChanged(const CapabilityStateChanged &ev)
    {
        std::string topic = "device/state";
        std::string payload = "{ \"device_id\":\"" + ev.device_id + "\",\"capability_name\":\"" + ev.capability_name + "\",\"value\":\"" + ev.value + "\",\"type\":\"" + ev.type + "\"}";

        mqttClient.publish(topic.c_str(),
                           payload.c_str(),
                           payload.length(),
                           false);
    }

} // namespace iotsmartsys::core