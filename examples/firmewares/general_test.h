#include <Arduino.h>
#include "Core/IoTCore.h"

#define CLAP_PIN 27 // D1 on XIAO ESP32C3 (GPIO3). Valid wake pin (D0~D3 map to GPIO2~GPIO5).

// #define BUTTON_PIN 7 // D5

IoTCore *iotCore = new IoTCore();
ClapSensorCapability *clapSensor = new ClapSensorCapability("clap_sensor", CLAP_PIN, 0);

unsigned long lastPressTime = 0;
unsigned long multiClickTimeout = 400;
int clickCount = 0;

void setup()
{
    Serial.begin(115200);
    Serial.print("Setup iniciado...");


    iotCore->setup();
    Serial.println("Mac Address: " + getMacAddress());

}
unsigned long countIteration = 0;
void loop()
{
    iotCore->handle();
}
