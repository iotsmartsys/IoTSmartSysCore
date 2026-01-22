#pragma once
#ifdef ESP8266

#include "Contracts/Mqtt/IMqttClient.h"
#include "Contracts/Logging/ILogger.h"

#include <string>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#include <PubSubClient.h>

namespace iotsmartsys::platform::esp8266
{
    class Esp8266PubSubMqttClient : public iotsmartsys::core::IMqttClient
    {
    public:
        explicit Esp8266PubSubMqttClient(iotsmartsys::core::ILogger &log);
        ~Esp8266PubSubMqttClient() override = default;

        bool begin(const iotsmartsys::core::TransportConfig &cfg) override;
        void start() override;
        void stop() override;
        void handle() override;

        bool isConnected() const override;

        bool publish(const char *topic, const void *payload, std::size_t len, bool retain) override;
        bool republish(const iotsmartsys::core::TransportMessageView &msg) override;
        bool subscribe(const char *topic) override;

        void setOnMessage(iotsmartsys::core::TransportOnMessageFn cb, void *user) override;
        void setOnConnected(iotsmartsys::core::TransportOnConnectedFn cb, void *user) override;
        void setOnDisconnected(iotsmartsys::core::TransportOnDisconnectedFn cb, void *user) override;

        const char *getName() const override { return _clientIdStr.c_str(); }

    private:
        struct ParsedUri
        {
            bool tls{false};
            std::string host;
            uint16_t port{0};
        };

        static bool parseUri(const char *uri, ParsedUri &out);

        static void onPubSubMessageThunk(char *topic, uint8_t *payload, unsigned int length);
        void onPubSubMessage(char *topic, uint8_t *payload, unsigned int length);

        void applyTlsDefaults(); // por enquanto: mqtts => insecure

    private:
        iotsmartsys::core::ILogger &_logger;

        WiFiClient _netPlain;
        WiFiClientSecure _netSecure;
        mutable PubSubClient _mqtt;

        bool _useTls{false};
        bool _connected{false};

        iotsmartsys::core::TransportConfig _cfg{};
        std::string _clientIdStr;
        std::string _userStr;
        std::string _passStr;
        std::string _subTopicStr;
        std::string _pubTopicStr;
        std::string _hostStr;
        uint16_t _port{0};

        // callbacks do seu contrato
        iotsmartsys::core::TransportOnMessageFn _onMsg{nullptr};
        void *_onMsgUser{nullptr};
        iotsmartsys::core::TransportOnConnectedFn _onConnected{nullptr};
        void *_onConnectedUser{nullptr};
        iotsmartsys::core::TransportOnDisconnectedFn _onDisconnected{nullptr};
        void *_onDisconnectedUser{nullptr};
    };

} // namespace iotsmartsys::platform::esp8266

#endif
