#include "CommandProcessorFactory.h"

namespace iotsmartsys::core
{
    CommandProcessorFactory::CommandProcessorFactory(ILogger &logger, CapabilityManager &capabilityManager)
        : _logger(logger), _capabilityManager(capabilityManager), _capabilityCommandProcessor(nullptr)
    {
    }

    ICommandProcessor *CommandProcessorFactory::createProcessor(const CommandTypes &type)
    {
        switch (type)
        {
        case CommandTypes::CAPABILITY:
            if (_capabilityCommandProcessor == nullptr)
            {
                _capabilityCommandProcessor = new CapabilityCommandProcessor(_logger, _capabilityManager);
            }
            return _capabilityCommandProcessor;
        case CommandTypes::SYSTEM:
            _logger.error("SYSTEM command processor not implemented yet.");
            return nullptr;
        default:
            _logger.error("Unknown command type.");
            return nullptr;
        }
    }
} // namespace iotsmartsys::core