#pragma once

#include <cstdint>

namespace iotsmartsys::core::firmware
{
    class IFirmwareRuntimeInfo
    {
    public:
        virtual ~IFirmwareRuntimeInfo() = default;

        virtual bool hasOtaPartitions() const = 0;
        virtual const char *runningPartitionLabel() const = 0;
        virtual uint32_t runningPartitionAddress() const = 0;
        virtual const char *bootPartitionLabel() const = 0;
        virtual uint32_t bootPartitionAddress() const = 0;
    };
} // namespace iotsmartsys::core::firmware
