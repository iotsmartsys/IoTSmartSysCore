#include "TransportHub.h"
#include <cstring>

namespace iotsmartsys::core
{

    TransportHub::TransportHub(ILogger &logger, IReadOnlySettingsProvider &settingsProvider)
        : logger_(logger), _settingsProvider(settingsProvider)
    {
    }

    TransportHub::~TransportHub()
    {
        clearChannels();
    }

    bool TransportHub::addChannel(const char *name, ITransportChannel *channel)
    {
        if (getChannel(name) != nullptr)
        {
            return false; // já existe
        }

        channel->setOnMessage(
            [](void *user, const TransportMessageView &msg)
            {
                static_cast<TransportHub *>(user)->onMessageReceived(user, msg);
            },
            this);
        ChannelEntry *entry = new ChannelEntry{name, channel};

        channels_.push_back(entry);

        return true;
    }

    ITransportChannel *TransportHub::getChannel(const char *name)
    {
        for (const auto &entry : channels_)
        {
            if (strcmp(entry->name, name) == 0)
            {
                return entry->channel;
            }
        }
        return nullptr;
    }

    void TransportHub::removeChannel(const char *name)
    {
        for (auto it = channels_.begin(); it != channels_.end(); ++it)
        {
            if (strcmp((*it)->name, name) == 0)
            {
                delete *it;
                channels_.erase(it);
                return;
            }
        }
    }

    void TransportHub::clearChannels()
    {
        for (auto &entry : channels_)
        {
            delete entry;
        }
        channels_.clear();
    }

    void TransportHub::start()
    {
        for (const auto &entry : channels_)
        {
            entry->channel->start();
        }
    }

    void TransportHub::stop()
    {
        for (const auto &entry : channels_)
        {
            entry->channel->stop();
        }
    }

    void TransportHub::handle()
    {
        for (const auto &entry : channels_)
        {
            entry->channel->handle();
        }
    }

    void TransportHub::onMessageReceived(void *, const TransportMessageView &msg)
    {
        auto &settings = _settingsProvider.getSettings();

        for (auto &dispatcher : dispatchers_)
        {
            if (dispatcher->dispatchMessage(msg))
            {
            }
            else
            {
                auto forwardChannels = getChannelsEnabledForForwarding(msg.kind);
                for (const auto &forwardChannel : forwardChannels)
                {
                    if (forwardChannel->getName() == msg.origin)
                    {
                        continue;
                    }

                    forwardChannel->publish(settings.mqtt.notify_topic.c_str(), msg.payload, msg.payloadLen, msg.retain);
                }
            }
        }
    }

    void TransportHub::onConnected(void *, const TransportConnectedView &info)
    {
        // Implementar lógica de conexão entre canais, se necessário
    }

    void TransportHub::onDisconnected(void *)
    {
        // Implementar lógica de desconexão entre canais, se necessário
    }

    void TransportHub::addDispatcher(ITransportDispatcher &dispatcher)
    {
        dispatchers_.push_back(&dispatcher);
    }

    std::vector<ITransportChannel *> TransportHub::getChannelsEnabledForForwarding(TransportKind kind)
    {
        std::vector<ITransportChannel *> enabledChannels;
        for (const auto &entry : channels_)
        {
            if (entry->channel->forwardRawMessages() && kind == TransportKind::Raw)
            {
                enabledChannels.push_back(entry->channel);
            }
        }
        return enabledChannels;
    }

} // namespace iotsmartsys::core