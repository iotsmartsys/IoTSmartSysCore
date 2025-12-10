#pragma once
#include <Arduino.h>
#include "MqttSettings.h"
#include "FirmwareConfig.h"
#include "WifiConfig.h"
#include "ApiConfig.h"

struct Settings
{
    bool in_config_mode{true};
    
    MqttSettings mqtt;
    FirmwareConfig firmware;
    WifiConfig wifi;
    ApiConfig api;
};