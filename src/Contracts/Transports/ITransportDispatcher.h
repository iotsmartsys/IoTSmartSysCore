#pragma once

#include "TransportMessageView.h"

namespace iotsmartsys::core
{

    class ITransportDispatcher
    {
    public:
        virtual ~ITransportDispatcher() = default;

        virtual bool dispatchMessage(const TransportMessageView &msg) = 0;
    };

}