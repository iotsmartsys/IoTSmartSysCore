#include "ArduinoSerialTransportChannel.h"
#include <cstring>

namespace iotsmartsys::core
{
    SerialTransportChannel::SerialTransportChannel(HardwareSerial &port, uint32_t baud, int8_t rxPin, int8_t txPin)
        : port_(port), baud_(baud), rxPin_(rxPin), txPin_(txPin)
    {
        forwardRawMessages_ = true;
        std::memset(&cfg_, 0, sizeof(cfg_));
        std::memset(&connected_, 0, sizeof(connected_));
    }

    bool SerialTransportChannel::begin(const TransportConfig &cfg)
    {
        cfg_ = cfg;

        connected_.clientId = cfg_.clientId ? cfg_.clientId : "";
        connected_.broker = cfg_.uri ? cfg_.uri : "serial://";
        connected_.keepAliveSec = cfg_.keepAliveSec;

        return true;
    }

    void SerialTransportChannel::start()
    {
        if (started_)
        {
            return;
        }

#if defined(ESP32)
        if (rxPin_ >= 0 && txPin_ >= 0)
        {
            port_.begin(baud_, SERIAL_8N1, rxPin_, txPin_);
        }
        else
        {
            port_.begin(baud_);
        }
#else
        port_.begin(baud_);
#endif

        rxEncLen_ = 0;
        started_ = true;

        if (onConn_)
        {
            onConn_(userConn_, connected_);
        }
    }

    void SerialTransportChannel::stop()
    {
        if (!started_)
        {
            return;
        }

#if defined(ESP32)
        port_.end();
#endif

        started_ = false;

        if (onDisc_)
        {
            onDisc_(userDisc_);
        }
    }

    bool SerialTransportChannel::isConnected() const
    {
        // UART não tem handshake: consideramos "conectado" se está iniciado.
        return started_;
    }

    void SerialTransportChannel::setOnMessage(TransportOnMessageFn cb, void *user)
    {
        onMsg_ = cb;
        userMsg_ = user;
    }

    void SerialTransportChannel::setOnConnected(TransportOnConnectedFn cb, void *user)
    {
        onConn_ = cb;
        userConn_ = user;
    }

    void SerialTransportChannel::setOnDisconnected(TransportOnDisconnectedFn cb, void *user)
    {
        onDisc_ = cb;
        userDisc_ = user;
    }

    bool SerialTransportChannel::publish(const char *topic, const void *payload, std::size_t len, bool retain)
    {
        (void)topic;
        (void)retain;

        if (!started_)
        {
            return false;
        }

        if (payload && len)
        {
            port_.write((const uint8_t *)payload, len);
        }
        port_.write((uint8_t)'\n');
        return true;
    }

    bool SerialTransportChannel::publishEx(const TransportMessageView &msg)
    {
        if (!started_)
        {
            return false;
        }

        // For compatibility with the current ESP32-C6 gateway (newline JSON),
        // send only the payload bytes (topic/kind/id/origin/hops are not encoded on the wire).
        if (msg.payload && msg.payloadLen)
        {
            port_.write((const uint8_t *)msg.payload, msg.payloadLen);
        }
        port_.write((uint8_t)'\n');
        return true;
    }

    bool SerialTransportChannel::subscribe(const char *topic)
    {
        (void)topic;
        return true;
    }

    bool SerialTransportChannel::sendSubscribeFrame(const char *topic)
    {
        if (!started_ || !topic)
        {
            return false;
        }

        // Envia um frame tipo Command em um tópico reservado "__sub"
        // payload = topic que deseja assinar.
        TransportMessageView msg;
        msg.topic = "__sub";
        msg.payload = topic;
        msg.payloadLen = std::strlen(topic);
        msg.retain = false;
        msg.kind = TransportKind::Command;
        msg.id = nextId_++;
        msg.origin = 0;
        msg.hops = 0;

        return sendFrame(msg);
    }

    void SerialTransportChannel::pump()
    {
        if (!started_)
        {
            return;
        }

        while (port_.available() > 0)
        {
            int v = port_.read();
            if (v < 0)
            {
                break;
            }

            const uint8_t b = (uint8_t)v;

            // Ignore CR to support \r\n
            if (b == (uint8_t)'\r')
            {
                continue;
            }

            // Newline delimits a complete message (the ESP32-C6 gateway sends one JSON per line)
            if (b == (uint8_t)'\n')
            {
                if (rxEncLen_ > 0)
                {
                    (void)decodeAndDispatch(rxEnc_, rxEncLen_);
                    rxEncLen_ = 0;
                }
                else
                {
                    rxEncLen_ = 0;
                }
                continue;
            }

            if (rxEncLen_ < sizeof(rxEnc_))
            {
                rxEnc_[rxEncLen_++] = b;
            }
            else
            {
                // Overflow: discard current line
                rxEncLen_ = 0;
            }
        }
    }

    // ---------------- Protocol helpers ----------------

    void SerialTransportChannel::write_u16_le(uint8_t *p, uint16_t v)
    {
        p[0] = (uint8_t)(v & 0xFF);
        p[1] = (uint8_t)((v >> 8) & 0xFF);
    }

    void SerialTransportChannel::write_u32_le(uint8_t *p, uint32_t v)
    {
        p[0] = (uint8_t)(v & 0xFF);
        p[1] = (uint8_t)((v >> 8) & 0xFF);
        p[2] = (uint8_t)((v >> 16) & 0xFF);
        p[3] = (uint8_t)((v >> 24) & 0xFF);
    }

    uint16_t SerialTransportChannel::read_u16_le(const uint8_t *p)
    {
        return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
    }

