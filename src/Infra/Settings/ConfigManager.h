#pragma once
#include <Arduino.h>
#include "Models/Settings.h"
#include "PreferencesKeys.h"

class ConfigManager
{
public:
    static ConfigManager &instance()
    {
        static ConfigManager inst;
        return inst;
    }

    const Settings &get() const { return g_config; }

    bool loadConfigFromPreferences(); // lê NVS
    bool saveConfigToPreferences();   // grava NVS
    bool updateConfig(const Settings &);
    bool loadConfig();
    bool setConfigValue(const String &key, const String &value);
    bool enterConfigMode();
    bool finalizeConfigMode();

private:
    Settings g_config;

    void appendMqttPreferences(const String &prefix, const MqttConfig &config);
    void appendFirmwarePreferences(const FirmwareConfig &config);
    void appendWifiPreferences(const WifiConfig &config);
    void appendApiPreferences(const ApiConfig &config);
};
