#include "CommandProcessorFactory.h"

namespace iotsmartsys::core
{
    CommandProcessorFactory::CommandProcessorFactory(ILogger &logger, CapabilityManager &capabilityManager)
        : _logger(logger), _capabilityManager(capabilityManager), _capabilityCommandProcessor(logger, capabilityManager), _systemCommandProcessor(logger)
    {
    }

    ICommandProcessor *CommandProcessorFactory::createProcessor(const CommandTypes &type)
    {
        switch (type)
        {
        case CommandTypes::CAPABILITY:
            return &_capabilityCommandProcessor;
        case CommandTypes::SYSTEM:
            return &_systemCommandProcessor;
        default:
            _logger.error("Unknown command type.");
            return nullptr;
        }
    }
} // namespace iotsmartsys::core
