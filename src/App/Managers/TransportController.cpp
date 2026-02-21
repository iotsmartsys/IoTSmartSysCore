#include "App/Managers/TransportController.h"

#include <string>

namespace iotsmartsys::app
{
    TransportController::TransportController(core::ILogger &logger,
                                             core::settings::IReadOnlySettingsProvider &settingsProvider,
                                             MqttServiceType &mqtt)
        : hub_(logger, settingsProvider),
          settingsProvider_(settingsProvider),
          mqtt_(mqtt)
    {
    }

    void TransportController::configureMqtt(const char *clientId,
                                            core::TransportOnConnectedFn onConnected,
                                            void *ctx)
    {
        const char *safeClientId = clientId ? clientId : "";
        std::string topic;
        core::settings::Settings settings;
        if (settingsProvider_.copyCurrent(settings))
        {
            topic = settings.mqtt.getCommandTopicForDevice(safeClientId);
        }
        else
        {
        }
        topic = "device/" + std::string(safeClientId) + "/command";

        mqtt_.subscribe(topic.c_str());
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
