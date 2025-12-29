
#pragma once

namespace iotsmartsys::core
{
    class ILuminositySensor
    {
    public:
        virtual void setup() = 0;
        virtual ~ILuminositySensor() = default;
        virtual float readLux() = 0;
    };
}