#include "CapabilityCommandTransportDispatcher.h"
#include "Contracts/Commands/CommandTypes.h"
#include "Contracts/Commands/SystemCommands.h"

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
        const std::size_t previewLen = (msg.payloadLen > 220) ? 220 : msg.payloadLen;
        _logger.info("CMD", "Received topic='%s' payload_len=%u payload='%.*s'",
                     msg.topic ? msg.topic : "(null)",
                     (unsigned)msg.payloadLen,
                     (int)previewLen,
                     msg.payload ? msg.payload : "");

        iotsmartsys::core::DeviceCommand *cmd = _commandParser.parseCommand(msg.payload, msg.payloadLen);
        if (!cmd)
        {
            _logger.error("CMD", "Failed to parse MQTT payload.");
            return false;
        }

        iotsmartsys::core::DeviceCommand command = *cmd;
        delete cmd;

        auto cmdType = command.getCommandType();
        const auto systemCmd = command.getSystemCommand();

        if ((cmdType == CommandTypes::UNKNOWN || cmdType == CommandTypes::CAPABILITY) &&
            systemCmd != SystemCommands::UNKNOWN)
        {
            // Be resilient to malformed "type" when value is a known system command.
            command.type = CommandTypeStrings::SYSTEM_STR;
            cmdType = CommandTypes::SYSTEM;
            _logger.warn("CMD", "Promoted command to SYSTEM (value='%s').", command.value.c_str());
        }

        _logger.info("CMD", "Dispatching command type='%s' value='%s' capability='%s'.",
                     CommandTypeUtils::toString(cmdType),
                     command.value.c_str(),
                     command.capability_name.c_str());

        ICommandProcessor *processor = _commandProcessorFactory.createProcessor(cmdType);
        if (processor)
        {
            return processor->process(command);
        }
        else
        {
            _logger.error("CMD", "No processor for command type='%s'.", CommandTypeUtils::toString(cmdType));
            return false;
        }
    }

} // namespace iotsmartsys::core
