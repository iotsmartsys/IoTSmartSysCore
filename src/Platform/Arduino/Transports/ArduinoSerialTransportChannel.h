#pragma once
#include <cstdint>
#include <cstddef>

#if !defined(ARDUINO)
#error "SerialTransportChannel é Arduino-only."
#endif

#include "Contracts/Transports/ITransportChannel.h"
#include <Arduino.h>

namespace iotsmartsys::core
{
    /// @brief Implementação UART (HardwareSerial) do ITransportChannel.
    /// @brief Protocolo: COBS(frame) + 0x00, com CRC16, suporta payload binário.
    class SerialTransportChannel final : public ITransportChannel
    {
    public:
        /// @brief rxPin/txPin (ESP32) são opcionais; se -1, usa begin(baud) padrão.
        SerialTransportChannel(HardwareSerial &port,
                               uint32_t baud,
                               int8_t rxPin = -1,
                               int8_t txPin = -1);

        bool begin(const TransportConfig &cfg) override;
        void start() override;
        void stop() override;
        void handle() override;

        bool isConnected() const override;

        bool publish(const char *topic,
                     const void *payload,
                     std::size_t len,
                     bool retain) override;

        bool subscribe(const char *topic) override;

        void setOnMessage(TransportOnMessageFn cb, void *user) override;
        void setOnConnected(TransportOnConnectedFn cb, void *user) override;
        void setOnDisconnected(TransportOnDisconnectedFn cb, void *user) override;

        /// @brief Chame no loop() para processar RX.
        void pump();

        /// @brief Opcional: envia uma mensagem completa (kind/id/origin/hops).
        bool publishEx(const TransportMessageView &msg);

        const char *getName() const override { return "ArduinoSerialTransportChannel"; }

    private:
        HardwareSerial &port_;
        uint32_t baud_;
        int8_t rxPin_;
        int8_t txPin_;

        TransportConfig cfg_{};

        bool started_{false};

        TransportOnMessageFn onMsg_{nullptr};
        TransportOnConnectedFn onConn_{nullptr};
        TransportOnDisconnectedFn onDisc_{nullptr};

        void *userMsg_{nullptr};
        void *userConn_{nullptr};
        void *userDisc_{nullptr};

        // Mensagem "connected" (entregue no callback)
        TransportConnectedView connected_{};

        // IDs locais (para publish() básico)
        uint32_t nextId_{1};

        // ---- Protocolo UART ----
        static constexpr uint8_t FRAME_VER = 1;

        // Tamanhos máximos (ajuste se precisar)
        static constexpr std::size_t MAX_TOPIC = 192;
        static constexpr std::size_t MAX_RAW_FRAME = 768;
        static constexpr std::size_t MAX_ENC_FRAME = (MAX_RAW_FRAME + (MAX_RAW_FRAME / 254) + 8);

        // RX: acumula bytes COBS até 0x00
        uint8_t rxEnc_[MAX_ENC_FRAME];
        std::size_t rxEncLen_{0};

        // RX/TX buffers
        uint8_t raw_[MAX_RAW_FRAME];
        uint8_t enc_[MAX_ENC_FRAME];

        char topicTmp_[MAX_TOPIC + 1]; // topic null-terminated para view

        static uint16_t crc16_ccitt_false(const uint8_t *data, std::size_t len);
        static std::size_t cobs_encode(const uint8_t *input, std::size_t length, uint8_t *output, std::size_t outMax);
        static std::size_t cobs_decode(const uint8_t *input, std::size_t length, uint8_t *output, std::size_t outMax);

        bool decodeAndDispatch(const uint8_t *frame, std::size_t len);

        static void write_u16_le(uint8_t *p, uint16_t v);
        static void write_u32_le(uint8_t *p, uint32_t v);
        static uint16_t read_u16_le(const uint8_t *p);
        static uint32_t read_u32_le(const uint8_t *p);

        bool sendFrame(const TransportMessageView &msg);

        // Controle: SUBSCRIBE via UART (opcional)
        bool sendSubscribeFrame(const char *topic);
    };
} // namespace iotsmartsys::core