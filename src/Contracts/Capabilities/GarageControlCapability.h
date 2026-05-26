#pragma once

#include "ICommandCapability.h"
#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{
#define GARAGE_ACTUATOR_TYPE "Garage Door"

// Commands
#define GARAGE_COMMAND_OPEN "open"
#define GARAGE_COMMAND_CLOSE "close"
#define GARAGE_COMMAND_UNLOCK "unlock"
#define GARAGE_COMMAND_STOP "stop"

// States
#define GARAGE_STATE_OPENED "opened"
#define GARAGE_STATE_CLOSED "closed"
#define GARAGE_STATE_LOCKED "locked"
#define GARAGE_STATE_UNLOCKED "unlocked"

    class GarageControlCapability : public ICommandCapability
    {
    public:
        GarageControlCapability(std::string capability_name, long debounceTimeMs, ICommandHardwareAdapter &hardwareAdapterOpen, ICommandHardwareAdapter &hardwareAdapterClose, ICommandHardwareAdapter &hardwareAdapterStopUnlock, ICommandHardwareAdapter &hardwareAdapterLock, ICapabilityEventSink *event_sink);

        virtual void handle() override;

        bool isOpen() const { return opened; }
        bool isClosed() const { return !opened; }
        bool isLocked() const { return locked; }
        bool isUnlocked() const { return !locked; }
        void open();
        void close();
        void lock();
        void unlock();
        void stop();
        
        void applyCommand(CapabilityCommand command) override;

    private:
        bool opened;
        bool locked;
        const char *currentState;
        char *lastState;
        long debounceTimeMs;

        ICommandHardwareAdapter &hardwareAdapterStopUnlock;
        ICommandHardwareAdapter &hardwareAdapterLock;
        ICommandHardwareAdapter &hardwareAdapterClose;

        void simulatePressCommand(ICommandHardwareAdapter &adapter);

    public:
        void applyArgs(std::vector<std::pair<const char *, const char *>> args);
    };
}
