#pragma once

#include "Capability.h"
#include <iostream>
#ifdef DHT_ENABLED
#include <Adafruit_Sensor.h>
#include <DHT.h>
#endif
#ifdef DS18B20_ENABLED
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

class TemperatureSensorCapability : public Capability
{
public:
#ifdef DHT_ENABLED
    TemperatureSensorCapability(DHT *dht, unsigned long intervalMinute = 1);
#endif
#ifdef DS18B20_ENABLED
    TemperatureSensorCapability(int pin, unsigned long intervalMinute = 1);
#endif
    TemperatureSensorCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;
    float getTemperature();
    void setReadIntervalMs(unsigned long intervalMs);

private:
    unsigned long readIntervalMs = 60000; 
    float temperature = 0;

#ifdef DHT_ENABLED
    DHT *dht = nullptr;
    unsigned long lastReadTime = 0;
    void handleDHT();
#endif

#ifdef DS18B20_ENABLED
    OneWire *oneWire = nullptr;
    DallasTemperature *sensors = nullptr;
    unsigned long lastReadTimeDS = 0;
    void handleDallasTemperature();
#endif

    bool isValidTemperature(float temp);
};
