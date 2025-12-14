#pragma once

#include "ITimeProvider.h"

namespace iotsmartsys::core
{

    class Time
    {
    public:
        static void setProvider(ITimeProvider *provider) { _provider = provider; }
        static ITimeProvider &get() { return *_provider; }

    private:
        inline static ITimeProvider *_provider = nullptr;
    };

}