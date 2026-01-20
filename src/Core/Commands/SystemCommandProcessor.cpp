#include "SystemCommandProcessor.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Contracts/System/ISystemControl.h"

namespace iotsmartsys::core
{
    SystemCommandProcessor::SystemCommandProcessor(ILogger &logger)
        : _logger(logger),
          _systemControl(iotsmartsys::core::ServiceProvider::instance().getSystemControl())
    {
    }

    bool SystemCommandProcessor::process(const DeviceCommand &command)
    {
        _logger.warn("SystemCommandProcessor: Processing system command.");
        switch (command.getSystemCommand())
        {
        case SystemCommands::REBOOT:
            _logger.warn("SystemCommandProcessor: Executing REBOOT command.");
            if (_systemControl)
            {
                _systemControl->restartSafely();
            }
            return true;
            break;
        case SystemCommands::FACTORY_RESET:
            _logger.info("SystemCommandProcessor: Executing FACTORY_RESET command.");
            /* code for factory reset */
            return true;
            break;
        case SystemCommands::UPDATE_FIRMWARE:
            _logger.info("SystemCommandProcessor: Executing UPDATE_FIRMWARE command.");
            /* code for update firmware */
            return true;
            break;
        default:
            _logger.warn("SystemCommandProcessor: Unknown system command received.");
            return false;
            break;
        }
    }

    void SystemCommandProcessor::restartSafely()
    {
        _logger.warn("SystemCommandProcessor: Performing safe restart.");
        if (_systemControl)
        {
            _systemControl->restartSafely();
        }
    }
} // namespace iotsmartsys::core
