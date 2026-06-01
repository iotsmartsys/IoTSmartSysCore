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
#define GARAGE_STATE_UNKNOWN "unknown"

    class GarageControlCapability : public ICommandCapability
    {
    public:
        GarageControlCapability(std::string capability_name, long debounceTimeMs, ICommandHardwareAdapter &hardwareAdapterOpen, ICommandHardwareAdapter &hardwareAdapterClose, ICommandHardwareAdapter &hardwareAdapterStopUnlock, ICommandHardwareAdapter &hardwareAdapterLock, IInputHardwareAdapter *openSensorAdapter, IInputHardwareAdapter *closeSensorAdapter, ICapabilityEventSink *event_sink);

        virtual void setup() override;
        virtual void handle() override;

        bool isOpen() const { return sensorOpenCompletedActualState == 0; }
        bool isClosed() const { return sensorCloseActualState == 0; }

        void open();
        void close();
        void lock();
        void unlock();
        void stop();
        /// @brief Verifica se a porta está abrindo. Retorna true se a porta estiver em processo de abertura.
        /// @return
        bool isOpening();
        bool isClosing();

        void applyCommand(CapabilityCommand command) override;

    private:
        bool locked = false;
        const char *currentState;
        const char *lastState;
        long debounceTimeMs;
        int sensorOpenCompletedActualState = -1;
        int sensorCloseActualState = -1;
        bool sensorStateInitialized = false;

        ICommandHardwareAdapter &hardwareAdapterStopUnlock;
        ICommandHardwareAdapter &hardwareAdapterLock;
        ICommandHardwareAdapter &hardwareAdapterClose;

        IInputHardwareAdapter *hardwareAdapterSensorOpen{nullptr};
        IInputHardwareAdapter *hardwareAdapterSensorClose{nullptr};

        void simulatePressCommand(ICommandHardwareAdapter &adapter);
        void handleSensorState();
        void setCurrentState(const char *newState) { currentState = newState; }

    public:
        void applyArgs(std::vector<std::pair<const char *, const char *>> args);
    };
}
