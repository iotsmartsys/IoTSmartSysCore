#pragma once

#include <cstdint>
#include <cstddef>

namespace iotsmartsys::core
{

    enum class TransportKind : uint8_t
    {
        Event = 0,
        Command = 1,
        Ack = 2,
        Raw = 3,
    };

    struct TransportMessageView
    {
        const char *topic;
        const char *payload; // pode conter '\0' (use len)
        std::size_t payloadLen;
        bool retain;

        TransportKind kind{TransportKind::Raw};

        uint32_t id{0};
        uint8_t origin{0};
        uint8_t hops{0};
    };

    struct TransportConnectedView
    {
        const char *clientId;
        const char *broker;
        uint16_t keepAliveSec;
    };
}