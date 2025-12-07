#pragma once
#ifdef DHT_ENABLED
#include "Capability.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>

class HumiditySensorCapability : public Capability
{
public:
    HumiditySensorCapability(DHT *dht, unsigned long intervalMinutes = 1);
    HumiditySensorCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void handle() override;

    float getHumidity();
    void setReadIntervalMs(unsigned long intervalMs);

private:
    DHT *dht;
    float humidity;
    unsigned long lastReadTime;
    unsigned long readIntervalMs;
};

#endif