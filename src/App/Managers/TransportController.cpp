#include "App/Managers/TransportController.h"

#include <cstdio>

namespace iotsmartsys::app
{
    TransportController::TransportController(core::ILogger &logger,
                                             core::settings::IReadOnlySettingsProvider &settingsProvider,
                                             MqttServiceType &mqtt)
        : hub_(logger, settingsProvider),
          mqtt_(mqtt)
    {
    }

    void TransportController::configureMqtt(const char *clientId,
                                            core::TransportOnConnectedFn onConnected,
                                            void *ctx)
    {
        const char *safeClientId = clientId ? clientId : "";
        char topic[128];
        snprintf(topic, sizeof(topic), "device/%s/command", safeClientId);

        mqtt_.subscribe(topic);
        mqtt_.setOnConnected(onConnected, ctx);
        mqtt_.setForwardRawMessages(true);
    }

    void TransportController::addDispatcher(core::ITransportDispatcher &dispatcher)
    {
        hub_.addDispatcher(dispatcher);
    }

    void TransportController::addChannel(const char *name, core::ITransportChannel *channel)
    {
        hub_.addChannel(name, channel);
    }

    void TransportController::start()
    {
        if (!mqttAdded_)
        {
            hub_.addChannel("mqtt", &mqtt_);
            mqttAdded_ = true;
        }
        hub_.start();
    }

    void TransportController::handle()
    {
        hub_.handle();
    }
} // namespace iotsmartsys::app
