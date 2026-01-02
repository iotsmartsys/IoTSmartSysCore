#pragma once
#ifdef ESP32

#include "Contracts/Transports/ITransportChannel.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Logging/Log.h"
#include <string>

extern "C"
{
#include "mqtt_client.h"
}

namespace iotsmartsys::core
{

    class IMqttClient : public iotsmartsys::core::ITransportChannel
    {
    public:
        explicit IMqttClient(iotsmartsys::core::ILogger &logger) : _logger(logger) {}
        bool begin(const iotsmartsys::core::TransportConfig &cfg) override = 0;
        void start() override = 0;
        void stop() override = 0;
        void handle() override {}

        bool isConnected() const override = 0;

        bool publish(const char *topic, const void *payload, std::size_t len, bool retain) override = 0;
        bool subscribe(const char *topic) override = 0;

        void setOnMessage(iotsmartsys::core::TransportOnMessageFn cb, void *user) override = 0;
        void setOnConnected(iotsmartsys::core::TransportOnConnectedFn cb, void *user) override = 0;
        void setOnDisconnected(iotsmartsys::core::TransportOnDisconnectedFn cb, void *user) override = 0;
        const char *getName() const override { return _clientIdStr.c_str(); }

    private:
        static esp_err_t eventHandlerThunk(esp_mqtt_event_handle_t event);
        // bridge para o esp_event_handler_t (usado por esp_mqtt_client_register_event em algumas IDFs)
        static esp_err_t eventHandlerBridge(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
        esp_err_t onEvent(esp_mqtt_event_handle_t event);

    protected:
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
#endif
