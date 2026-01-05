#include "CapabilityCommandProcessor.h"
#include "Contracts/Capabilities/ICommandCapability.h"

namespace iotsmartsys::core
{
    CapabilityCommandProcessor::CapabilityCommandProcessor(ILogger &logger, CapabilityManager &capabilityManager)
        : _logger(logger), _capabilityManager(capabilityManager)
    {
    }

    bool CapabilityCommandProcessor::process(const DeviceCommand &command)
    {
        _logger.debug("CapabilityCommandProcessor: Processing command...");
        _logger.warn("Processing capability command for capability: %s", command.capability_name.c_str());
        _logger.warn("Setting value: %s", command.value.c_str());

        ICommandCapability *cap = _capabilityManager.getCommandCapabilityByName(command.capability_name.c_str());
        if (cap)
        {
            _logger.warn("Found capability: %s", command.capability_name.c_str());
            CapabilityCommand capabilityCmd;
            capabilityCmd.capability_name = command.capability_name.c_str();
            capabilityCmd.value = command.value.c_str();
            cap->applyCommand(capabilityCmd);
            return true;
        }
        else
        {
            _logger.error("Capability not found: %s", command.capability_name.c_str());
            return false;
        }
    }
}
