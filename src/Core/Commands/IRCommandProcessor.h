#pragma once

#include "Contracts/Sensors/IIRCommandSensor.h"
#include "Contracts/Commands/IIRCommandProcessor.h"

namespace iotsmartsys::core
{
    class IRCommandProcessor : public IIRCommandProcessor
    {
    public:
        bool process(IRCommand &command) override
        {
            if (!command.triggered)
            {
                return false;
            }

            if (handlerCallback)
            {
                handlerCallback(command);
            }
            return true;
        }

        void setHandlerCallback(void (*handler)(IRCommand &)) override
        {
            handlerCallback = handler;
        }

    private:
        void (*handlerCallback)(IRCommand &) = nullptr;
    };
} // namespace iotsmartsys::core
