#include "LEDCapability.h"
#include <Arduino.h>

LEDCapability::LEDCapability(String capability_name, int ledPin, DigitalLogic stateLogic)
    : Capability(capability_name, LED_ACTUATOR_TYPE, LED_STATE_OFF), lightPin(ledPin), lightState(false), stateOn(HIGH), stateOff(LOW), lastBlink(0)
{
    if (stateLogic == DigitalLogic::INVERSE)
    {
        stateOn = LOW;
        stateOff = HIGH;
    }
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Callback da " + this->capability_name + " chamado com o estado: " + *state);
                    executeCommand(*state);
                });
}

LEDCapability::LEDCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), lightPin(-1), lightState(false), stateOn(HIGH), stateOff(LOW), lastBlink(0)
{
}

void LEDCapability::setup()
{
    pinMode(this->lightPin, OUTPUT);
}

void LEDCapability::handle()
{
    bool currentState = digitalRead(this->lightPin) == stateOn;
    if (currentState != this->lightState)
    {
        this->lightState = currentState;
        updateState(currentState ? LED_STATE_ON : LED_STATE_OFF);
    }
}

void LEDCapability::toggle()
{
    if (this->lightState)
        turnOff();
    else
        turnOn();
}

void LEDCapability::turnOn()
{
    digitalWrite(this->lightPin, stateOn);
}

void LEDCapability::turnOff()
{
    digitalWrite(this->lightPin, stateOff);
}

bool LEDCapability::isOn()
{
    return this->lightState;
}

void LEDCapability::executeCommand(String state)
{
    if (state == POWER_ON_COMMAND || state == POWER_OFF_COMMAND)
        power(state);
    else if (state == TOGGLE_COMMAND)
        toggle();
}

void LEDCapability::blink(int time)
{
    if (millis() - lastBlink > time)
    {
        lastBlink = millis();
        digitalWrite(this->lightPin, !digitalRead(this->lightPin));
    }
}

void LEDCapability::blinkWithPause(unsigned long blinkIntervalMs, unsigned long pauseMs, unsigned long activeWindowMs)
{
    unsigned long now = millis();

    // Inicializa o padrão na primeira chamada
    if (!patternInitialized)
    {
        patternInitialized = true;
        patternInPause = false;            // começa piscando
        patternPhaseStart = now;
        patternLastToggle = 0;             // força primeiro toggle imediato
    }

    if (!patternInPause)
    {
        // Fase ativa (piscando)
        if (now - patternPhaseStart >= activeWindowMs)
        {
            // Entra em pausa e garante LED desligado
            patternInPause = true;
            patternPhaseStart = now;
            turnOff();
            return;
        }

        if (now - patternLastToggle >= blinkIntervalMs)
        {
            patternLastToggle = now;
            digitalWrite(this->lightPin, !digitalRead(this->lightPin));
        }
    }
    else
    {
        // Fase de pausa (mantém desligado por pauseMs)
        if (now - patternPhaseStart >= pauseMs)
        {
            // Volta a piscar
            patternInPause = false;
            patternPhaseStart = now;
            patternLastToggle = 0; // reinicia a cadência de piscadas
        }
        else
        {
            // Garante LED desligado durante a pausa
            if (digitalRead(this->lightPin) == stateOn)
                turnOff();
        }
    }
}

void LEDCapability::power(String state)
{
    state.toLowerCase();
    if (state == LED_STATE_ON)
        turnOn();
    else if (state == LED_STATE_OFF)
        turnOff();
}
