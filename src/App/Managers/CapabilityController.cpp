#include "App/Managers/CapabilityController.h"

#include "App/Builders/Builders/CapabilitiesBuilder.h"
#include "Core/Commands/CapabilityCommandTransportDispatcher.h"
#include "Core/Commands/CommandProcessorFactory.h"

namespace iotsmartsys::app
{
    CapabilityController::CapabilityController(core::ILogger &logger,
                                               core::ICommandParser &commandParser,
                                               core::SystemCommandProcessor &systemCommandProcessor)
        : logger_(logger),
          commandParser_(commandParser),
          systemCommandProcessor_(systemCommandProcessor)
    {
    }

    void CapabilityController::setup(app::CapabilitiesBuilder &builder)
    {
        static iotsmartsys::core::CapabilityManager capManager = builder.build();
        capabilityManager_ = &capManager;
        capabilityManager_->setup();

        commandProcessorFactory_ = new core::CommandProcessorFactory(
            logger_,
            *capabilityManager_,
            systemCommandProcessor_);
        commandDispatcher_ = new core::CapabilityCommandTransportDispatcher(
            *commandProcessorFactory_,
            commandParser_,
            logger_);
    }

    void CapabilityController::handle()
    {
        if (capabilityManager_)
        {
            capabilityManager_->handle();
        }
    }

    core::CapabilityManager *CapabilityController::manager() const
    {
        return capabilityManager_;
    }

    core::ITransportDispatcher *CapabilityController::dispatcher() const
    {
        return commandDispatcher_;
    }
} // namespace iotsmartsys::app
