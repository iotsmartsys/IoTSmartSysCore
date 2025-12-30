#pragma once

#include "Contracts/Commands/ICommandProcessor.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Capabilities/Managers/CapabilityManager.h"

namespace iotsmartsys::core
{
    class CapabilityCommandProcessor : public ICommandProcessor
    {
    public:
        CapabilityCommandProcessor(ILogger &logger, CapabilityManager &capabilityManager);

        void process(const DeviceCommand &command) override;

    private:
        ILogger &_logger;
        CapabilityManager &_capabilityManager;
    };
}