#pragma once

#include "Contracts/Commands/ICommandProcessor.h"
#include "Contracts/Logging/ILogger.h"

namespace iotsmartsys::core
{
    class SystemCommandProcessor : public ICommandProcessor
    {
    public:
        SystemCommandProcessor(ILogger &logger);

        bool process(const DeviceCommand &command) override;

    private:
        ILogger &_logger;
        void reset_all_gpio_safely();
        void full_soft_powercycle_restart();
    };
}