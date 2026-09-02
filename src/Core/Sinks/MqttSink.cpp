#include "MqttSink.h"
#include "Contracts/Providers/ServiceProvider.h"

namespace iotsmartsys::core
{

    MqttSink::MqttSink(ITransportChannel &mqttClient, IReadOnlySettingsProvider &settingsProvider)
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
        std::string payload = "{ \"device_id\":\"" + std::string(currentSettings.clientId) + "\",\"capability_name\":\"" + ev.capability_name + "\",\"value\":\"" + ev.value + "\",\"type\":\"" + ev.type + "\"";
        if (ev.measurementStatus)
        {
            payload += ",\"measurementStatus\":\"" + *ev.measurementStatus + "\"";
        }
        if (ev.supplyStatus)
        {
            payload += ",\"supplyStatus\":\"" + *ev.supplyStatus + "\"";
        }
        if (ev.energyWh)
        {
            payload += ",\"energyWh\":\"" + *ev.energyWh + "\"";
        }
        payload += "}";

        mqttClient.publish(topic.c_str(),
                           payload.c_str(),
                           payload.length(),
                           false);
    }

} // namespace iotsmartsys::core
