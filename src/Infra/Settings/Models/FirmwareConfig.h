#pragma once
#include <Arduino.h>

enum class FirmwareUpdateMethod
{
    NONE,
    OTA,
    AUTO
};

struct FirmwareConfig
{
    String url;
    String manifest;
    bool verify_sha256{false};
    FirmwareUpdateMethod update{FirmwareUpdateMethod::OTA};

    void setUpdate(const char *value)
    {
        update = FirmwareConfig::firmwareUpdateMethodFromCString(value);
    }

    String getUpdateString() const
    {
        switch (update)
        {
        case FirmwareUpdateMethod::NONE:
            return "NONE";
        case FirmwareUpdateMethod::OTA:
            return "OTA";
        case FirmwareUpdateMethod::AUTO:
            return "AUTO";
        default:
            return "NONE";
        }
    }
    
    static FirmwareUpdateMethod firmwareUpdateMethodFromCString(const char *value)
    {
        if (value == nullptr)
            return FirmwareUpdateMethod::NONE;

        // Comparação case-insensitive simples
        String v(value);
        if (v.equalsIgnoreCase("none"))
            return FirmwareUpdateMethod::NONE;
        if (v.equalsIgnoreCase("ota"))
            return FirmwareUpdateMethod::OTA;
        if (v.equalsIgnoreCase("auto"))
            return FirmwareUpdateMethod::AUTO;

        return FirmwareUpdateMethod::NONE;
    }
};