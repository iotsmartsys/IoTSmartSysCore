#include <Arduino.h>
#include <unity.h>

#include "App/Builders/Builders/CapabilitiesBuilder.h"
#include "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.h"
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Platform/Arduino/Providers/ArduinoTimeProvider.h"
#include "Contracts/Logging/Log.h"
#include "mocks/TestWaterLevelSensor.h"
#include "mocks/MockHardwareAdapterFactory.h"

using namespace iotsmartsys;

static platform::arduino::ArduinoSerialLogger logger(Serial);
static iotsmartsys::test::mocks::MockHardwareAdapterFactory hwFactory;
static platform::arduino::ArduinoTimeProvider timeProvider;

// storage for builder
static core::ICapability *capSlots[8];
static void (*capDtors[8])(void *);
static void *adapterSlots[8];
static void (*adapterDtors[8])(void *);
static uint8_t arena[2048];

static app::CapabilitiesBuilder builder(
    hwFactory,
    capSlots,
    capDtors,
    8,
    adapterSlots,
    adapterDtors,
    8,
    arena,
    sizeof(arena));

void test_waterlevelliters_builder()
{
    builder.reset();
    for (size_t i = 0; i < 8; ++i)
    {
        capSlots[i] = nullptr;
        capDtors[i] = nullptr;
        adapterSlots[i] = nullptr;
        adapterDtors[i] = nullptr;
    }

    TestWaterLevelSensor sensor;
    hwFactory.setWaterLevelSensor(&sensor);
    sensor.setLiters(223.456f);

    app::WaterLevelSensorConfig cfg;
    cfg.capability_name = "nivel_litros";
    cfg.sensor = &sensor;

    auto *cap = builder.addWaterLevelLiters(cfg);
    TEST_ASSERT_NOT_NULL(cap);

    auto list = builder.build();
    TEST_ASSERT_EQUAL(1, list.count);

    cap->setup();
    cap->handle();
    TEST_ASSERT_EQUAL(223.456f, cap->getLevelLiters());

    delay(1200); // simulate time passing
    sensor.setLiters(225.567f);
    cap->handle();

    TEST_ASSERT_EQUAL(225.567f, cap->getLevelLiters());

    TEST_ASSERT_TRUE(cap->hasChanged());
    auto s = cap->readState();
    TEST_ASSERT_NOT_EQUAL_MESSAGE("0", s.value.c_str(), "Water level in liters is not as expected");
}

void setup()
{
    delay(200);
    Serial.begin(115200);
    core::Log::setLogger(&logger);
    core::Time::setProvider(&timeProvider);

    UNITY_BEGIN();
    RUN_TEST(test_waterlevelliters_builder);
    UNITY_END();
}

void loop() {}
