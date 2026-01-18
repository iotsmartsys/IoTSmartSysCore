#pragma once

namespace iotsmartsys::core
{
    enum class StateDevice
    {
        InProvisioning = 0,
        ConnectingToNetwork = 1,
        Connected = 2,
        Operational = 3,
        TransportConnected = 4,
        Error = 5
    };
} // namespace iotsmartsys::core
