#pragma once

#include "ICommandCapability.h"

namespace iotsmartsys::core
{
    class ValveCapability : public ICommandCapability
    {
    public:
        ValveCapability(std::string capability_name, ICommandHardwareAdapter &hardwareAdapter);

        void setup() override;
        void handle() override;

        void turnOpen();
        void turnClosed();
        bool isOpen() const;

    private:
        void power(const std::string &state);
    };

} // namespace iotsmartsys::core
