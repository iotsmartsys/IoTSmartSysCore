#pragma once

#include "Contracts/Events/ICapabilityEventSink.h"
#include "Contracts/Transports/IMqttClient.h"

namespace iotsmartsys::core
{
    class MqttSink : public ICapabilityEventSink
    {
    public:
        MqttSink(IMqttClient &mqttClient);
        virtual ~MqttSink() = default;

        void onStateChanged(const CapabilityStateChanged &ev) override;

    private:
        IMqttClient &mqttClient;
    };

} // namespace iotsmartsys::core