#include <Arduino.h>
#include <unity.h>

#include "App/Builders/Builders/CapabilitiesBuilder.h"
#include "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.h"
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Platform/Arduino/Providers/ArduinoTimeProvider.h"
#include "Contracts/Logging/Log.h"
#include "Infra/Sensors/Modules/WaterLevelSensor.h"

using namespace iotsmartsys;

// simple test sensor implementation
class TestWaterLevelSensor : public WaterLevelSensor
{
public:
    TestWaterLevelSensor() : levelPercent(0.0f) {}
    void setup() override {}
    void handleSensor() override {}
    float getLevelPercent() override { return levelPercent; }
    float getLevelLiters() override { return 0.0f; }
    float getHeightWaterInCm() override { return 0.0f; }

    void setLevel(float p) { levelPercent = p; }

private:
    float levelPercent;
};

static platform::arduino::ArduinoSerialLogger logger(Serial);
static platform::arduino::ArduinoHardwareAdapterFactory hwFactory;
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

void test_waterlevelpercent_builder()
{
    // reset builder
    builder.reset();
    for (size_t i = 0; i < 8; ++i)
    {
        capSlots[i] = nullptr;
        capDtors[i] = nullptr;
        adapterSlots[i] = nullptr;
        adapterDtors[i] = nullptr;
    }

    TestWaterLevelSensor sensor;
    sensor.setLevel(12.345f);

    app::WaterLevelSensorConfig cfg;
    cfg.capability_name = "nivel_tanque";
    cfg.sensor = &sensor;

    auto *cap = builder.addWaterLevelPercent(cfg);
    TEST_ASSERT_NOT_NULL(cap);

    auto list = builder.build();
    TEST_ASSERT_EQUAL(1, list.count);

    cap->setup();
    cap->handle();

    TEST_ASSERT_TRUE(cap->hasChanged());
    auto s = cap->readState();
    TEST_ASSERT_NOT_EQUAL_MESSAGE("0", s.value.c_str(), "Water level percent should not be zero");
}

void setup()
{
    delay(200);
    Serial.begin(115200);
    core::Log::setLogger(&logger);
    core::Time::setProvider(&timeProvider);

    UNITY_BEGIN();
    RUN_TEST(test_waterlevelpercent_builder);
    UNITY_END();
}

void loop() {}
