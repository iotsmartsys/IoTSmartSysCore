// Core/LuminosityCapability.h
#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/ILuminositySensor.h"

namespace iotsmartsys::core
{
    class LuminosityCapability : public ICapability
    {
    public:
        LuminosityCapability(const std::string &name,
                             ILuminositySensor &sensor,
                             ICapabilityEventSink *event_sink,
                             float variationTolerance,
                             float readIntervalSeconds);

        void setup() override;
        void handle() override;
        float getLux();

    
    protected:

    private:
        ILuminositySensor &_sensor;
        
        float _lastLux{0.0f};
        float _variationTolerance{0.0f};
        unsigned long _lastReadMs{0};
        unsigned long _readIntervalMs{0};
    };

} // namespace iotsmartsys::core