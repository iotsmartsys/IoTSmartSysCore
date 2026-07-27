#include "Contracts/Capabilities/GarageControlCapability.h"
#include "Arduino.h"

namespace iotsmartsys::core
{
    GarageControlCapability::GarageControlCapability(std::string capability_name, long debounceTimeMs, ICommandHardwareAdapter &hardwareAdapterOpen, ICommandHardwareAdapter &hardwareAdapterClose, ICommandHardwareAdapter &hardwareAdapterStopUnlock, ICommandHardwareAdapter &hardwareAdapterLock,
                                                     IInputHardwareAdapter *openSensorAdapter, IInputHardwareAdapter *closeSensorAdapter, ICapabilityEventSink *event_sink, long sensorDebounceTimeMs)
        : ICommandCapability(hardwareAdapterOpen, event_sink, capability_name, GARAGE_ACTUATOR_TYPE, GARAGE_STATE_UNKNOWN),
          currentState(GARAGE_STATE_UNKNOWN),
          lastState(""),
          debounceTimeMs(debounceTimeMs),
          sensorDebounceTimeMs(sensorDebounceTimeMs),
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
            rawOpenState = hardwareAdapterSensorOpen->readDigitalState();
            stableOpenState = rawOpenState;
            openLastRawChangeMs = timeProvider.nowMs();
        }
        if (hardwareAdapterSensorClose)
        {
            hardwareAdapterSensorClose->setup();
            rawCloseState = hardwareAdapterSensorClose->readDigitalState();
            stableCloseState = rawCloseState;
            closeLastRawChangeMs = timeProvider.nowMs();
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
        requestedDirection = GarageRequestedDirection::Open;
    }

    void GarageControlCapability::close()
    {
        simulatePressCommand(hardwareAdapterClose);
        requestedDirection = GarageRequestedDirection::Close;
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
        const std::uint64_t now = timeProvider.nowMs();

        if (hardwareAdapterSensorOpen)
        {
            int currentOpenState = hardwareAdapterSensorOpen->readDigitalState();
            if (currentOpenState != rawOpenState)
            {
                rawOpenState = currentOpenState;
                openLastRawChangeMs = now;
            }
        }

        if (hardwareAdapterSensorClose)
        {
            int currentCloseState = hardwareAdapterSensorClose->readDigitalState();
            if (currentCloseState != rawCloseState)
            {
                rawCloseState = currentCloseState;
                closeLastRawChangeMs = now;
            }
        }

        updateStableSensorStates(now);
        evaluateStateFromSensors();
    }

    void GarageControlCapability::updateStableSensorStates(std::uint64_t now)
    {
        const std::uint64_t debounceThreshold = static_cast<std::uint64_t>(sensorDebounceTimeMs);

        if (hardwareAdapterSensorOpen && now >= openLastRawChangeMs + debounceThreshold)
        {
            stableOpenState = rawOpenState;
        }

        if (hardwareAdapterSensorClose && now >= closeLastRawChangeMs + debounceThreshold)
        {
            stableCloseState = rawCloseState;
        }
    }

    void GarageControlCapability::evaluateStateFromSensors()
    {
        const bool hasOpenSensor = hardwareAdapterSensorOpen != nullptr;
        const bool hasCloseSensor = hardwareAdapterSensorClose != nullptr;
        const bool openActiveStable = hasOpenSensor && stableOpenState == 0;
        const bool closeActiveStable = hasCloseSensor && stableCloseState == 0;

        const std::string previousState = currentState;

        // GAR-016: contradictory evidence -> unknown.
        if (openActiveStable && closeActiveStable)
        {
            setCurrentState(GARAGE_STATE_UNKNOWN);
            requestedDirection = GarageRequestedDirection::None;
            return;
        }

        // GAR-004: stable close active -> closed.
        if (closeActiveStable)
        {
            setCurrentState(GARAGE_STATE_CLOSED);
            requestedDirection = GarageRequestedDirection::None;
            return;
        }

        // GAR-005: stable open active -> opened.
        if (openActiveStable)
        {
            setCurrentState(GARAGE_STATE_OPENED);
            requestedDirection = GarageRequestedDirection::None;
            return;
        }

        // No stable active endpoint.
        if (!hasOpenSensor && !hasCloseSensor)
        {
            // GAR-018: no sensors at all, rely only on requested direction.
            if (requestedDirection == GarageRequestedDirection::Open)
            {
                setCurrentState(GARAGE_STATE_OPENING);
            }
            else if (requestedDirection == GarageRequestedDirection::Close)
            {
                setCurrentState(GARAGE_STATE_CLOSING);
            }
            else if (!isCurrentState(GARAGE_STATE_OPENING) && !isCurrentState(GARAGE_STATE_CLOSING))
            {
                setCurrentState(GARAGE_STATE_UNKNOWN);
            }
            return;
        }

        // At least one sensor present, no stable active endpoint.
        if (requestedDirection == GarageRequestedDirection::Open)
        {
            setCurrentState(GARAGE_STATE_OPENING);
        }
        else if (requestedDirection == GarageRequestedDirection::Close)
        {
            setCurrentState(GARAGE_STATE_CLOSING);
        }
        else
        {
            // GAR-012: infer external movement from the last known terminal state.
            if (previousState == GARAGE_STATE_CLOSED)
            {
                setCurrentState(GARAGE_STATE_OPENING);
            }
            else if (previousState == GARAGE_STATE_OPENED)
            {
                setCurrentState(GARAGE_STATE_CLOSING);
            }
            // Otherwise keep the current movement state (opening/closing/unknown).
        }
    }

    /// @brief
    /// @return
    bool GarageControlCapability::isOpening()
    {
        return isCurrentState(GARAGE_STATE_OPENING);
    }

    bool GarageControlCapability::isClosing()
    {
        return isCurrentState(GARAGE_STATE_CLOSING);
    }

} // namespace iotsmartsys::core
