#pragma once

#include "Contracts/Firmware/IFirmwareRuntimeInfo.h"

namespace iotsmartsys::platform::espressif
{
    class EspIdfFirmwareRuntimeInfo final : public iotsmartsys::core::firmware::IFirmwareRuntimeInfo
    {
    public:
        bool hasOtaPartitions() const override;
        const char *runningPartitionLabel() const override;
        uint32_t runningPartitionAddress() const override;
        const char *bootPartitionLabel() const override;
        uint32_t bootPartitionAddress() const override;
    };
} // namespace iotsmartsys::platform::espressif
