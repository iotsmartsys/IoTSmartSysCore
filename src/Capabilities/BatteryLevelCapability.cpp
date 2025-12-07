#include "BatteryLevelCapability.h"

BatteryLevelCapability::BatteryLevelCapability(String capability_name, int pin)
    : Capability(capability_name, BATTERY_LEVEL_TYPE, "-1"), pin(pin), batteryLevel(0.0), lastTime(0), interval(10000), PIN_BATTERY_ADC(34),
      R1(4658.0), R2(9957.0), VREF(3.26), ADC_RESOLUTION(4095)
{
}

void BatteryLevelCapability::handle()
{
    handleBatteryLevel();
}

void BatteryLevelCapability::setup()
{
    pinMode(pin, INPUT);
}

int BatteryLevelCapability::readStableADC(int pin, int samples)
{
    long total = 0;
    for (int i = 0; i < samples; i++)
    {
        total += analogRead(pin);
        delay(5); 
    }
    return total / samples;
}

void BatteryLevelCapability::handleBatteryLevel()
{
    if (lastTime == 0 || millis() - lastTime > interval)
    {
        lastTime = millis();
    }
    else
    {
        return;
    }

    int raw = readStableADC(PIN_BATTERY_ADC);
    float v_adc = (float)raw / ADC_RESOLUTION * VREF;
    float v_bat = v_adc * (R1 + R2) / R2;

    float percent = (v_bat - 3.0) / (4.08 - 3.0) * 100.0;
    percent = constrain(percent, 0.0, 100.0);

    LOG_PRINTLN("=============================================");
    LOG_PRINTLN("Battery Level " + capability_name);
    LOG_PRINT("ADC raw: ");
    LOG_PRINT(raw);
    LOG_PRINT(" | V_adc: ");
    LOG_PRINT(v_adc);
    LOG_PRINT(" V | V_bat: ");
    LOG_PRINT(v_bat);
    LOG_PRINT(" V | Battery: ");
    LOG_PRINT(percent);
    LOG_PRINTLN(" %");
    if (batteryLevel != percent)
    {
        updateState(String(percent));
        batteryLevel = percent;
    }
}
