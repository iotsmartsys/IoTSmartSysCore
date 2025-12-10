#pragma once

#include <Arduino.h>
#include "Capability.h"

class LEDCapability : public Capability
{
public:
    LEDCapability(String capability_name, int ledPin, DigitalLogic stateLogic = DigitalLogic::NORMAL);
    LEDCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup();
    void handle() override;

    void toggle();
    void turnOn();
    void turnOff();
    bool isOn();
    void executeCommand(String state);
    void blink(int time = 500);
    // Pisca com intermitência (blinkIntervalMs) e depois pausa (pauseMs).
    // Durante a fase ativa ele alterna a cada blinkIntervalMs por activeWindowMs,
    // então mantém desligado por pauseMs. Chame repetidamente no loop.
    void blinkWithPause(unsigned long blinkIntervalMs, unsigned long pauseMs, unsigned long activeWindowMs = 2000);

private:
    int lightPin;
    bool lightState;
    int stateOn;
    int stateOff;
    int lastBlink;

    // Estado interno para blink com pausa
    unsigned long patternPhaseStart = 0;
    unsigned long patternLastToggle = 0;
    bool patternInPause = false;
    bool patternInitialized = false;

    void power(String state);
};
