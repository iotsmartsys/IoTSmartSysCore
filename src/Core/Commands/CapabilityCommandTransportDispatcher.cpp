#include "CapabilityCommandTransportDispatcher.h"

namespace iotsmartsys::core
{

    CapabilityCommandTransportDispatcher::CapabilityCommandTransportDispatcher(CommandProcessorFactory &factory,
                                                                               ICommandParser &commandParser,
                                                                               ILogger &logger)
        : _commandParser(commandParser), _logger(logger), _commandProcessorFactory(factory)
    {
    }

    CapabilityCommandTransportDispatcher::~CapabilityCommandTransportDispatcher()
    {
    }

    bool CapabilityCommandTransportDispatcher::dispatchMessage(const TransportMessageView &msg)
    {
        iotsmartsys::core::DeviceCommand *cmd = _commandParser.parseCommand(msg.payload, msg.payloadLen);
        if (!cmd)
        {
           // _logger.error("Failed to parse MQTT message payload.");
            return false;
        }

        iotsmartsys::core::DeviceCommand command = *cmd;
        delete cmd;

        ICommandProcessor *processor = _commandProcessorFactory.createProcessor(command.getCommandType());
        if (processor)
        {
            return processor->process(command);
        }
        else
        {
           // _logger.warn("No command processor available for command type.");
            return false;
        }
    }

} // namespace iotsmartsys::core
