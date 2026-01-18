#pragma once

#include <string>
#include "IHardwareCommand.h"
#include "IHardwareAdapter.h"
#include "IHardwareState.h"

namespace iotsmartsys::core
{
    struct ICommandHardwareAdapter : public IHardwareAdapter
    {
    public:
        virtual ~ICommandHardwareAdapter() = default;
        virtual void setup() = 0;
        virtual void handle() = 0;
        virtual bool applyCommand(const IHardwareCommand &command) = 0;
        virtual bool applyCommand(const char *value) = 0;
        virtual std::string getStateValue() = 0;
        virtual IHardwareState getState() = 0;
    };
} // namespace iotsmartsys::core
