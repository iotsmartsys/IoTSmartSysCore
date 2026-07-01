#include "Contracts/Capabilities/GarageControlCapability.h"
#include "Arduino.h"

namespace iotsmartsys::core
{
    GarageControlCapability::GarageControlCapability(std::string capability_name, long debounceTimeMs, ICommandHardwareAdapter &hardwareAdapterOpen, ICommandHardwareAdapter &hardwareAdapterClose, ICommandHardwareAdapter &hardwareAdapterStopUnlock, ICommandHardwareAdapter &hardwareAdapterLock,
                                                     IInputHardwareAdapter *openSensorAdapter, IInputHardwareAdapter *closeSensorAdapter, ICapabilityEventSink *event_sink)
        : ICommandCapability(hardwareAdapterOpen, event_sink, capability_name, GARAGE_ACTUATOR_TYPE, GARAGE_STATE_UNKNOWN),
          currentState(GARAGE_STATE_UNKNOWN),
          lastState(""),
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
        if (currentState.empty())
        {
            return;
        }

        if (currentState != lastState)
        {
            logger.info("GarageControlCapability", "State changed from '%s' to '%s'", lastState.empty() ? "(none)" : lastState.c_str(), currentState.c_str());
            updateState(currentState.c_str());
            lastState = currentState;
        }
    }

    void GarageControlCapability::open()
    {
        simulatePressCommand(command_hardware_adapter);
        if (!isOpen())
        {
            setCurrentState(GARAGE_STATE_OPENING);
        }
    }

    void GarageControlCapability::close()
    {
        simulatePressCommand(hardwareAdapterClose);
        if (!isClosed())
        {
            setCurrentState(GARAGE_STATE_CLOSING);
        }
    }

    void GarageControlCapability::lock()
    {
        simulatePressCommand(hardwareAdapterLock);
        locked = true;

        String stateWithoutLock = String(currentState.c_str()) + "_lock";
        setCurrentState(stateWithoutLock.c_str());
        updateState(currentState.c_str());
    }

    void GarageControlCapability::unlock()
    {
        stop();
        delay(500);
        stop();
        locked = false;
    }

    void GarageControlCapability::stop()
    {
        simulatePressCommand(hardwareAdapterStopUnlock);
        String stateWithoutLock = currentState.c_str();
        stateWithoutLock.replace("_lock", "");
        setCurrentState(stateWithoutLock.c_str());
        updateState(currentState.c_str());
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

        const bool currentlyOpening = isCurrentState(GARAGE_STATE_OPENING);
        const bool currentlyClosing = isCurrentState(GARAGE_STATE_CLOSING);

        if (isClosed() && !currentlyOpening)
        {
            if (!isCurrentState(GARAGE_STATE_CLOSED))
                setCurrentState(GARAGE_STATE_CLOSED);
        }
        else if (isOpen() && !currentlyClosing)
        {
            if (!isCurrentState(GARAGE_STATE_OPENED))
                setCurrentState(GARAGE_STATE_OPENED);
        }
        else
        {
            if (currentlyOpening || isOpening())
            {
                setCurrentState(GARAGE_STATE_OPENING);
            }
            else if (currentlyClosing || isClosing())
            {
                setCurrentState(GARAGE_STATE_CLOSING);
            }
        }
    }

    /// @brief
    /// @return
    bool GarageControlCapability::isOpening()
    {
        return isCurrentState(GARAGE_STATE_OPENING) ||
               (isCurrentState(GARAGE_STATE_CLOSED) && sensorCloseActualState == HIGH && sensorOpenCompletedActualState == HIGH);
    }

    bool GarageControlCapability::isClosing()
    {
        return isCurrentState(GARAGE_STATE_CLOSING) ||
               (isCurrentState(GARAGE_STATE_OPENED) && sensorCloseActualState == HIGH && sensorOpenCompletedActualState == HIGH);
    }

}
