#pragma once
#ifdef ESP32

#include "Contracts/Transports/ITransportChannel.h"
#include "Core/Services/IMqttClient.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Logging/Log.h"
#include <string>

extern "C"
{
#include "mqtt_client.h"
}

namespace iotsmartsys::platform::espressif
{

    class EspIdfMqttClient : public iotsmartsys::core::IMqttClient
    {
    public:
        EspIdfMqttClient(iotsmartsys::core::ILogger &log);
        ~EspIdfMqttClient() override;

        bool begin(const iotsmartsys::core::TransportConfig &cfg) override;
        void start() override;
        void stop() override;
        void handle() override {}

        bool isConnected() const override;

        bool publish(const char *topic, const void *payload, std::size_t len, bool retain) override;
        bool subscribe(const char *topic) override;

        void setOnMessage(iotsmartsys::core::TransportOnMessageFn cb, void *user) override;
        void setOnConnected(iotsmartsys::core::TransportOnConnectedFn cb, void *user) override;
        void setOnDisconnected(iotsmartsys::core::TransportOnDisconnectedFn cb, void *user) override;
        const char *getName() const override { return _clientIdStr.c_str(); }

    private:
        static esp_err_t eventHandlerThunk(esp_mqtt_event_handle_t event);
        // bridge para o esp_event_handler_t (usado por esp_mqtt_client_register_event em algumas IDFs)
        static esp_err_t eventHandlerBridge(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
        esp_err_t onEvent(esp_mqtt_event_handle_t event);

    private:
        // buffers para evitar alocações internas do mqtt_event (topic/payload não são null-terminated)
        // (opcional: pode processar direto via len)
    };

} // namespace iotsmartsys::platform::esp32
#endif
