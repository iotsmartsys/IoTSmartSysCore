#pragma once

#include "Contracts/Settings/IReadOnlySettingsProvider.h"
#include "Contracts/Transports/ITransportChannel.h"
#include "Contracts/Transports/ITransportDispatcher.h"
#include "Core/Services/MqttService.h"
#include "Core/Transports/TransportHub.h"

namespace iotsmartsys::app
{
    class TransportController
    {
    public:
        using MqttServiceType = app::MqttService<12, 16, 256>;

        TransportController(core::ILogger &logger,
                            core::settings::IReadOnlySettingsProvider &settingsProvider,
                            MqttServiceType &mqtt);

        void configureMqtt(const char *clientId,
                           core::TransportOnConnectedFn onConnected,
                           void *ctx);
        void addDispatcher(core::ITransportDispatcher &dispatcher);
        void addChannel(const char *name, core::ITransportChannel *channel);
        void start();
        void handle();

    private:
        core::TransportHub hub_;
        core::settings::IReadOnlySettingsProvider &settingsProvider_;
        MqttServiceType &mqtt_;
        bool mqttAdded_{false};
    };
} // namespace iotsmartsys::app
