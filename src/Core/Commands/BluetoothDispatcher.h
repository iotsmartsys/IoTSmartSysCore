#pragma once

#include "Config/BuildConfig.h"
#if IOTSMARTSYS_BLE_COMMAND_ENABLED
#include <atomic>
#include "Contracts/Transports/BluetoothControl.h"
#include "Contracts/Transports/ITransportDispatcher.h"
#include "Contracts/Parsers/ICommandParser.h"
#include "Contracts/Logging/ILogger.h"

namespace iotsmartsys::core
{
    class CapabilityManager;

    // Driven only by the existing transport task (or its cooperative fallback).
    class BluetoothDispatcher final : public ITransportDispatcher
    {
    public:
        BluetoothDispatcher(IBluetoothControlPort &port, ICommandParser &parser,
                            ITransportDispatcher &common, CapabilityManager &capabilities,
                            ILogger &logger, const BluetoothControlConfig &config,
                            const char *deviceId);
        ~BluetoothDispatcher();
        void connected(uint32_t now);
        void disconnected();
        void invalidate();
        void secured();
        void notifications(bool auth, bool response);
        BluetoothAccess readChallenge(uint32_t now, uint8_t (&out)[17]);
        BluetoothAccess writeAuth(const uint8_t *data, std::size_t size, uint32_t now);
        BluetoothAccess writeCommand(const uint8_t *data, std::size_t size, uint32_t now);
        void handle(uint32_t now);
        bool dispatchMessage(const TransportMessageView &msg) override;
        BluetoothControlState state() const { return state_.load(); }
        bool authenticated() const { return authenticated_; }

    private:
        void response(const char *error, uint32_t now);
        void failAuth(BluetoothControlResult error, uint32_t now);
        static void wipe(void *data, std::size_t size);
        IBluetoothControlPort &port_;
        ICommandParser &parser_;
        ITransportDispatcher &common_;
        CapabilityManager &capabilities_;
        ILogger &logger_;
        BluetoothControlConfig config_;
        char deviceId_[14]{};
        std::atomic<BluetoothControlState> state_{BluetoothControlState::Disabled};
        uint8_t nonce_[16]{};
        uint8_t proof_[32]{};
        char command_[1025]{};
        std::size_t proofSize_{0}, commandSize_{0};
        uint32_t connectedAt_{0}, authAt_{0}, receiveAt_{0}, closingAt_{0};
        bool secure_{false}, authNotify_{false}, responseNotify_{false};
        bool challenged_{false}, authPending_{false}, authenticated_{false};
        bool commandPending_{false}, commandConsumed_{false}, closing_{false};
        uint32_t now_{0};
    };
}
#endif
