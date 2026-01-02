#include "TransportHub.h"
#include <cstring>

namespace iotsmartsys::core
{

    TransportHub::TransportHub(ILogger &logger) : logger_(logger) {}

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

        logger_.info("TransportHub", "Message received on channel payload: %.*s",
                     (int)msg.payloadLen, msg.payload);

        for (auto &dispatcher : dispatchers_)
        {
            if (dispatcher->dispatchMessage(msg))
            {
                logger_.info("TransportHub", "Message dispatched successfully");
            }
            else
            {
                logger_.warn("TransportHub", "Message dispatch failed");
                ITransportChannel *forwardChannel = getChannelEnabledForForwarding(msg.kind);
                if (forwardChannel)
                {
                    // if (forwardChannel->getName() != msg.)

                    if (forwardChannel->publish(msg.topic, msg.payload, msg.payloadLen, msg.retain))
                    {
                        logger_.info("TransportHub", "Message forwarded successfully to channel name: %s", forwardChannel->getName());
                    }
                    else
                    {
                        logger_.error("TransportHub", "Failed to forward message to channel name: %s", forwardChannel->getName());
                    }
                }
            }
        }

        // Implementar lógica de roteamento de mensagens entre canais, se necessário
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

    ITransportChannel *TransportHub::getChannelEnabledForForwarding(TransportKind kind)
    {
        for (const auto &entry : channels_)
        {
            if (entry->channel->forwardRawMessages() && kind == TransportKind::Raw)
            {
                return entry->channel;
            }
            // Adicionar outras verificações de tipo de transporte, se necessário
        }
        return nullptr;
    }

}