    uint32_t SerialTransportChannel::read_u32_le(const uint8_t *p)
    {
        return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    }

    uint16_t SerialTransportChannel::crc16_ccitt_false(const uint8_t *data, std::size_t len)
    {
        uint16_t crc = 0xFFFF;
        for (std::size_t i = 0; i < len; i++)
        {
            crc ^= (uint16_t)data[i] << 8;
            for (int b = 0; b < 8; b++)
            {
                if (crc & 0x8000)
                    crc = (uint16_t)((crc << 1) ^ 0x1021);
                else
                    crc = (uint16_t)(crc << 1);
            }
        }
        return crc;
    }

    std::size_t SerialTransportChannel::cobs_encode(const uint8_t *input, std::size_t length, uint8_t *output, std::size_t outMax)
    {
        if (!input || !output || outMax < 2)
        {
            return 0;
        }

        std::size_t read_index = 0;
        std::size_t write_index = 1;
        std::size_t code_index = 0;
        uint8_t code = 1;

        while (read_index < length)
        {
            if (input[read_index] == 0)
            {
                output[code_index] = code;
                code = 1;
                code_index = write_index++;
                if (write_index >= outMax)
                {
                    return 0;
                }
                read_index++;
            }
            else
            {
                output[write_index++] = input[read_index++];
                if (write_index >= outMax)
                {
                    return 0;
                }
                code++;
                if (code == 0xFF)
                {
                    output[code_index] = code;
                    code = 1;
                    code_index = write_index++;
                    if (write_index >= outMax)
                    {
                        return 0;
                    }
                }
            }
        }

        output[code_index] = code;
        return write_index;
    }

    std::size_t SerialTransportChannel::cobs_decode(const uint8_t *input, std::size_t length, uint8_t *output, std::size_t outMax)
    {
        if (!input || !output || length == 0)
        {
            return 0;
        }

        std::size_t read_index = 0;
        std::size_t write_index = 0;

        while (read_index < length)
        {
            uint8_t code = input[read_index];
            if (code == 0)
            {
                return 0;
            }
            read_index++;

            for (uint8_t i = 1; i < code; i++)
            {
                if (read_index >= length || write_index >= outMax)
                {
                    return 0;
                }
                output[write_index++] = input[read_index++];
            }

            if (code != 0xFF && read_index < length)
            {
                if (write_index >= outMax)
                {
                    return 0;
                }
                output[write_index++] = 0;
            }
        }

        return write_index;
    }

    // Frame layout (raw, antes de COBS):
    // ver(1) kind(1) flags(1) origin(1) hops(1) id(4) topicLen(2) payloadLen(4) topic(N) payload(M) crc16(2)
    bool SerialTransportChannel::sendFrame(const TransportMessageView &msg)
    {
        if (!started_ || !msg.topic)
        {
            return false;
        }

        const std::size_t topicLen = std::strlen(msg.topic);
        const std::size_t payloadLen = (msg.payload && msg.payloadLen) ? msg.payloadLen : 0;

        // Header = 1+1+1+1+1+4+2+4 = 15 bytes, CRC=2
        const std::size_t totalRaw = 15 + topicLen + payloadLen + 2;
        if (totalRaw > sizeof(raw_))
        {
            return false;
        }

        std::size_t p = 0;
        raw_[p++] = FRAME_VER;
        raw_[p++] = (uint8_t)msg.kind;
        raw_[p++] = (uint8_t)(msg.retain ? 0x01 : 0x00);
        raw_[p++] = msg.origin;
        raw_[p++] = msg.hops;

        write_u32_le(&raw_[p], msg.id);
        p += 4;

        write_u16_le(&raw_[p], (uint16_t)topicLen);
        p += 2;

        write_u32_le(&raw_[p], (uint32_t)payloadLen);
        p += 4;

        std::memcpy(&raw_[p], msg.topic, topicLen);
        p += topicLen;

        if (payloadLen && msg.payload)
        {
            std::memcpy(&raw_[p], msg.payload, payloadLen);
            p += payloadLen;
        }

        const uint16_t crc = crc16_ccitt_false(raw_, p);
        write_u16_le(&raw_[p], crc);
        p += 2;

        const std::size_t encLen = cobs_encode(raw_, p, enc_, sizeof(enc_));
        if (encLen == 0)
        {
            return false;
        }

        port_.write(enc_, encLen);
        port_.write((uint8_t)0x00);

        return true;
    }

    bool SerialTransportChannel::decodeAndDispatch(const uint8_t *frame, std::size_t len)
    {
        if (!frame || len == 0)
        {
            return false;
        }

        // Copy into raw_ so payload pointer stays valid and we can optionally null-terminate
        if (len >= sizeof(raw_))
        {
            // Too large: drop
            return false;
        }

        std::memcpy(raw_, frame, len);
        raw_[len] = 0; // safe terminator (payloadLen still controls the true size)

        // Provide a fixed topic for line-based UART input (keeps compatibility with existing app logic)
        static const char *kTopic = "uart/raw";
        std::strncpy(topicTmp_, kTopic, MAX_TOPIC);
        topicTmp_[MAX_TOPIC] = '\0';

        TransportMessageView msg;
        msg.topic = topicTmp_;
        msg.payload = (const char *)raw_;
        msg.payloadLen = len;
        msg.retain = false;
        msg.kind = TransportKind::Raw;
        msg.id = nextId_++;
        msg.origin = 0;
        msg.hops = 0;

        if (onMsg_)
        {
            onMsg_(userMsg_, msg);
        }

        return true;
    }

    void SerialTransportChannel::handle()
    {
        pump();
    }
} // namespace iotsmartsys::core