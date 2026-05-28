#include "Contracts/Capabilities/GarageControlCapability.h"
#include "Arduino.h"

namespace iotsmartsys::core
{
    GarageControlCapability::GarageControlCapability(std::string capability_name, long debounceTimeMs, ICommandHardwareAdapter &hardwareAdapterOpen, ICommandHardwareAdapter &hardwareAdapterClose, ICommandHardwareAdapter &hardwareAdapterStopUnlock, ICommandHardwareAdapter &hardwareAdapterLock,
                                                     IInputHardwareAdapter *openSensorAdapter, IInputHardwareAdapter *closeSensorAdapter, ICapabilityEventSink *event_sink)
        : ICommandCapability(hardwareAdapterOpen, event_sink, capability_name, GARAGE_ACTUATOR_TYPE, GARAGE_STATE_UNKNOWN),
          currentState(GARAGE_STATE_UNKNOWN),
          lastState(nullptr),
          debounceTimeMs(debounceTimeMs),
          hardwareAdapterStopUnlock(hardwareAdapterStopUnlock),
          hardwareAdapterLock(hardwareAdapterLock),
          hardwareAdapterClose(hardwareAdapterClose),
          hardwareAdapterSensorOpen(openSensorAdapter),
          hardwareAdapterSensorClose(closeSensorAdapter)
    {
    }

    void GarageControlCapability::setup()
    {
        command_hardware_adapter.setup();
        hardwareAdapterClose.setup();
        hardwareAdapterStopUnlock.setup();
        hardwareAdapterLock.setup();

        if (hardwareAdapterSensorOpen)
        {
            hardwareAdapterSensorOpen->setup();
        }
        if (hardwareAdapterSensorClose)
        {
            hardwareAdapterSensorClose->setup();
        }
    }

    void GarageControlCapability::handle()
    {
        handleSensorState();
        if (currentState == nullptr)
        {
            return;
        }

        if (lastState == nullptr || strcmp(currentState, lastState) != 0)
        {
            logger.info("GarageControlCapability", "State changed from '%s' to '%s'", lastState ? lastState : "(none)", currentState);
            updateState(currentState);
            lastState = currentState;
        }
    }

    void GarageControlCapability::open()
    {
        simulatePressCommand(command_hardware_adapter);
    }

    void GarageControlCapability::close()
    {
        simulatePressCommand(hardwareAdapterClose);
    }

    void GarageControlCapability::lock()
    {
        simulatePressCommand(hardwareAdapterLock);
        locked = true;

        String stateWithoutLock = String(currentState) + "_lock";
        setCurrentState(stateWithoutLock.c_str());
        updateState(stateWithoutLock.c_str());
    }

    void GarageControlCapability::unlock()
    {
        stop();
        locked = false;
    }
    
    void GarageControlCapability::stop()
    {
        simulatePressCommand(hardwareAdapterStopUnlock);
        String stateWithoutLock = currentState;
        stateWithoutLock.replace("_lock", "");
        setCurrentState(stateWithoutLock.c_str());
        updateState(stateWithoutLock.c_str());
    }

    void GarageControlCapability::applyCommand(CapabilityCommand command)
    {
        applyArgs(command.args);
        if (command.isCommand(GARAGE_COMMAND_OPEN))
        {
            open();
        }
        else if (command.isCommand(GARAGE_COMMAND_CLOSE))
        {
            close();
        }
        else if (command.isCommand(GARAGE_COMMAND_LOCK))
        {
            lock();
        }
        else if (command.isCommand(GARAGE_COMMAND_UNLOCK) || command.isCommand(GARAGE_COMMAND_STOP_UNLOCK))
        {
            unlock();
        }
        else if (command.isCommand(GARAGE_COMMAND_STOP))
        {
            stop();
        }
    }

    void GarageControlCapability::applyArgs(std::vector<std::pair<const char *, const char *>> args)
    {
        for (const auto &arg : args)
        {
        }
    }

    void GarageControlCapability::simulatePressCommand(ICommandHardwareAdapter &adapter)
    {
        adapter.applyCommand(POWER_ON_COMMAND);
        logger.info("GarageControlCapability", "Simulating press command on adapter debounce time: %ld ms", debounceTimeMs);
        delay(debounceTimeMs);
        adapter.applyCommand(POWER_OFF_COMMAND);
    }

    void GarageControlCapability::handleSensorState()
    {
        if (hardwareAdapterSensorOpen)
        {
            int currentOpenCompletedState = hardwareAdapterSensorOpen->readDigitalState();
            if (currentOpenCompletedState != sensorOpenCompletedActualState)
            {
                sensorOpenCompletedActualState = currentOpenCompletedState;
            }
        }

        if (hardwareAdapterSensorClose)
        {
            int currentCloseState = hardwareAdapterSensorClose->readDigitalState();
            if (sensorCloseActualState != currentCloseState)
            {
                sensorCloseActualState = currentCloseState;
            }
        }

        if (!sensorStateInitialized)
        {
            sensorStateInitialized = true;
            return;
        }

        if (isClosed())
        {
            if (strcmp(currentState, GARAGE_STATE_CLOSED) != 0)
                setCurrentState(GARAGE_STATE_CLOSED);
        }
        else if (isOpen())
        {
            if (strcmp(currentState, GARAGE_STATE_OPENED) != 0)
                setCurrentState(GARAGE_STATE_OPENED);
        }
        else
        {
            if (isOpening())
            {
                setCurrentState(GARAGE_STATE_OPENING);
            }
            else if (isClosing())
            {
                setCurrentState(GARAGE_STATE_CLOSING);
            }
        }
    }

    /// @brief
    /// @return
    bool GarageControlCapability::isOpening()
    {
        return (strcmp(currentState, GARAGE_STATE_CLOSED) == 0 && sensorCloseActualState == HIGH && sensorOpenCompletedActualState == HIGH);
    }

    bool GarageControlCapability::isClosing()
    {
        return (strcmp(currentState, GARAGE_STATE_OPENED) == 0 && sensorCloseActualState == HIGH && sensorOpenCompletedActualState == HIGH);
    }

}
