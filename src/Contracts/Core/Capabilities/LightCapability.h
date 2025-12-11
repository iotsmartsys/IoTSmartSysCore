
#pragma once

#include "ICapability.h"

namespace iotsmartsys::core
{
    class LightCapability : public ICapability
    {
    public:
        LightCapability(std::string name,
                        IHardwareAdapater &hardwareAdapter);
        void handle() override;

        void toggle();
        void turnOn();
        void turnOff();
        bool isOn() const;
        void executeCommand(const std::string &state);

    private:
        void power(const std::string &state);
    };
}