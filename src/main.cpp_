#include <Arduino.h>
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Platform/Arduino/Providers/ArduinoTimeProvider.h"
#include "Contracts/Logging/Log.h"
#include "Contracts/Providers/Time.h"
#include "Contracts/Connections/WiFiManager.h"

#include "Contracts/Transports/IMqttClient.h"
#include "Core/Services/MqttService.h"
#include "Platform/Espressif/Mqtt/EspIdfMqttClient.h"

using namespace iotsmartsys;

static platform::arduino::ArduinoSerialLogger logger(Serial);
static platform::arduino::ArduinoTimeProvider timeProvider;

static app::WiFiManager wifi(logger);

static platform::esp32::EspIdfMqttClient mqttClient(logger);
static app::MqttService<12, 16, 256> mqtt(mqttClient, logger);

static void onMqttMessage(void *, const core::MqttMessageView &msg)
{
    logger.warn("MQTT RX payload_len=%lu retain=%d",
                (unsigned long)msg.payloadLen,
                (int)msg.retain);
}

void setup()
{
    Serial.begin(115200);
    delay(5000); // esperar serial
    Serial.println("[Serial] Starting IoT SmartSys Core example...");
    logger.setMinLevel(core::LogLevel::Warn);
    core::Log::setLogger(&logger);
    core::Time::setProvider(&timeProvider);
    logger.info("Logger and TimeProvider initialized.");

    app::WiFiConfig cfg;
    cfg.ssid = "IoT_SmartHome";
    cfg.password = "Ma522770";
    cfg.initialBackoffMs = 1000;
    cfg.maxBackoffMs = 60000;
    cfg.jitterMs = 300;
    cfg.autoReconnect = false;
    cfg.persistent = false;

    logger.warn("Basic_usageWiFi configuration set. SSID: %s", cfg.ssid);

    wifi.begin(cfg);

    logger.info("Configuring MQTT client...");
    core::MqttConfig mcfg;
    mcfg.uri = "mqtt://192.168.0.222:1883";
    mcfg.clientId = "esp32s3-basic-usage";
    mcfg.username = "smarthomeiot";
    mcfg.password = "Smarthomeiot@123";
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