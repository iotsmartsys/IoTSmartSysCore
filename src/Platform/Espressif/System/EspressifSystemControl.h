#pragma once

#include "Contracts/System/ISystemControl.h"

namespace iotsmartsys::platform::espressif
{
    class EspressifSystemControl final : public iotsmartsys::core::ISystemControl
    {
    public:
        void restart() override;
        void stopWifi(bool turnOffRadio) override;
        void restartSafely() override;

    private:
        void reset_all_gpio_safely();
        void full_soft_powercycle_restart();
    };
} // namespace iotsmartsys::platform::espressif
