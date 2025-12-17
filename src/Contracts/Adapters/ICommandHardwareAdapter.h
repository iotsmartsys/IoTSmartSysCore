#pragma once

#include <string>
#include "IHardwareCommand.h"
#include "IHardwareAdapter.h"

namespace iotsmartsys::core
{
    struct ICommandHardwareAdapter : public IHardwareAdapter
    {
    public:
        virtual ~ICommandHardwareAdapter() = default;
        virtual void setup() = 0;
        virtual bool applyCommand(const IHardwareCommand &command) = 0;
        virtual bool applyCommand(const std::string &value) = 0;
        virtual std::string getState() = 0;
    };
} // namespace iotsmartsys::core
