#pragma once

#include "ICommandCapability.h"
#include "Contracts/Adapters/IHardwareAdapter.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::core
{
#define GARAGE_ACTUATOR_TYPE "Garage Door"

// Commands
#define GARAGE_COMMAND_OPEN "open"
#define GARAGE_COMMAND_CLOSE "close"
#define GARAGE_COMMAND_UNLOCK "unlock"
#define GARAGE_COMMAND_LOCK "lock"
#define GARAGE_COMMAND_STOP "stop"
#define GARAGE_COMMAND_STOP_UNLOCK "stop_unlock"

// States
#define GARAGE_STATE_OPENED "opened"
#define GARAGE_STATE_CLOSED "closed"
#define GARAGE_STATE_LOCKED "locked"
#define GARAGE_STATE_UNLOCKED "unlocked"
#define GARAGE_STATE_OPENING "opening"
#define GARAGE_STATE_CLOSING "closing"

    class GarageControlCapability : public ICommandCapability
    {
    public:
        GarageControlCapability(std::string capability_name, long debounceTimeMs, ICommandHardwareAdapter &hardwareAdapterOpen, ICommandHardwareAdapter &hardwareAdapterClose, ICommandHardwareAdapter &hardwareAdapterStopUnlock, ICommandHardwareAdapter &hardwareAdapterLock, IInputHardwareAdapter *openSensorAdapter, IInputHardwareAdapter *closeSensorAdapter, ICapabilityEventSink *event_sink);

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
        bool opening;
        bool locked;
        const char *currentState;
        char *lastState;
        long debounceTimeMs;
        bool sensorStateOpen;
        bool sensorStateClose;

        const char *actualReceivedCommand;

        ICommandHardwareAdapter &hardwareAdapterStopUnlock;
        ICommandHardwareAdapter &hardwareAdapterLock;
        ICommandHardwareAdapter &hardwareAdapterClose;

        IInputHardwareAdapter *hardwareAdapterSensorOpen{nullptr};
        IInputHardwareAdapter *hardwareAdapterSensorClose{nullptr};

        void simulatePressCommand(ICommandHardwareAdapter &adapter);
        void handleSensorState();

    public:
        void applyArgs(std::vector<std::pair<const char *, const char *>> args);
    };
}
