#pragma once

#include "Contracts/Sensors/IIRCommandSensor.h"

namespace iotsmartsys::core
{
    class IIRCommandProcessor
    {
    public:
        virtual ~IIRCommandProcessor() = default;

        virtual bool process(IRCommand &command) = 0;
        virtual void setHandlerCallback(void (*handler)(IRCommand &)) = 0;

    private:
        void (*handlerCallback)(IRCommand &) = nullptr;
    };
} // namespace iotsmartsys::core
