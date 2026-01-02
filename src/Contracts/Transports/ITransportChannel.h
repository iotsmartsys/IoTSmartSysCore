#pragma once
#include <cstdint>
#include <cstddef>
#include "TransportMessageView.h"

namespace iotsmartsys::core
{

    using TransportOnMessageFn = void (*)(void *user, const TransportMessageView &msg);
    using TransportOnConnectedFn = void (*)(void *user, const TransportConnectedView &info);
    using TransportOnDisconnectedFn = void (*)(void *user);

    struct TransportConfig
    {
        const char *uri;
        const char *clientId;
        const char *username;
        const char *password;
        uint16_t keepAliveSec{30};
        bool cleanSession{true};
    };

    class ITransportChannel
    {
    public:
        virtual ~ITransportChannel() = default;

        virtual bool begin (const TransportConfig &cfg) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void handle() = 0;

        virtual bool isConnected() const = 0;

        virtual bool publish(const char *topic,
                             const void *payload,
                             std::size_t len,
                             bool retain) = 0; // QoS 0

        virtual bool subscribe(const char *topic) = 0; // QoS 0

        virtual void setOnMessage(TransportOnMessageFn cb, void *user) = 0;
        virtual void setOnConnected(TransportOnConnectedFn cb, void *user) = 0;
        virtual void setOnDisconnected(TransportOnDisconnectedFn cb, void *user) = 0;
        virtual const char *getName() const = 0;

    public:
        bool forwardRawMessages() const { return forwardRawMessages_; }
        void setForwardRawMessages(bool value) { forwardRawMessages_ = value; }

    protected:
        bool forwardRawMessages_{false};
    };

} // namespace iotsmartsys::core
