#include <Arduino.h>
#include "Core/IoTCore.h"
#include "Utils/Logger.h"
#ifdef ESP_NOW_ENABLED
#include <esp_now.h>
#include "esp_now_utils/esp_now_utils.h"
#endif

IoTCore *iotCore = new IoTCore();

void setup()
{
  LOG_BEGIN(115200);
  delay(2000);
  LOG_PRINTLN("[ESPNOW-CLIENT] Setup");

  
  iotCore->addLightCapability(44, DigitalLogic::INVERSE);
  iotCore->setup();

  
}

void loop()
{
  iotCore->handle();
}
