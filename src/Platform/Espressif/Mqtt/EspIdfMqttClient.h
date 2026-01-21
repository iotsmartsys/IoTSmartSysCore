#pragma once
#ifdef ESP32

#include "Contracts/Mqtt/IMqttClient.h"
#include "Contracts/Logging/ILogger.h"
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
        bool republish(const iotsmartsys::core::TransportMessageView &msg) override;
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
        iotsmartsys::core::ILogger &_logger;
        esp_mqtt_client_handle_t _client{nullptr};
        bool _connected{false};

        iotsmartsys::core::TransportOnMessageFn _onMsg{nullptr};
        void *_onMsgUser{nullptr};
        iotsmartsys::core::TransportOnConnectedFn _onConnected{nullptr};
        void *_onConnectedUser{nullptr};
        iotsmartsys::core::TransportOnDisconnectedFn _onDisconnected{nullptr};
        void *_onDisconnectedUser{nullptr};
        std::string _clientIdStr;
        std::string _brokerStr;
        uint16_t _keepAliveSec{0};
    };

} // namespace iotsmartsys::platform::esp32

#else

#include "Contracts/Mqtt/IMqttClient.h"
#include "Contracts/Logging/ILogger.h"

namespace iotsmartsys::platform::espressif
{
    // Minimal stub implementation used when not compiling for ESP32.
    class EspIdfMqttClient : public iotsmartsys::core::IMqttClient
    {
    public:
        EspIdfMqttClient(iotsmartsys::core::ILogger &) {}
        ~EspIdfMqttClient() override = default;
        bool begin(const iotsmartsys::core::TransportConfig &) override { return false; }
        void start() override {}
        void stop() override {}
        void handle() override {}
        bool isConnected() const override { return false; }
        bool publish(const char *, const void *, std::size_t, bool) override { return false; }
        bool republish(const iotsmartsys::core::TransportMessageView &) override { return false; }
        bool subscribe(const char *) override { return false; }
        void setOnMessage(iotsmartsys::core::TransportOnMessageFn, void *) override {}
        void setOnConnected(iotsmartsys::core::TransportOnConnectedFn, void *) override {}
        void setOnDisconnected(iotsmartsys::core::TransportOnDisconnectedFn, void *) override {}
        const char *getName() const override { return "espid_stub"; }
    };

} // namespace iotsmartsys::platform::espressif

#endif
