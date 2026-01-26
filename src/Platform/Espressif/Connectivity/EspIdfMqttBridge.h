// Only compile the ESP-IDF MQTT bridge on ESP32 targets
#if defined(ESP32)

#pragma once
#include "Contracts/Connectivity/ConnectivityGate.h"
#include "mqtt_client.h"

namespace iotsmartsys::platform::espressif
{

    class EspIdfMqttBridge
    {
    public:
        static esp_err_t attach(esp_mqtt_client_handle_t client);
        static esp_err_t detach(esp_mqtt_client_handle_t client);

    private:
        static void mqttHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
    };

} // namespace iotsmartsys::platform::espressif

#endif // ESP32