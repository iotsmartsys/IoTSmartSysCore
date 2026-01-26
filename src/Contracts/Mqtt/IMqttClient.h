#pragma once

#include "Contracts/Transports/ITransportChannel.h"

namespace iotsmartsys::core
{
    class IMqttClient : public iotsmartsys::core::ITransportChannel
    {
    public:
        ~IMqttClient() override = default;
    };

} // namespace iotsmartsys::core
