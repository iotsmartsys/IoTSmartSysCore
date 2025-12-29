#include "EspIdfMqttBridge.h"
#include "esp_log.h"

using iotsmartsys::core::ConnectivityGate;

namespace iotsmartsys::platform::espressif
{

    static const char *TAG = "EspIdfMqttBridge";

    esp_err_t EspIdfMqttBridge::attach(esp_mqtt_client_handle_t client)
    {
        // registra eventos do mqtt
        return esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, &EspIdfMqttBridge::mqttHandler, nullptr);
    }

    esp_err_t EspIdfMqttBridge::detach(esp_mqtt_client_handle_t client)
    {
        // IDF não tem unregister fácil em versões antigas; se não tiver, ignore por enquanto.
        (void)client;
        return ESP_OK;
    }

    void EspIdfMqttBridge::mqttHandler(void *, esp_event_base_t base, int32_t event_id, void *event_data)
    {
        (void)base;
        (void)event_data;
        auto &gate = ConnectivityGate::instance();

        switch (event_id)
        {
        case MQTT_EVENT_CONNECTED:
            gate.setBits(ConnectivityGate::MQTT_CONNECTED);
            ESP_LOGI(TAG, "MQTT_CONNECTED set");
            break;

        case MQTT_EVENT_DISCONNECTED:
        case MQTT_EVENT_ERROR:
            gate.clearBits(ConnectivityGate::MQTT_CONNECTED);
            ESP_LOGW(TAG, "MQTT_CONNECTED cleared");
            break;

        default:
            break;
        }
    }

} // namespace iotsmartsys::platform::espressif