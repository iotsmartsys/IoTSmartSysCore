#include <Arduino.h>
#include "../src/Utils/Logger.h"
#include "esp_task_wdt.h"

#define LED_BUILTIN 22

void setup()
{
  LOG_BEGIN(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  LOG_PRINTLN("Iniciando o programa...");
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  LOG_PRINTLN("Ligando e desligando GPIOs...");
  delay(2000);
  LOG_PRINTLN("Ligando GPIO 22");
  digitalWrite(LED_BUILTIN, LOW);
  delay(2000);
  LOG_PRINTLN("Desligando GPIO 22");
}
