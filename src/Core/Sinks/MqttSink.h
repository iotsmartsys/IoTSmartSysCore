#pragma once

#include "Contracts/Events/ICapabilityEventSink.h"
#include "Contracts/Transports/ITransportChannel.h"
#include "Contracts/Settings/IReadOnlySettingsProvider.h"

using namespace iotsmartsys::core::settings;
namespace iotsmartsys::core
{
    class MqttSink : public ICapabilityEventSink
    {
    public:
        MqttSink(ITransportChannel &mqttClient, IReadOnlySettingsProvider &settingsProvider);
        virtual ~MqttSink() = default;

        void onStateChanged(const CapabilityStateChanged &ev) override;

    private:
        ITransportChannel &mqttClient;
        IReadOnlySettingsProvider &settingsProvider;
    };

} // namespace iotsmartsys::core