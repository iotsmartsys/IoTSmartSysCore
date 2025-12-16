#include <Arduino.h>
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Platform/Arduino/Providers/ArduinoTimeProvider.h"
#include "Contracts/Logging/Log.h"
#include "Contracts/Providers/Time.h"
#include "Contracts/Connections/WiFiManager.h"

using namespace iotsmartsys;

static platform::arduino::ArduinoSerialLogger logger(Serial);
static platform::arduino::ArduinoTimeProvider timeProvider;

static app::WiFiManager wifi(logger);

void setup()
{
    Serial.begin(115200);
    delay(5000); // esperar serial
    Serial.println("[Serial] Starting IoT SmartSys Core example...");

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

    logger.info("Basic_usageWiFi configuration set. SSID: %s", cfg.ssid);

    wifi.begin(cfg);
}

void loop()
{
    if (wifi.isConnected())
    {
        logger.info("Basic_usage WiFi is connected. IP Address: %s", WiFi.localIP().toString().c_str());
    }
    else
    {
        logger.warn("Basic_usageWiFi is not connected. Current state: %s", wifi.stateName());
    }
    wifi.handle();
    Serial.println("Basic_usage Looping...");
    delay(5);
}