#include "Contracts/Connectivity/ConnectivityGate.h"
#include "Platform/Espressif/Arduino/Connectivity/ArduinoEventLatch.h"
#include "Platform/Espressif/Arduino/Connectivity/ArduinoConnectivityBridge.h"
#include <WiFi.h>

void setup()
{
    static iotsmartsys::platform::espressif::arduino::ArduinoEventLatch latch;
    iotsmartsys::core::ConnectivityGate::init(latch);

    iotsmartsys::platform::espressif::arduino::ArduinoConnectivityBridge::start();

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop()
{
}