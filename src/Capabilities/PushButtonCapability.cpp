
#include "PushButtonCapability.h"


PushButtonCapability::PushButtonCapability(String capability_name, int buttonPin)
    : Capability(capability_name, PUSH_BUTTON_TYPE, PUSH_BUTTON_NO_PRESSED)
{
    this->buttonPin = buttonPin;
}

PushButtonCapability::PushButtonCapability(String capability_name, int buttonPin, int toleranceTime)
    : Capability(capability_name, PUSH_BUTTON_TYPE, PUSH_BUTTON_NO_PRESSED)
{
    this->buttonPin = buttonPin;
    this->timeTolerance = toleranceTime;
}

void PushButtonCapability::handle()
{
    handleButtonPress();
}

void PushButtonCapability::setup()
{
    if (this->buttonPin != -1)
        pinMode(this->buttonPin, INPUT_PULLUP);
}

void PushButtonCapability::handleButtonPress()
{
    static bool lastState = HIGH;
  bool currentState = digitalRead(buttonPin);

  if (lastState == HIGH && currentState == LOW) {
    unsigned long now = millis();

    if (now - lastPressTime < multiClickTimeout) {
      clickCount++;
    } else {
      clickCount = 1;
    }

    lastPressTime = now;
  }
  
  if (clickCount > 0 && (millis() - lastPressTime > multiClickTimeout)) {
    if (clickCount == 1) {
      LOG_PRINTLN("👉 Clique Único");
    } else if (clickCount == 2) {
      LOG_PRINTLN("👉 Duplo Clique");
    } else if (clickCount == 3) {
      LOG_PRINTLN("👉 Triplo Clique");
    } else {
      LOG_PRINTF("👉 %d Cliques\n", clickCount);
    }
    clickCount = 0; 
  }

  lastState = currentState;
}