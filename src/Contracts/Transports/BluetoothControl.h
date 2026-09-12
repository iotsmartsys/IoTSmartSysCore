#pragma once

#include <cstddef>
#include <cstdint>

namespace iotsmartsys::core
{
    struct BluetoothControlConfig
    {
        const uint8_t *sharedSecret{nullptr};
        std::size_t sharedSecretSize{0};
        bool localControlsConfigured{false};
        bool deviceInfoEnabled{true};
        uint32_t pairingWindowMs{60000};
        uint32_t securityTimeoutMs{15000};
        uint32_t authTimeoutMs{5000};
        uint32_t receiveTimeoutMs{5000};
        uint32_t sessionTimeoutMs{30000};
        uint32_t disconnectTimeoutMs{2000};
    };

    enum class BluetoothControlState : uint8_t
    {
        Disabled, Unavailable, Advertising, Connected, Securing, Authenticating,
        Authorized, Receiving, Dispatching, Responding, AwaitingDisconnect
    };

    enum class BluetoothControlResult : uint8_t
    {
        Ok, Pending, Disabled, InvalidConfiguration, AlreadyBound, Busy,
        StackUnavailable, StorageError, AuthFailed, AuthTimeout, AuthProtocolError, AuthStorageError, SecurityFailed, SecurityTimeout, SessionTimeout, NotifyFailed, ProtocolError
    };

    enum class BluetoothAccess : uint8_t { Ok, NotAuthorized, InvalidLength, NotReady };

    // BLE-only boundary. Neither capabilities nor the shared transport hub use this port.
    class IBluetoothControlPort
    {
    public:
        virtual ~IBluetoothControlPort() = default;
        virtual bool sessionAllowed() const = 0;
        virtual uint32_t nowMs() const = 0;
        virtual bool randomNonce(uint8_t *nonce, std::size_t size) = 0;
        virtual bool verifyProof(const char *deviceId, const uint8_t *nonce, const uint8_t *proof) = 0;
        virtual BluetoothControlResult authorizePeer(uint32_t authStartedAt) = 0;
        virtual bool validateCommandJson(const char *json, std::size_t size) = 0;
        virtual bool notifyAuth(bool success) = 0;
        virtual bool notifyResponse(const char *json, std::size_t size) = 0;
        virtual void closeSession() = 0;
        virtual void reportFailure(BluetoothControlResult reason) = 0;
    };
}
