#pragma once

#include "Contracts/Events/ICapabilityEventSink.h"
#include "Contracts/Transports/IMqttClient.h"
#include "Contracts/Settings/IReadOnlySettingsProvider.h"

using namespace iotsmartsys::core::settings;
namespace iotsmartsys::core
{
    class MqttSink : public ICapabilityEventSink
    {
    public:
        MqttSink(IMqttClient &mqttClient, IReadOnlySettingsProvider &settingsProvider);
        virtual ~MqttSink() = default;

        void onStateChanged(const CapabilityStateChanged &ev) override;

    private:
        IMqttClient &mqttClient;
        IReadOnlySettingsProvider &settingsProvider;
    };

} // namespace iotsmartsys::core