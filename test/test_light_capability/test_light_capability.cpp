#include <Arduino.h>
#include <unity.h>
#include "Contracts/Core/Capabilities/LightCapability.h"
#include "Platform/Arduino/Adapters/RelayHardwareAdapter.h"

using namespace iotsmartsys;
platform::arduino::RelayHardwareAdapter relayAdapter(5, platform::arduino::HardwareDigitalLogic::HIGH_IS_ON);

void test_light_capability()
{
    core::LightCapability lightCap("TestLight", relayAdapter);
    lightCap.setup();
    lightCap.turnOn();
    TEST_ASSERT_TRUE(lightCap.isOn());
    lightCap.turnOff();
    TEST_ASSERT_FALSE(lightCap.isOn());
    lightCap.toggle();
    TEST_ASSERT_TRUE(lightCap.isOn());
    lightCap.toggle();
    TEST_ASSERT_FALSE(lightCap.isOn());
    TEST_ASSERT_TRUE(lightCap.isOn());
    lightCap.applyCommand(core::SWITCH_STATE_OFF);
    TEST_ASSERT_FALSE(lightCap.isOn());
}   