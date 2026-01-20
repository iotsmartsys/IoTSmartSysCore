#pragma once

#include "Contracts/System/ISystemControl.h"

namespace iotsmartsys::platform::arduino
{
    class Esp8266SystemControl final : public iotsmartsys::core::ISystemControl
    {
    public:
        void restart() override;
        void stopWifi(bool turnOffRadio) override;
        void restartSafely() override;
    };
} // namespace iotsmartsys::platform::arduino
