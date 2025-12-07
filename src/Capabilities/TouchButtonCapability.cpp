#include "TouchButtonCapability.h"

TouchButtonCapability::TouchButtonCapability(String capability_name, int pin)
    : Capability(capability_name, BUTTON_TOUCH_TYPE, BUTTON_NO_PRESSED)
{
    this->buttonPin = pin;
}

TouchButtonCapability::TouchButtonCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value)
{
    buttonPin = -1;
}

void TouchButtonCapability::handle()
{
    handleTouchButton();
}

void TouchButtonCapability::setup()
{
    if (this->buttonPin != -1)
        pinMode(this->buttonPin, INPUT);
}

bool TouchButtonCapability::isPressedButton()
{
    return pressed;
}

void TouchButtonCapability::handleTouchButton()
{
    currentState = digitalRead(this->buttonPin);

    if (lastState == LOW && currentState == HIGH)
    {
        pressed = true;
    }
    else if (lastState == HIGH && currentState == LOW)
    {
        pressed = false;
    }

    lastState = currentState;
}
