#pragma once

#include "Contracts/Transports/ITransportHub.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Settings/IReadOnlySettingsProvider.h"
#include "vector"

namespace iotsmartsys::core
{
    using namespace iotsmartsys::core::settings;
    class TransportHub : public ITransportHub
    {
    public:
        TransportHub(ILogger &logger, IReadOnlySettingsProvider &settingsProvider);
        virtual ~TransportHub() override;

        virtual bool addChannel(const char *name, ITransportChannel *channel) override;
        virtual ITransportChannel *getChannel(const char *name) override;
        virtual void removeChannel(const char *name) override;
        virtual void clearChannels() override;
        virtual void start() override;
        virtual void stop() override;
        virtual void handle() override;

        virtual void addDispatcher(ITransportDispatcher &dispatcher) override;

    private:
        ILogger &logger_;
        struct ChannelEntry
        {
            const char *name;
            ITransportChannel *channel;
        };

        std::vector<ChannelEntry *> channels_;
        std::vector<ITransportDispatcher *> dispatchers_;
        IReadOnlySettingsProvider &_settingsProvider;

        void onMessageReceived(void *, const TransportMessageView &msg);
        void onConnected(void *, const TransportConnectedView &info);
        void onDisconnected(void *);
        std::vector<ITransportChannel *> getChannelsEnabledForForwarding(TransportKind kind);
    };

}