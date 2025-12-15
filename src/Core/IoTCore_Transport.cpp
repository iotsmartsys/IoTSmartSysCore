#include "IoTSmartSysCore.h"
#include "Infra/Utils/Logger.h"

#ifdef ESP_NOW_ENABLED
#include "esp_now_utils/esp_now_utils.h"

static IoTCore *g_iotCoreForEspNow = nullptr;
static bool g_espNowBridgeActive = true;

void setEspNowMqttBridge(bool active) { g_espNowBridgeActive = active; }
bool isEspNowMqttBridgeEnabled() { return g_espNowBridgeActive; }

void __attribute__((weak)) sendNotify(String payload)
{
    if (!g_espNowBridgeActive)
        return;
    if (g_iotCoreForEspNow && g_iotCoreForEspNow->transport)
    {
        g_iotCoreForEspNow->transport->sendMessage("device/state", payload);
    }
}

extern "C" void espnow_on_peer_registered(const char *device_id)
{
    if (!g_iotCoreForEspNow)
        return;
    String did = String(device_id ? device_id : "");
    String name = did;

    String ssid = "ESPNOW";
    String signal = "";
    String ip = "";
    std::vector<CapabilityState> caps;

    DeviceAnnouncement ann(did, name, ssid, signal, ip, caps);
    g_iotCoreForEspNow->sendDeviceIncoming(ann);

#if defined(TRANSPORT_MQTT)

    if (g_iotCoreForEspNow->transport)
    {

        auto *mqttAdapter = static_cast<MqttTransportAdapter *>(g_iotCoreForEspNow->transport);
        if (mqttAdapter)
        {
            mqttAdapter->subscribeDeviceCommand(did);
        }
    }
#endif
}

void IoTCore::setupEspNow()
{
    g_iotCoreForEspNow = this;

    setup_esp_now();
    LOG_PRINTLN("[IoTCore] ESP-NOW gateway inicializado.");
}
#endif

void IoTCore::subscribe(const char *topic)
{
    try
    {
        if (transport)
        {
            LOG_PRINTLN("[IoTCore] Inscrevendo no tópico " + String(topic) + "...");
            transport->subscribe(topic);
        }
        else
            LOG_PRINTLN("[IoTCore] Erro: transporte não instanciado.");
    }
    catch (const std::exception &e)
    {
        // std::cerr << e.what() << '\n';
    }
}

void IoTCore::sendState(CapabilityState state)
{
    try
    {
        if (transport)
        {
            LOG_PRINTLN("[IoTCore] Enviando estado da capability " + state.capability_name + "...");
            transport->sendState(state);
        }
        else
            LOG_PRINTLN("[IoTCore] Erro: transporte não instanciado.");
    }
    catch (const std::exception &e)
    {
        // std::cerr << e.what() << '\n';
    }
}

void IoTCore::sendDeviceIncoming(DeviceAnnouncement announcement)
{
    try
    {
        if (transport)
        {
            LOG_PRINTLN("[IoTCore] Enviando estado do dispositivo " + announcement.device_name + "...");
            transport->sendMessage(discoveryTopic.c_str(), announcement.toJson());
        }
        else
            LOG_PRINTLN("[IoTCore] Erro: transporte não instanciado.");
    }
    catch (const std::exception &e)
    {
        // std::cerr << e.what() << '\n';
    }
}

bool IoTCore::isOk()
{
    if (transport)
    {
        if (!transport->isConnected())
        {
            LOG_PRINTLN("[IoTCore] Transporte não está conectado.");
            return false;
        }
    }

#if !defined(TRANSPORT_ESP_NOW)
    if (WiFi.status() != WL_CONNECTED)
    {
        LOG_PRINTLN("[IoTCore] WiFi não está conectado.");
        return false;
    }
#endif

    return true;
}

void IoTCore::notifyDeviceState()
{
    #if defined(NOTIFY_DEVICE_STATE_INTERVAL_MS)
    if (millis() - lastSendDeviceState > NOTIFY_DEVICE_STATE_INTERVAL_MS)
    {
        lastSendDeviceState = millis();
        LOG_PRINTLN("[IoTCore] Enviando estado do dispositivo...");

        String deviceId = getDeviceId();
#if !defined(TRANSPORT_ESP_NOW)
        int rssi = WiFi.RSSI();
        if (rssi != lastRSSI)
        {
            LOG_PRINTLN("[IoTCore] RSSI alterado: " + String(rssi));
            lastRSSI = rssi;

        transport->sendMessage(discoveryTopic.c_str(), PropertyState(deviceId, "wifi_signal", String(rssi)).toJson());
    }
#endif
        transport->sendMessage(discoveryTopic.c_str(), PropertyState(deviceId, "device_state", "online").toJson());
    }
    #endif
}
