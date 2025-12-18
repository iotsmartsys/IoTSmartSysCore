#ifndef UNIT_TEST
#include <Arduino.h>
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Platform/Arduino/Providers/ArduinoTimeProvider.h"
#include "Contracts/Logging/Log.h"
#include "Contracts/Providers/Time.h"
#include "Contracts/Connections/WiFiManager.h"

#include "Contracts/Transports/IMqttClient.h"
#include "Core/Services/MqttService.h"
#include "Platform/Espressif/Mqtt/EspIdfMqttClient.h"
#include "Core/Sinks/MqttSink.h"
#include "Contracts/Events/CapabilityStateChanged.h"
#include "Contracts/Capabilities/LightCapability.h"
#include "Contracts/Capabilities/PushButtonCapability.h"
#include "Platform/Arduino/Adapters/RelayHardwareAdapter.h"
#include "Platform/Arduino/Adapters/InputHardwareAdapter.h"
#include "Platform/Espressif/Pinouts/ESP32_S3_Pinouts.h"

using namespace iotsmartsys;

static platform::arduino::ArduinoSerialLogger logger(Serial);
static platform::arduino::ArduinoTimeProvider timeProvider;

static app::WiFiManager wifi(logger);

static platform::esp32::EspIdfMqttClient mqttClient(logger);
static app::MqttService<12, 16, 256> mqtt(mqttClient, logger);
static core::MqttSink *mqttSink = new core::MqttSink(mqttClient);

static void onMqttMessage(void *, const core::MqttMessageView &msg)
{
    logger.warn("MQTT RX payload_len=%lu retain=%d",
                (unsigned long)msg.payloadLen,
                (int)msg.retain);
}
#define BUTTON_PIN ESP32_S3_GPIO0
#define LED_PIN PIN_TEST

platform::arduino::RelayHardwareAdapter relayAdapter(LED_PIN, platform::arduino::HardwareDigitalLogic::LOW_IS_ON);
core::LightCapability lightCap("living_room", relayAdapter, mqttSink);
platform::arduino::InputHardwareAdapter buttonAdapter(BUTTON_PIN, platform::arduino::HardwareDigitalLogic::LOW_IS_ON, platform::arduino::InputPullMode::UP);
core::PushButtonCapability buttonCap("button", buttonAdapter, mqttSink);
unsigned long lastButtonCheck = 0;
void simulatedButtonPress()
{
    buttonCap.handle();
    if (buttonCap.isPressed())
    {
        logger.info("Button pressed, LED state toggled.");
        lightCap.toggle();
        lastButtonCheck = millis();
        logger.info("LED is now %s", lightCap.isOn() ? "ON" : "OFF");
    }
    else
    {
        // logger.info("Button not pressed.");
    }
    if (millis() - lastButtonCheck > 5000 && lastButtonCheck != 0)
    {
        digitalWrite(43, !digitalRead(43));
        lastButtonCheck = millis();
    }

    lightCap.handle();

    delay(200); // debounce
}

void setup()
{
    Serial.begin(115200);
    delay(500); // esperar serial

    buttonAdapter.setup();
    relayAdapter.setup();

    Serial.println("[Serial] Starting IoT SmartSys Core example...");
    logger.setMinLevel(core::LogLevel::Debug);
    core::Log::setLogger(&logger);
    core::Time::setProvider(&timeProvider);
    logger.info("Logger and TimeProvider initialized.");

    app::WiFiConfig cfg;
    cfg.ssid = WIFI_SSID;
    cfg.password = WIFI_PASSWORD;
    cfg.initialBackoffMs = 1000;
    cfg.maxBackoffMs = 60000;
    cfg.jitterMs = 300;
    cfg.autoReconnect = false;
    cfg.persistent = false;

    logger.warn("Basic_usageWiFi configuration set. SSID: %s", cfg.ssid);

    wifi.begin(cfg);

    logger.info("Configuring MQTT client...");
    core::MqttConfig mcfg;
    mcfg.clientId = "esp32s3-basic-usage";
    mcfg.uri = MQTT_BROKER;
    mcfg.username = MQTT_USERNAME;
    mcfg.password = MQTT_PASSWORD;
    mcfg.keepAliveSec = 30;
    mcfg.cleanSession = true;

    logger.info("Starting MQTT client...");
    mqtt.begin(mcfg);
    logger.info("MQTT client started.");

    mqtt.setOnMessage(&onMqttMessage, nullptr);
    logger.info("MQTT message handler set.");
    mqtt.subscribe("iotsmartsys/cmd/#");
    logger.info("MQTT client subscribed to topic.");
}

void loop()
{
    simulatedButtonPress();
    wifi.handle();
    mqtt.handle();

    static uint32_t last = 0;
    const uint32_t now = (uint32_t)timeProvider.nowMs();
    if (now - last >= 5000)
    {
        last = now;
        const char payload[] = "{\"alive\":true}";
        mqtt.publish("iotsmartsys/state/alive", payload, sizeof(payload) - 1, false);

        // Log status occasionally (not every loop)
        if (wifi.isConnected())
        {
            logger.info("WiFi OK IP=%s MQTT=%s",
                        WiFi.localIP().toString().c_str(),
                        mqtt.isOnline() ? "online" : "offline");
        }
        else
        {
            logger.warn("WiFi down state=%s MQTT=%s",
                        wifi.stateName(),
                        mqtt.isOnline() ? "online" : "offline");
        }
    }

    delay(2);
}
#endif