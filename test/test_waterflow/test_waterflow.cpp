#include <Arduino.h>
#include <unity.h>
#include "App/Builders/Builders/CapabilitiesBuilder.h"
#include "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.h"
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Platform/Arduino/Providers/ArduinoTimeProvider.h"
#include "Platform/Arduino/Adapters/OutputHardwareAdapter.h"

#include "Contracts/Logging/Log.h"

using namespace iotsmartsys;

static iotsmartsys::platform::arduino::ArduinoSerialLogger logger(Serial);
static platform::arduino::ArduinoHardwareAdapterFactory hwFactory;
static platform::arduino::ArduinoTimeProvider timeProvider;
static platform::arduino::OutputHardwareAdapter *outputAdapter = new platform::arduino::OutputHardwareAdapter(PIN_TEST, platform::arduino::HardwareDigitalLogic::HIGH_IS_ON);

// storage fixo (sem heap)
static core::ICapability *capSlots[8];
static void (*capDtors[8])(void *);
static void *adapterSlots[8];
static void (*adapterDtors[8])(void *);
static uint8_t arena[2048];

// builder
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

static void reset_builder_storage()
{
    builder.reset();
    for (size_t i = 0; i < 8; ++i)
    {
        capSlots[i] = nullptr;
        capDtors[i] = nullptr;
        adapterSlots[i] = nullptr;
        adapterDtors[i] = nullptr;
    }
}

void test_water_flow_with_builder_capabilities()
{
    reset_builder_storage();
    // Adicionar WaterFlowHallSensorCapability
    app::WaterFlowHallSensorConfig waterFlowConfig;
    waterFlowConfig.capability_name = "water_flow_sensor";
    waterFlowConfig.pin = PIN_TEST; // pino de teste para sensor de fluxo de água
    auto *waterFlowSensor = builder.addWaterFlowHallSensor(waterFlowConfig);
    TEST_ASSERT_NOT_NULL(waterFlowSensor);
    auto list = builder.build();
    TEST_ASSERT_EQUAL(1, list.count);
    waterFlowSensor->setup();
    TEST_ASSERT_FALSE(waterFlowSensor->hasChanged());

    // Reset builder storage
    reset_builder_storage();
}

void setup()
{
    delay(200);
    Serial.begin(115200);
    iotsmartsys::core::Log::setLogger(&logger);
    iotsmartsys::core::Time::setProvider(&timeProvider);

    iotsmartsys::core::Log::get().info("BOOT", "Iniciando...");
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_water_flow_with_builder_capabilities);
    UNITY_END();
}

void loop() {}