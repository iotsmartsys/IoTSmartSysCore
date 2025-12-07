#include "TemperatureSensorCapability.h"
#include "Utils/Logger.h"

#ifdef DHT_ENABLED
TemperatureSensorCapability::TemperatureSensorCapability(DHT *dht, unsigned long intervalMinute)
    : Capability("", TEMPERATURE_SENSOR_TYPE, "0"), dht(dht), readIntervalMs(intervalMinute * 60000)
{
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("Nada a fazer com o estado: " + *state); });
}
#endif

#ifdef DS18B20_ENABLED
TemperatureSensorCapability::TemperatureSensorCapability(int pin, unsigned long intervalMinute)
    : Capability("", TEMPERATURE_SENSOR_TYPE, "0"), readIntervalMs(intervalMinute * 60000)
{
    try
    {
        oneWire = new OneWire(pin);
        sensors = new DallasTemperature(oneWire);
    }
    catch (const std::exception &e)
    {
        LOG_PRINTLN("Erro ao criar sensor de temperatura DS18B20");
        std::cerr << e.what() << '\n';
    }
    
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("Nada a fazer com o estado: " + *state); });
}
#endif

TemperatureSensorCapability::TemperatureSensorCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value)
{
}

void TemperatureSensorCapability::setup()
{
#ifdef DHT_ENABLED
    if (dht)
        dht->begin();
#endif
#ifdef DS18B20_ENABLED
    if (sensors)
        sensors->begin();
#endif
}

void TemperatureSensorCapability::handle()
{
#ifdef DHT_ENABLED
    if (dht)
        handleDHT();
#endif
#ifdef DS18B20_ENABLED
    if (sensors)
        handleDallasTemperature();
#endif
}

float TemperatureSensorCapability::getTemperature()
{
    return temperature;
}

void TemperatureSensorCapability::setReadIntervalMs(unsigned long intervalMs)
{
    readIntervalMs = intervalMs;
}

#ifdef DHT_ENABLED
void TemperatureSensorCapability::handleDHT()
{
    if (!dht)
        return;

    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < readIntervalMs && temperature > 0)
        return;

    lastReadTime = currentTime;
    float temp = dht->readTemperature();

    if (isValidTemperature(temp))
    {
        temperature = temp;
        updateState(String(temperature));
    }
}
#endif

#ifdef DS18B20_ENABLED
void TemperatureSensorCapability::handleDallasTemperature()
{
    if (!sensors)
        return;

    unsigned long currentTime = millis();
    if (currentTime - lastReadTimeDS < readIntervalMs)
        return;

    lastReadTimeDS = currentTime;
    sensors->requestTemperatures();
    float tempC = sensors->getTempCByIndex(0);

    if (isValidTemperature(tempC))
    {
        temperature = tempC;
        updateState(String(temperature));
    }
}
#endif

bool TemperatureSensorCapability::isValidTemperature(float temp)
{
    return !isnan(temp) && temp > 0 && temp < 80;
}
