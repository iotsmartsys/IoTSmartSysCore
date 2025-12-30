#include "CapabilityCommandProcessor.h"
#include "Contracts/Capabilities/ICommandCapability.h"

namespace iotsmartsys::core
{
    CapabilityCommandProcessor::CapabilityCommandProcessor(ILogger &logger, CapabilityManager &capabilityManager)
        : _logger(logger), _capabilityManager(capabilityManager)
    {
    }

    void CapabilityCommandProcessor::process(const DeviceCommand &command)
    {
        _logger.debug("CapabilityCommandProcessor: Processing command...");
        _logger.warn("Processing capability command for capability: %s", command.capability_name);
        _logger.warn("Setting value: %s", command.value);

        ICommandCapability *cap = _capabilityManager.getCommandCapabilityByName(command.capability_name);
        if (cap)
        {
            _logger.warn("Found capability: %s", command.capability_name);
            CapabilityCommand capabilityCmd;
            capabilityCmd.capability_name = command.capability_name;
            capabilityCmd.value = command.value;
            cap->applyCommand(capabilityCmd);
        }
        else
        {
            _logger.error("Capability not found: %s", command.capability_name);
        }
    }
}