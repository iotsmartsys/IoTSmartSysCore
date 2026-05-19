#pragma once

#include "Contracts/Adapters/IHardwareAdapter.h"
#include "Contracts/Providers/Time.h"

namespace iotsmartsys::core
{
    struct DistanceSensorConfig
    {
        long minDistance = 0;
        long maxDistance = 1000;
    };

    enum class DistanceSensorType : uint8_t
    {
        ULTRASONIC_HCSR04 = 0,
        // Add more sensor types here as needed
    };

    class IDistanceSensor : public IHardwareAdapter
    {
    public:
        virtual void setup() = 0;
        virtual void handleSensor() = 0;
        float getActualDistanceCm() const { return actualDistanceCm; }
        /// @brief Get the minimum distance the sensor already read.
        /// @return
        float getMinDistanceCm() const { return minDistanceCm; }
        /// @brief Get the maximum distance the sensor already read.
        /// @return
        float getMaxDistanceCm() const { return maxDistanceCm; }

        void handle()
        {
            unsigned long currentTime = Time::get().nowMs();
            if (currentTime - lastReadTime >= readIntervalMs)
            {
                lastReadTime = currentTime;
                handleSensor();
            }
        }

        void setReadIntervalMs(unsigned long intervalMs)
        {
            this->readIntervalMs = intervalMs;
        }

        void setReadIntervalMinute(unsigned long intervalMinute)
        {
            this->readIntervalMs = intervalMinute * 60000;
        }

    protected:
        unsigned long lastReadTime = 0;
        unsigned long readIntervalMs = 0;
        float minDistanceCm = 0;
        float maxDistanceCm = 0;
        float actualDistanceCm = 0;
    };

} // namespace iotsmartsys::core
