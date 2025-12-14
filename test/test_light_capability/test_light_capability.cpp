#include <Arduino.h>
#include <unity.h>
#include "Contracts/Core/Capabilities/LightCapability.h"
#include "Platform/Arduino/Adapters/RelayHardwareAdapter.h"

using namespace iotsmartsys;
platform::arduino::RelayHardwareAdapter relayAdapter(PIN_TEST, platform::arduino::HardwareDigitalLogic::HIGH_IS_ON);
core::LightCapability lightCap("TestLight", relayAdapter);

void test_light_capability()
{
    Serial.begin(115200);
    Serial.println("Iniciando teste de LightCapability...");
    lightCap.setup();
    lightCap.turnOn();
    TEST_ASSERT_TRUE(lightCap.isOn());
    lightCap.turnOff();
    TEST_ASSERT_FALSE(lightCap.isOn());
    lightCap.toggle();
    TEST_ASSERT_TRUE(lightCap.isOn());
    lightCap.toggle();
    TEST_ASSERT_FALSE(lightCap.isOn());
    lightCap.applyCommand(core::ICommand{"TestLight", SWITCH_STATE_OFF});
    TEST_ASSERT_FALSE(lightCap.isOn());
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_light_capability);
    UNITY_END();
}

void loop()
{
    // not used in unit-test harness
}
