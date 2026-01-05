#pragma once
#include "Contracts/Transports/ITransportDispatcher.h"
#include "CommandProcessorFactory.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Parsers/ICommandParser.h"

namespace iotsmartsys::core
{

    class CapabilityCommandTransportDispatcher : public ITransportDispatcher
    {
    public:
        CapabilityCommandTransportDispatcher(CommandProcessorFactory &factory,
                                             ICommandParser &commandParser,
                                             ILogger &logger);

        virtual ~CapabilityCommandTransportDispatcher() override;

        virtual bool dispatchMessage(const TransportMessageView &msg) override;

    private:
        CommandProcessorFactory &_commandProcessorFactory;
        ICommandParser &_commandParser;
        ILogger &_logger;
    };

} // namespace iotsmartsys::core
