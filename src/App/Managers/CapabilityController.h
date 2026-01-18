#pragma once

#include "Contracts/Logging/ILogger.h"
#include "Contracts/Parsers/ICommandParser.h"
#include "Contracts/Transports/ITransportDispatcher.h"
#include "Core/Commands/SystemCommandProcessor.h"

namespace iotsmartsys::app
{
    class CapabilitiesBuilder;
}

namespace iotsmartsys::core
{
    class CapabilityManager;
    class CommandProcessorFactory;
    class CapabilityCommandTransportDispatcher;
}

namespace iotsmartsys::app
{
    class CapabilityController
    {
    public:
        CapabilityController(core::ILogger &logger,
                             core::ICommandParser &commandParser,
                             core::SystemCommandProcessor &systemCommandProcessor);

        void setup(app::CapabilitiesBuilder &builder);
        void handle();
        core::CapabilityManager *manager() const;
        core::ITransportDispatcher *dispatcher() const;

    private:
        core::ILogger &logger_;
        core::ICommandParser &commandParser_;
        core::SystemCommandProcessor &systemCommandProcessor_;
        core::CapabilityManager *capabilityManager_{nullptr};
        core::CommandProcessorFactory *commandProcessorFactory_{nullptr};
        core::CapabilityCommandTransportDispatcher *commandDispatcher_{nullptr};
    };
} // namespace iotsmartsys::app
