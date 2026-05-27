#include "Contracts/Capabilities/GarageControlCapability.h"
#include "Arduino.h"

namespace iotsmartsys::core
{
    GarageControlCapability::GarageControlCapability(std::string capability_name, long debounceTimeMs, ICommandHardwareAdapter &hardwareAdapterOpen, ICommandHardwareAdapter &hardwareAdapterClose, ICommandHardwareAdapter &hardwareAdapterStopUnlock, ICommandHardwareAdapter &hardwareAdapterLock,
                                                     IInputHardwareAdapter *openSensorAdapter, IInputHardwareAdapter *closeSensorAdapter, ICapabilityEventSink *event_sink)
        : ICommandCapability(hardwareAdapterOpen, event_sink, capability_name, GARAGE_ACTUATOR_TYPE, ""),
          hardwareAdapterStopUnlock(hardwareAdapterStopUnlock),
          hardwareAdapterLock(hardwareAdapterLock),
          hardwareAdapterClose(hardwareAdapterClose),
          hardwareAdapterSensorOpen(openSensorAdapter),
          hardwareAdapterSensorClose(closeSensorAdapter),
          opened(false),
          locked(true),
          currentState(GARAGE_STATE_CLOSED),
          lastState(nullptr),
          debounceTimeMs(debounceTimeMs)
    {
    }

    void GarageControlCapability::handle()
    {
        if (currentState == nullptr)
        {
            return;
        }

        if (lastState == nullptr || strcmp(currentState, lastState) != 0)
        {
            updateState(currentState);
            if (lastState)
                free(lastState);
            lastState = strdup(currentState);
        }
    }

    void GarageControlCapability::open()
    {
        // unlock();
        simulatePressCommand(command_hardware_adapter);
    }

    void GarageControlCapability::close()
    {
        // unlock();
        simulatePressCommand(hardwareAdapterClose);
    }

    void GarageControlCapability::lock()
    {
        // stop();
        simulatePressCommand(hardwareAdapterLock);
        locked = true;
    }

    void GarageControlCapability::unlock()
    {
        stop();
    }

    void GarageControlCapability::stop()
    {
        simulatePressCommand(hardwareAdapterStopUnlock);
        locked = false;
    }

    void GarageControlCapability::applyCommand(CapabilityCommand command)
    {
        logger.info("GarageControlCapability", " received command: %s", command.value);
        actualReceivedCommand = command.value;
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
        else if (command.isCommand(DOOR_SENSOR_CLOSED))
        {
            currentState = GARAGE_STATE_CLOSED;
            opened = false;
        }
        else if (command.isCommand(DOOR_SENSOR_OPEN))
        {
            currentState = DOOR_SENSOR_OPEN;
            opened = true;
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
        if(hardwareAdapterSensorOpen)
        {
            sensorStateOpen = hardwareAdapterSensorOpen->digitalActive();
        }
        if(hardwareAdapterSensorClose)
        {
            sensorStateClose = hardwareAdapterSensorClose->digitalActive();
        }

        if(!sensorStateClose && !sensorStateOpen)
        {
            currentState = GARAGE_STATE_OPENING;
            opened = false;
        }
        
        if(sensorStateOpen && !sensorStateClose && 
            actualReceivedCommand != nullptr && 
            strcmp(actualReceivedCommand, GARAGE_COMMAND_CLOSE) == 0 &&
            currentState != GARAGE_STATE_CLOSING)
        {
            currentState = GARAGE_STATE_CLOSING;
            opened = true;
        }
        else if(sensorStateClose && !sensorStateOpen && 
            actualReceivedCommand != nullptr && 
            strcmp(actualReceivedCommand, GARAGE_COMMAND_OPEN) == 0 &&
            currentState != GARAGE_STATE_OPENING)
        {
            currentState = GARAGE_STATE_OPENED;
            opened = true;
        }
    }
}
