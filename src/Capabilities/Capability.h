#pragma once

#include <Arduino.h>
#include <vector>
#include "CapabilityState.h"
#include "Utils/Logger.h"
#include "Models/ContactState.h"
#include "Capabilities/CapabilityType.h"

#define CAPABILITY_CALLBACK_SIGNATURE std::function<void(String *)> callback

struct Capability
{
public:
    Capability() {}
    Capability(String type, String value)
        : capability_name(""), type(type), value(value) {}
    Capability(String capability_name, String type, String value)
        : capability_name(capability_name), type(type), value(value) {}
    Capability(String capability_name, String description, String owner, String type, String mode, String value)
        : capability_name(capability_name), description(description), owner(owner), type(type), mode(mode), value(value) {}
    String capability_name;
    String description;
    String owner;
    String type;
    String mode;
    String value;

    void execute(String state)
    {
        this->callback(&state);
    }

    void updateState(String value)
    {
        this->last_update = millis();
        this->value = value;
        this->changed = true;
    }

    CapabilityState readState()
    {
        this->changed = false;
        return CapabilityState(capability_name, value, type);
    }

    bool hasChanged()
    {
#ifndef RECURRENTLY_UPDATE_STATE_ENABLED
    //    LOG_INFO("RECURRENTLY_UPDATE_STATE_ENABLED não está definido. Retornando changed = ");
    //    LOG_INFO(changed);
       return changed;
#endif

    //    LOG_INFO("RECURRENTLY_UPDATE_STATE_ENABLED está definido. Verificando tempo desde a última atualização...");

        // if (!notifyStateChangeAlwaysActive())
        //     return changed;

        // long now = millis();
        // if (now - last_update > time_between_updates)
        // {
        //     changed = true;
        //     last_update = now;
        // }
        // return changed;
    }

    Capability setCallback(CAPABILITY_CALLBACK_SIGNATURE)
    {
        this->callback = callback;
        return *this;
    }
    CAPABILITY_CALLBACK_SIGNATURE;

    virtual void handle()
    {
    }

    virtual void setup()
    {
    }

    void applyRenamedName(String device_id)
    {
        this->capability_name = device_id + "_" + this->type;
    }

    void rename(String new_capability_name)
    {
        this->capability_name = new_capability_name;
    }

    void setTimeBetweenUpdates(long time_between_updates)
    {
        this->time_between_updates = time_between_updates;
    }

private:
    bool changed = false;
    long last_update = 0;
    long time_between_updates = DEFAULT_TIME_BETWEEN_UPDATES;

    bool notifyStateChangeAlwaysActive()
    {
        return time_between_updates > 0;
    }
};