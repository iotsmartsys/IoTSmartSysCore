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
static platform::arduino::OutputHardwareAdapter *outputAdapter = new platform::arduino::OutputHardwareAdapter(13, platform::arduino::HardwareDigitalLogic::HIGH_IS_ON);

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

void test_builder_capabilities()
{
    reset_builder_storage();

    const uint8_t alarmPin = 44; // pino de teste para alarme (LED Blue)


    // adicionar luz
    app::LightConfig lightCfg;
    lightCfg.capability_name = "luz_sala";
    lightCfg.pin = PIN_TEST;
    lightCfg.activeHigh = false;

    auto *luz = builder.addLight(lightCfg);
    TEST_ASSERT_NOT_NULL(luz);

    auto list = builder.build();
    TEST_ASSERT_EQUAL(1, list.count);

    luz->setup();

    luz->turnOn();
    TEST_ASSERT_TRUE(luz->hasChanged());

    TEST_ASSERT_TRUE(luz->isOn());
    delay(3000);
    luz->turnOff();
    TEST_ASSERT_TRUE(luz->hasChanged());
    TEST_ASSERT_FALSE(luz->isOn());
    luz->turnOn();

    // adicionar alarme
    app::AlarmConfig alarmCfg;
    alarmCfg.capability_name = "alarme_sala";
    alarmCfg.pin = alarmPin;
    alarmCfg.activeState = false;

    auto *alarme = builder.addAlarm(alarmCfg);
    TEST_ASSERT_NOT_NULL(alarme);

    list = builder.build();
    TEST_ASSERT_EQUAL(2, list.count);

    alarme->setup();

    alarme->powerOn();
    TEST_ASSERT_TRUE(alarme->hasChanged());
    auto s = alarme->readState();
    TEST_ASSERT_EQUAL_STRING("on", s.value.c_str());
    delay(3000);
    alarme->powerOff();
    TEST_ASSERT_TRUE(alarme->hasChanged());
    s = alarme->readState();
    TEST_ASSERT_EQUAL_STRING("off", s.value.c_str());

    // adicionar sensor de porta
    app::DoorSensorConfig doorSensorCfg;
    doorSensorCfg.capability_name = "sensor_porta";
    doorSensorCfg.pin = 45; // pino de teste para sensor de porta

    auto *sensorPorta = builder.addDoorSensor(doorSensorCfg);
    TEST_ASSERT_NOT_NULL(sensorPorta);

    list = builder.build();
    TEST_ASSERT_EQUAL(3, list.count);

    sensorPorta->setup();

    TEST_ASSERT_FALSE(sensorPorta->hasChanged());

    // Adicionar sensor PIR
    app::PirSensorConfig pirSensorCfg;
    pirSensorCfg.capability_name = "sensor_pir";
    pirSensorCfg.pin = 46; // pino de teste para sensor PIR
    pirSensorCfg.toleranceTime = 3; // 3 segundos
    auto *sensorPir = builder.addPirSensor(pirSensorCfg);
    TEST_ASSERT_NOT_NULL(sensorPir);
    list = builder.build();
    TEST_ASSERT_EQUAL(4, list.count);

    sensorPir->setup();

    TEST_ASSERT_FALSE(sensorPir->hasChanged());

    // Adicionar sensor de palmas (clap)
    app::ClapSensorConfig clapCfg;
    clapCfg.capability_name = "sensor_clap";
    clapCfg.pin = PIN_TEST; // pino de teste para sensor de palmas
    clapCfg.toleranceTime = 2; // 2 segundos
    auto *sensorClap = builder.addClapSensor(clapCfg);
    TEST_ASSERT_NOT_NULL(sensorClap);
    list = builder.build();
    TEST_ASSERT_EQUAL(5, list.count);

    sensorClap->setup();
    TEST_ASSERT_FALSE(sensorClap->hasChanged());
    sensorClap->handle();
    TEST_ASSERT_TRUE(sensorClap->isClapDetected());
    TEST_ASSERT_TRUE(sensorClap->hasChanged());

    // Simula o input de palmas no pino sem detectar palmas
    pinMode(PIN_TEST, OUTPUT);
    digitalWrite(PIN_TEST, LOW);
    delay(3000);
    sensorClap->handle();
    TEST_ASSERT_FALSE(sensorClap->isClapDetected());
    TEST_ASSERT_TRUE(sensorClap->hasChanged());
    

    // Adicionar Switch Plug
    app::SwitchPlugConfig switchCfg;
    switchCfg.capability_name = "switch_plug";
    switchCfg.pin = PIN_TEST; // pino de teste para switch plug
    switchCfg.switchLogic = DigitalLogic::NORMAL;
    auto *switchPlug = builder.addSwitchPlug(switchCfg);
    TEST_ASSERT_NOT_NULL(switchPlug);
    list = builder.build();
    TEST_ASSERT_EQUAL(6, list.count);

    switchPlug->setup();

    switchPlug->turnOn();
    TEST_ASSERT_TRUE(switchPlug->hasChanged());
    auto st = switchPlug->readState();
    TEST_ASSERT_EQUAL_STRING("on", st.value.c_str());
    delay(1000);
    switchPlug->turnOff();
    TEST_ASSERT_TRUE(switchPlug->hasChanged());
    st = switchPlug->readState();
    TEST_ASSERT_EQUAL_STRING("off", st.value.c_str());

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
    RUN_TEST(test_builder_capabilities);
    UNITY_END();
}

void loop() {}