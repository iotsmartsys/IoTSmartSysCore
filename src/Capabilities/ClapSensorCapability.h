#pragma once
#include <Arduino.h>
#include "Capability.h"

class ClapSensorCapability : public Capability
{
public:
    /// @brief Constructor for ClapSensorCapability
    /// @param ClapPin The pin number for the clap sensor
    /// @param toleranceTime The time in seconds to ignore subsequent claps after a clap is detected
    /// @note Default toleranceTime is 5 seconds if not provided
    ClapSensorCapability(int ClapPin, int toleranceTime = 5);
    ClapSensorCapability(String capability_name, int ClapPin, int toleranceTime);
    ClapSensorCapability(String capability_name, int ClapPin);
    ClapSensorCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

    bool isClapDetected();

private:
    int ClapPin;
    long lastTimeClapDetected;
    bool ClapDetected;
    bool lastState;
    int timeTolerance;

    bool ClapSensorIsTriggered();
    long getTimeSinceLastClapDetected();
    void handleClap();
};
