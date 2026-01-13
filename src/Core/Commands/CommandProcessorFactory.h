#pragma once

#include "Contracts/Commands/ICommandProcessorFactory.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Capabilities/Managers/CapabilityManager.h"
#include "Core/Commands/CapabilityCommandProcessor.h"
#include "Core/Commands/SystemCommandProcessor.h"

namespace iotsmartsys::core
{
    class CommandProcessorFactory : public ICommandProcessorFactory
    {
    public:
        CommandProcessorFactory(ILogger &logger, CapabilityManager &capabilityManager, SystemCommandProcessor &systemCommandProcessor);
        ICommandProcessor *createProcessor(const CommandTypes &type) override;

    private:
        ILogger &_logger;
        CapabilityManager &_capabilityManager;
        CapabilityCommandProcessor _capabilityCommandProcessor;
        SystemCommandProcessor &_systemCommandProcessor;
    };
}
