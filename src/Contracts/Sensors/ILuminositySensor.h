
#pragma once

namespace iotsmartsys::core
{
    class ILuminositySensor
    {
    public:
        virtual ~ILuminositySensor() = default;
        virtual float readLux() = 0;
    };
}