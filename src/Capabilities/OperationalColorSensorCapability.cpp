#include "OperationalColorSensorCapability.h"

#ifdef OPERATIONAL_COLOR_SENSOR_ENABLED
#include <Wire.h>
#include <SparkFun_APDS9960.h>


SparkFun_APDS9960 apds = SparkFun_APDS9960();
uint16_t ambient_light = 0;
uint16_t red_light = 0;
uint16_t green_light = 0;
uint16_t blue_light = 0;



OperationalColorSensorCapability::OperationalColorSensorCapability(unsigned int readIntervalMs)
    : Capability(OPERATIONAL_COLOR_SENSOR_TYPE, OPERATIONAL_COLOR_SENSOR_NORMAL)
{
    this->readIntervalMs = readIntervalMs * 60000;
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Callback da " + this->capability_name + " chamado com o estado: " + *state); });
}

OperationalColorSensorCapability::OperationalColorSensorCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value)
{
}

void OperationalColorSensorCapability::setup()
{
    LOG_PRINTLN("");
    LOG_PRINTLN(F("--------------------------------"));
    LOG_PRINTLN(F("SparkFun APDS-9960 - ColorSensor"));
    LOG_PRINTLN(F("--------------------------------"));

    
    if (apds.init())
    {
        LOG_PRINTLN(F("APDS-9960 initialization complete"));
    }
    else
    {
        LOG_PRINTLN(F("Something went wrong during APDS-9960 init!"));
    }

    
    if (apds.enableLightSensor(false))
    {
        LOG_PRINTLN(F("Light sensor is now running"));
    }
    else
    {
        LOG_PRINTLN(F("Something went wrong during light sensor init!"));
    }

    
    delay(500);
}

void OperationalColorSensorCapability::handle()
{
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= readIntervalMs)
    {
        lastReadTime = currentTime;
        String state = readState();
        if (state != currentState)
        {
            currentState = state;
            updateState(currentState);
        }
    }
}

String OperationalColorSensorCapability::readState()
{
    if (!apds.readAmbientLight(ambient_light) ||
        !apds.readRedLight(red_light) ||
        !apds.readGreenLight(green_light) ||
        !apds.readBlueLight(blue_light))
    {
        
        return OPERATIONAL_COLOR_SENSOR_OK;
    }
    else
    {
        LOG_PRINT("Ambient: ");
        LOG_PRINT(ambient_light);
        LOG_PRINT(" Red: ");
        LOG_PRINT(red_light);
        LOG_PRINT(" Green: ");
        LOG_PRINT(green_light);
        LOG_PRINT(" Blue: ");
        LOG_PRINTLN(blue_light);

        
        if (green_light > red_light && green_light > blue_light)
        {
            LOG_PRINTLN(">>> Cor predominante: VERDE <<<");

            return OPERATIONAL_COLOR_SENSOR_NORMAL;
        }
        else if (red_light > green_light && red_light > blue_light)
        {
            LOG_PRINTLN(">>> Cor predominante: VERMELHA <<<");

            return OPERATIONAL_COLOR_SENSOR_FAIL;
        }
        else if (blue_light > red_light && blue_light > green_light)
        {
            LOG_PRINTLN(">>> Cor predominante: AZUL <<<");

            return OPERATIONAL_COLOR_SENSOR_OK;
        }
        else
        {
            LOG_PRINTLN(">>> Cor predominante: INDEFINIDA <<<");
            return OPERATIONAL_COLOR_SENSOR_NORMAL;
        }
    }
}

#endif
