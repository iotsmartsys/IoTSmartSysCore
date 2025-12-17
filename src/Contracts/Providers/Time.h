#pragma once

#include "ITimeProvider.h"

namespace iotsmartsys::core
{

    class Time
    {
    public:
        static void setProvider(ITimeProvider *provider) { _provider = provider; }
        static ITimeProvider &get();

    private:
        static ITimeProvider *_provider;
    };

}