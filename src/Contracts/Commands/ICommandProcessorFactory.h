#pragma once

#include "ICommandProcessor.h"
#include "CommandTypes.h"

namespace iotsmartsys::core
{
    class ICommandProcessorFactory
    {
    public:
        virtual ~ICommandProcessorFactory() = default;

        virtual ICommandProcessor *createProcessor(const CommandTypes &type) = 0;
    };
}