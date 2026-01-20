#pragma once

namespace iotsmartsys::core
{
    class ISystemControl
    {
    public:
        virtual ~ISystemControl() = default;

        virtual void restart() = 0;
        virtual void stopWifi(bool turnOffRadio) = 0;
        virtual void restartSafely() = 0;
    };
} // namespace iotsmartsys::core
