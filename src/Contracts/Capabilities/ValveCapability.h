#pragma once

#include "ICapability.h"

namespace iotsmartsys::core
{
    class ValveCapability : public ICapability
    {
    public:
        ValveCapability(std::string capability_name, IHardwareAdapter &hardwareAdapter);

        void setup() override;
        void handle() override;

        void turnOpen();
        void turnClosed();
        bool isOpen() const;

    private:
        void power(const std::string &state);
    };

} // namespace iotsmartsys::core
