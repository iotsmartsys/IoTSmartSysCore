#pragma once
#ifdef ESP32

#include "Contracts/Transports/IMqttClient.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Logging/Log.h"

extern "C"
{
#include "mqtt_client.h"
}

namespace iotsmartsys::platform::esp32
{

    class EspIdfMqttClient : public iotsmartsys::core::IMqttClient
    {
    public:
        EspIdfMqttClient(iotsmartsys::core::ILogger &log);
        ~EspIdfMqttClient() override;

        bool begin(const iotsmartsys::core::MqttConfig &cfg) override;
        void start() override;
        void stop() override;

        bool isConnected() const override;

        bool publish(const char *topic, const void *payload, std::size_t len, bool retain) override;
        bool subscribe(const char *topic) override;

        void setOnMessage(iotsmartsys::core::MqttOnMessageFn cb, void *user) override;

    private:
        static esp_err_t eventHandlerThunk(esp_mqtt_event_handle_t event);
        // bridge para o esp_event_handler_t (usado por esp_mqtt_client_register_event em algumas IDFs)
        static esp_err_t eventHandlerBridge(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
        esp_err_t onEvent(esp_mqtt_event_handle_t event);

    private:
        iotsmartsys::core::ILogger &_logger;
        esp_mqtt_client_handle_t _client{nullptr};
        bool _connected{false};

        iotsmartsys::core::MqttOnMessageFn _onMsg{nullptr};
        void *_onMsgUser{nullptr};

        // buffers para evitar alocações internas do mqtt_event (topic/payload não são null-terminated)
        // (opcional: pode processar direto via len)
    };

} // namespace iotsmartsys::platform::esp32
#endif