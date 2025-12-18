// Core/Connectivity/IConnectivityBits.h
#pragma once
#include <cstdint>

namespace iotsmartsys::core
{
    class IConnectivityBits
    {
    public:
        virtual ~IConnectivityBits() = default;
        virtual void set(uint32_t bits) = 0;
        virtual void clear(uint32_t bits) = 0;
    };
}