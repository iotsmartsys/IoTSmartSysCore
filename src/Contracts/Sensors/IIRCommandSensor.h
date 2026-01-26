#pragma once

#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{
    struct IRCommand
    {
        bool triggered;
        uint64_t code;
        std::string type;
    };

    class IIRCommandSensor : public IHardwareAdapter
    {
    public:
        IIRCommandSensor() = default;
        virtual ~IIRCommandSensor() = default;

        virtual void setup() = 0;
        virtual void handle() = 0;

        // returns last received command
        virtual IRCommand readCommand() const = 0;
        // marks the last command as read
        virtual void readed() = 0;
    };

} // namespace iotsmartsys::core