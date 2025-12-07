#include <Arduino.h>
#include "Core/IoTCore.h"
#include "Utils/Logger.h"

IoTCore *iotCore = new IoTCore();
LEDCapability *ledCapability;
void setup()
{
    LOG_PRINTLN("Setup");
    iotCore = new IoTCore();
    // ledCapability = &iotCore->configureLEDControl(2, DigitalLogic::INVERSE);

    iotCore->addLightCapability(2, DigitalLogic::INVERSE);
    iotCore->setup();
}

void loop()
{
   iotCore->handle();
}
