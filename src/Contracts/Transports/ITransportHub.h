#pragma once

#include "ITransportChannel.h"
#include "ITransportDispatcher.h"

namespace iotsmartsys::core
{

    class ITransportHub
    {
    public:
        virtual ~ITransportHub() = default;

        virtual bool addChannel(const char *name, ITransportChannel *channel) = 0;
        virtual ITransportChannel *getChannel(const char *name) = 0;
        virtual void removeChannel(const char *name) = 0;
        virtual void clearChannels() = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void handle() = 0;
        virtual void addDispatcher(ITransportDispatcher &dispatcher) = 0;
    };

}