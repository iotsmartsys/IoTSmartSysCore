#pragma once
#include <cstdint>

namespace iotsmartsys::core
{

    class IEventLatch;

    class ConnectivityGate
    {
    public:
        static constexpr uint32_t WIFI_CONNECTED = (1u << 0);
        static constexpr uint32_t IP_READY = (1u << 1);
        static constexpr uint32_t MQTT_CONNECTED = (1u << 2);

        static constexpr uint32_t NETWORK_READY_BITS = WIFI_CONNECTED | IP_READY;

        static ConnectivityGate &instance();

        // Deve ser chamado pela plataforma (1x) no boot, antes de usar.
        static void init(IEventLatch &latch);

        uint32_t bits() const;
        bool isSet(uint32_t requiredBits) const;

        bool isNetworkReady() const { return isSet(NETWORK_READY_BITS); }
        bool isMqttReady() const { return isSet(NETWORK_READY_BITS | MQTT_CONNECTED); }

        bool waitBits(uint32_t requiredBits, int32_t timeoutMs = -1);
        bool waitNetworkReady(int32_t timeoutMs = -1) { return waitBits(NETWORK_READY_BITS, timeoutMs); }
        bool waitMqttReady(int32_t timeoutMs = -1) { return waitBits(NETWORK_READY_BITS | MQTT_CONNECTED, timeoutMs); }

        // Chamado pela plataforma (handlers) para atualizar estado
        void setBits(uint32_t bitsToSet);
        void clearBits(uint32_t bitsToClear);

    private:
        ConnectivityGate() = default;
        IEventLatch *_latch = nullptr;
    };

} // namespace iotsmartsys::core