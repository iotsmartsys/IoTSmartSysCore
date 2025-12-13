#include <Arduino.h>
#include <unity.h>
#include "App/Builders/Builders/CapabilitiesBuilder.h"
#include "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.h"

using namespace iotsmartsys;

static platform::arduino::ArduinoHardwareAdapterFactory hwFactory;

// storage fixo (sem heap)
static core::ICapability *capSlots[8];
static void (*capDtors[8])(void *);
static uint8_t arena[2048];

// builder
static app::CapabilitiesBuilder builder(hwFactory, capSlots, capDtors, 8, arena, sizeof(arena));

static void reset_builder_storage()
{
    builder.reset();
    for (size_t i = 0; i < 8; ++i)
    {
        capSlots[i] = nullptr;
        capDtors[i] = nullptr;
    }
}

void test_builder_addRelay_and_execute_commands()
{
    reset_builder_storage();

    app::LightConfig lightCfg;
    lightCfg.capability_name = "luz_sala";
    lightCfg.pin = 5;
    lightCfg.activeHigh = true;

    auto *luz = builder.addLight(lightCfg);
    TEST_ASSERT_NOT_NULL(luz);

    auto list = builder.build();
    TEST_ASSERT_EQUAL(1, list.count);

    luz->setup();

    luz->turnOn();
    TEST_ASSERT_TRUE(luz->hasChanged());

    TEST_ASSERT_TRUE(luz->isOn());

    luz->turnOff();
    TEST_ASSERT_TRUE(luz->hasChanged());
    TEST_ASSERT_FALSE(luz->isOn());
}

void setup()
{
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_builder_addRelay_and_execute_commands);
    UNITY_END();
}

void loop() {}