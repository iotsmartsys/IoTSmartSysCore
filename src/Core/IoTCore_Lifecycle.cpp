#include <Arduino.h>
#include "IoTSmartSysCore.h"
#include "Utils/Logger.h"

#include "OTA/OTA.h"
#include "OTA/OTAManager.h"
#include "Settings/ConfigManager.h"
#include "Settings/PortalConfiguration.h"
#include "Capabilities/Capability.h"
#include <vector>

#if defined(NODE_BLE_ENABLED)
#include "Core/BLE.h"
#endif

bool inRecoveryMode = false;
unsigned long lastRecoveryAttempt = 0;

IoTCore::IoTCore()
    : transport(nullptr), discoveryTopic(DISCOVERY_TOPIC_DEFAULT)
{
}

IoTCore::IoTCore(const char *device_name, std::vector<Capability *> capabilities)
    : transport(nullptr),
      capabilities(capabilities),
      device_name(device_name ? device_name : ""),
      discoveryTopic(DISCOVERY_TOPIC_DEFAULT)
{
}

IoTCore::~IoTCore()
{
    if (transport)
    {
        delete transport;
        transport = nullptr;
    }
    for (auto *cap : capabilities)
    {
        delete cap;
    }
    capabilities.clear();

    for (auto *prop : properties)
    {
        delete prop;
    }
    properties.clear();

    if (ledCapability)
    {
        delete ledCapability;
        ledCapability = nullptr;
    }

    if (capabilityBuilder)
    {
        delete capabilityBuilder;
        capabilityBuilder = nullptr;
    }
}

LEDCapability &IoTCore::configureLEDControl(int ledPin, DigitalLogic ledLogic)
{
    ledCapability = new LEDCapability("led_control", ledPin, ledLogic);
    return *ledCapability;
}

void IoTCore::setup()
{
#ifdef BAUD_RATE
    LOG_BEGIN(BAUD_RATE);
#else
    LOG_BEGIN(115200);
#endif
    LOG_INFO("[IoTCore] Iniciando setup...");

    ConfigManager::instance().loadConfigFromPreferences();
    settings = &ConfigManager::instance().get();

    BootstrapDevice::initialize(*(Settings *)settings);
    resolveDiscoveryTopic();

#if !defined(TRANSPORT_ESP_NOW)

    setupWifi(settings->wifi.ssid.c_str(), settings->wifi.password.c_str());

    if (WiFi.status() != WL_CONNECTED)
    {
        LOG_ERROR("[IoTCore] Erro: Não foi possível conectar ao Wi‑Fi.");
    }
#endif
    resolveDeviceIdentity();
    ConfigManager::instance().loadConfig();
    if (capabilities.size() == 0)
    {
        this->capabilities = capabilityBuilder->build();
    }
    for (const auto &cap : capabilities)
    {
        try
        {
            if (cap->capability_name == "" || cap->capability_name == nullptr)
            {
                cap->applyRenamedName(getDeviceId());
            }
            cap->setup();
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("[IoTCore] Erro ao configurar capability: " + cap->capability_name);
        }
    }

    OTA::setup(settings->firmware);
#if defined(NODE_BLE_ENABLED)
    setupBLEGateway();
#endif

#if defined(TRANSPORT_ESP_NOW) && defined(TRANSPORT_MQTT)
#error "Defina apenas um transporte: TRANSPORT_MQTT ou TRANSPORT_ESP_NOW"
#endif

#if defined(TRANSPORT_ESP_NOW)
    transport = new EspNowClientHandler(device_name.c_str(), capabilities);
#else
    addDefaultProperties();
    addProperty("broker", settings->mqtt.primary.host);
    transport = new MqttTransportAdapter(device_name.c_str(), settings->mqtt, capabilities, properties);
#endif

    transport->setup();

    if (ledCapability != nullptr)
    {
        ledCapability->setup();
        ledCapability->turnOn();
    }

#ifdef ESP_NOW_ENABLED

#if !defined(TRANSPORT_ESP_NOW)
    setupEspNow();
#endif
#endif

    LOG_INFO("[IoTCore] Setup concluído.");
}

void IoTCore::handle()
{

#if !defined(TRANSPORT_ESP_NOW)

    if (WiFi.status() != WL_CONNECTED)
    {
        if (!inRecoveryMode)
        {
            LOG_PRINTLN("[IoTCore] Entrando em modo de recuperação: Wi‑Fi desconectado.");
            inRecoveryMode = true;
        }

        if (ledCapability)
            ledCapability->blink(2000);

        if (millis() - lastRecoveryAttempt > 10000)
        {
            lastRecoveryAttempt = millis();
            LOG_PRINTLN("[IoTCore] Tentando reconectar Wi‑Fi...");
            maintainWiFiConnection(settings->wifi.ssid.c_str(), settings->wifi.password.c_str());
        }

        return;
    }

    if (inRecoveryMode)
    {
        LOG_PRINTLN("[IoTCore] Wi‑Fi reconectado. Saindo do modo de recuperação.");
        inRecoveryMode = false;
    }
#endif

    try
    {
        if (transport)
            transport->handle();

        if (transport && transport->isPowerOn())
        {
            if (ledCapability != nullptr && ledCapability->isOn() == false)
                ledCapability->turnOn();

            capabilitiesHandle();
        }
        else
        {
            LOG_PRINTLN("[IoTCore] Dispositivo está desligado. Ignorando loop.");
            if (ledCapability != nullptr)
                ledCapability->blink(2500);
        }

        notifyDeviceState();

        switch (settings->firmware.update)
        {
        case FirmwareUpdateMethod::OTA:
            handleOTA();
            break;
        case FirmwareUpdateMethod::AUTO:
            // LOG_DEBUG("[IoTCore] Atualizações automáticas de firmware habilitadas.");
            break;
        default:
            // LOG_DEBUG("[IoTCore] Atualizações de firmware desabilitadas.");
            break;
        }
    }
    catch (const std::exception &e)
    {
        LOG_PRINTLN("[IoTCore] Erro ao processar loop principal.");
        std::cerr << e.what() << '\n';
    }

#if defined(NODE_BLE_ENABLED)
    handleBLEGateway(transport);
#endif
}

void IoTCore::setBuild(const String &build)
{
    addProperty("build", build);
}

void IoTCore::resolveDeviceIdentity()
{
    if (device_name.length() == 0)
    {
        String deviceId = getDeviceId();
        if (deviceId.length() == 0)
        {
            deviceId = getMacAddress();
        }
        device_name = deviceId;
    }
}

void IoTCore::resolveDiscoveryTopic()
{
    discoveryTopic = DISCOVERY_TOPIC_DEFAULT;
    if (settings && settings->mqtt.announce_topic.length() > 0)
    {
        discoveryTopic = settings->mqtt.announce_topic;
    }
}
