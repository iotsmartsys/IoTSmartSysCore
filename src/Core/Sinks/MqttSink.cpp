#include "MqttSink.h"
#include "Contracts/Providers/ServiceProvider.h"

namespace iotsmartsys::core
{

    MqttSink::MqttSink(IMqttClient &mqttClient, IReadOnlySettingsProvider &settingsProvider)
        : mqttClient(mqttClient), settingsProvider(settingsProvider)
    {
    }

    void MqttSink::onStateChanged(const CapabilityStateChanged &ev)
    {
        Settings currentSettings;
        if(!settingsProvider.copyCurrent(currentSettings))
        {
            return;
        }

        std::string topic = currentSettings.mqtt.notify_topic;
        std::string payload = "{ \"device_id\":\"" + std::string(currentSettings.clientId) + "\",\"capability_name\":\"" + ev.capability_name + "\",\"value\":\"" + ev.value + "\",\"type\":\"" + ev.type + "\"}";

        mqttClient.publish(topic.c_str(),
                           payload.c_str(),
                           payload.length(),
                           false);
    }

} // namespace iotsmartsys::core