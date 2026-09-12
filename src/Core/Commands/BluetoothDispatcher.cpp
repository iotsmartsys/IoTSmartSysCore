#include "BluetoothDispatcher.h"
#if IOTSMARTSYS_BLE_COMMAND_ENABLED
#include "Contracts/Capabilities/Managers/CapabilityManager.h"
#include <cstring>
#include <cstdio>
#include <memory>

namespace iotsmartsys::core
{
    BluetoothDispatcher::BluetoothDispatcher(IBluetoothControlPort &port, ICommandParser &parser,
                                             ITransportDispatcher &common, CapabilityManager &capabilities,
                                             ILogger &logger, const BluetoothControlConfig &config,
                                             const char *deviceId)
        : port_(port), parser_(parser), common_(common), capabilities_(capabilities),
          logger_(logger), config_(config)
    {
        std::strncpy(deviceId_, deviceId, sizeof(deviceId_) - 1);
        config_.sharedSecret = nullptr;
        config_.sharedSecretSize = 0;
    }

    void BluetoothDispatcher::wipe(void *data, std::size_t size)
    {
        auto *p = static_cast<volatile uint8_t *>(data);
        while (size--) *p++ = 0;
    }

    BluetoothDispatcher::~BluetoothDispatcher() { disconnected(); }

    void BluetoothDispatcher::disconnected()
    {
        wipe(nonce_, sizeof(nonce_)); wipe(proof_, sizeof(proof_)); wipe(command_, sizeof(command_));
        proofSize_ = commandSize_ = 0;
        secure_ = authNotify_ = responseNotify_ = challenged_ = authPending_ = false;
        authenticated_ = commandPending_ = commandConsumed_ = closing_ = false;
        state_ = BluetoothControlState::Advertising;
    }

    void BluetoothDispatcher::invalidate()
    {
        authenticated_ = authPending_ = commandPending_ = false;
        wipe(nonce_, sizeof(nonce_)); wipe(proof_, sizeof(proof_)); wipe(command_, sizeof(command_));
        state_ = BluetoothControlState::AwaitingDisconnect;
    }

    void BluetoothDispatcher::connected(uint32_t now)
    {
        disconnected(); connectedAt_ = now; now_ = now;
        state_ = BluetoothControlState::Connected;
    }

    void BluetoothDispatcher::secured()
    {
        secure_ = true; state_ = BluetoothControlState::Securing;
    }

    void BluetoothDispatcher::notifications(bool auth, bool response)
    {
        authNotify_ = auth; responseNotify_ = response;
        if ((!auth || !response) && (challenged_ || authenticated_))
        {
            authenticated_ = false; port_.closeSession();
        }
    }

    BluetoothAccess BluetoothDispatcher::readChallenge(uint32_t now, uint8_t (&out)[17])
    {
        if (!secure_ || !port_.sessionAllowed()) return BluetoothAccess::NotAuthorized;
        if (!authNotify_ || !responseNotify_ || authenticated_ || closing_) return BluetoothAccess::NotReady;
        if (!challenged_)
        {
            if (!port_.randomNonce(nonce_, sizeof(nonce_)))
            {
                failAuth(BluetoothControlResult::AuthFailed, now); return BluetoothAccess::NotReady;
            }
            challenged_ = true; authAt_ = now; state_ = BluetoothControlState::Authenticating;
        }
        if (uint32_t(now - authAt_) >= config_.authTimeoutMs)
        {
            failAuth(BluetoothControlResult::AuthTimeout, now); return BluetoothAccess::NotAuthorized;
        }
        out[0] = 1; std::memcpy(out + 1, nonce_, sizeof(nonce_)); return BluetoothAccess::Ok;
    }

    BluetoothAccess BluetoothDispatcher::writeAuth(const uint8_t *data, std::size_t size, uint32_t now)
    {
        if (!secure_ || !port_.sessionAllowed()) return BluetoothAccess::NotAuthorized;
        if (!challenged_ || authenticated_ || authPending_ || closing_ || !authNotify_ || !responseNotify_)
        {
            failAuth(BluetoothControlResult::AuthProtocolError, now); return BluetoothAccess::NotReady;
        }
        if (!size || size > sizeof(proof_) - proofSize_)
        {
            failAuth(BluetoothControlResult::AuthProtocolError, now); return BluetoothAccess::InvalidLength;
        }
        if (uint32_t(now - authAt_) >= config_.authTimeoutMs)
        {
            failAuth(BluetoothControlResult::AuthTimeout, now); return BluetoothAccess::NotAuthorized;
        }
        std::memcpy(proof_ + proofSize_, data, size); proofSize_ += size;
        authPending_ = proofSize_ == sizeof(proof_); return BluetoothAccess::Ok;
    }

    void BluetoothDispatcher::failAuth(BluetoothControlResult error, uint32_t now)
    {
        authenticated_ = authPending_ = commandPending_ = false;
        wipe(proof_, sizeof(proof_)); wipe(nonce_, sizeof(nonce_)); wipe(command_, sizeof(command_));
        port_.reportFailure(error);
        closing_ = true; closingAt_ = now; state_ = BluetoothControlState::AwaitingDisconnect;
        if (!authNotify_ || !port_.notifyAuth(false)) port_.closeSession();
    }

    BluetoothAccess BluetoothDispatcher::writeCommand(const uint8_t *data, std::size_t size, uint32_t now)
    {
        if (!authenticated_ || !secure_ || !port_.sessionAllowed() || !responseNotify_)
            return BluetoothAccess::NotAuthorized;
        if (closing_ || commandConsumed_ || commandPending_) return BluetoothAccess::NotReady;
        if (!size) return BluetoothAccess::InvalidLength;
        if (commandSize_ == 0) receiveAt_ = now;
        if (uint32_t(now - receiveAt_) >= config_.receiveTimeoutMs)
        {
            response("RECEIVE_TIMEOUT", now); return BluetoothAccess::NotReady;
        }
        // A complete line must be the only line and end of this write.
        const auto *lf = static_cast<const uint8_t *>(std::memchr(data, '\n', size));
        if (lf && lf != data + size - 1)
        {
            response("INVALID_COMMAND", now); return BluetoothAccess::InvalidLength;
        }
        const auto bytes = size - (lf ? 1 : 0);
        if (bytes > 1024 - commandSize_)
        {
            response("PAYLOAD_TOO_LARGE", now); return BluetoothAccess::InvalidLength;
        }
        if (std::memchr(data, 0, bytes))
        {
            response("INVALID_COMMAND", now); return BluetoothAccess::InvalidLength;
        }
        std::memcpy(command_ + commandSize_, data, bytes); commandSize_ += bytes;
        command_[commandSize_] = 0; state_ = BluetoothControlState::Receiving;
        if (lf) commandPending_ = true;
        return BluetoothAccess::Ok;
    }

    void BluetoothDispatcher::response(const char *error, uint32_t now)
    {
        char result[128];
        const auto n = error ? std::snprintf(result, sizeof(result), "{\"ack\":false,\"error\":\"%s\"}\n", error)
                             : std::snprintf(result, sizeof(result), "{\"ack\":true}\n");
        commandConsumed_ = true; commandPending_ = false;
        closing_ = true; closingAt_ = port_.nowMs(); state_ = BluetoothControlState::Responding;
        if (error) logger_.warn("BLE", "%s", error);
        if (n <= 0 || !responseNotify_ || !port_.notifyResponse(result, static_cast<std::size_t>(n)))
            port_.closeSession();
        wipe(command_, sizeof(command_));
    }

    bool BluetoothDispatcher::dispatchMessage(const TransportMessageView &msg)
    {
        if (!authenticated_ || !port_.sessionAllowed() || msg.kind != TransportKind::Command ||
            !msg.origin || std::strcmp(msg.origin, "ble") != 0) return false;
        if (!port_.validateCommandJson(msg.payload, msg.payloadLen))
        { response("INVALID_COMMAND", now_); return false; }
        std::unique_ptr<DeviceCommand> cmd(parser_.parseCommand(msg.payload, msg.payloadLen, false));
        if (!cmd) { response("INVALID_COMMAND", now_); return false; }
        if (cmd->device_id != deviceId_) { response("DEVICE_MISMATCH", now_); return false; }
        if (cmd->getCommandType() != CommandTypes::CAPABILITY || cmd->getSystemCommand() != SystemCommands::UNKNOWN)
        { response("UNSUPPORTED_COMMAND", now_); return false; }
        if (!capabilities_.getCommandCapabilityByName(cmd->capability_name.c_str()))
        { response("CAPABILITY_NOT_FOUND", now_); return false; }
        if (!port_.sessionAllowed()) return false;
        state_ = BluetoothControlState::Dispatching;
        const bool accepted = common_.dispatchMessage(msg);
        response(accepted ? nullptr : "DISPATCH_FAILED", now_);
        return accepted;
    }

    void BluetoothDispatcher::handle(uint32_t now)
    {
        now_ = now;
        if (state_ == BluetoothControlState::Advertising || state_ == BluetoothControlState::Disabled) return;
        if (!port_.sessionAllowed()) { authenticated_ = false; port_.closeSession(); return; }
        if (closing_)
        {
            if (uint32_t(now - closingAt_) >= config_.disconnectTimeoutMs) port_.closeSession();
            return;
        }
        if (uint32_t(now - connectedAt_) >= config_.sessionTimeoutMs ||
            (!secure_ && uint32_t(now - connectedAt_) >= config_.securityTimeoutMs))
        {
            port_.reportFailure(!secure_ ? BluetoothControlResult::SecurityTimeout : BluetoothControlResult::SessionTimeout);
            port_.closeSession(); return;
        }
        if (challenged_ && !authenticated_ && uint32_t(now - authAt_) >= config_.authTimeoutMs)
        { failAuth(BluetoothControlResult::AuthTimeout, now); return; }
        if (authPending_)
        {
            authPending_ = false;
            if (!port_.verifyProof(deviceId_, nonce_, proof_))
            { failAuth(BluetoothControlResult::AuthFailed, now); return; }
            if (uint32_t(port_.nowMs() - authAt_) >= config_.authTimeoutMs)
            { failAuth(BluetoothControlResult::AuthTimeout, port_.nowMs()); return; }
            const auto authorization = port_.authorizePeer(authAt_);
            if (authorization != BluetoothControlResult::Ok)
            { failAuth(authorization, port_.nowMs()); return; }
            wipe(proof_, sizeof(proof_)); wipe(nonce_, sizeof(nonce_));
            authenticated_ = true; state_ = BluetoothControlState::Authorized;
            if (!port_.notifyAuth(true)) { authenticated_ = false; port_.closeSession(); }
        }
        if (commandPending_ && authenticated_)
        {
            commandPending_ = false; commandConsumed_ = true;
            char topic[48]; std::snprintf(topic, sizeof(topic), "device/%s/command", deviceId_);
            TransportMessageView msg{}; msg.topic = topic; msg.payload = command_; msg.payloadLen = commandSize_;
            msg.kind = TransportKind::Command; msg.origin = "ble";
            dispatchMessage(msg);
        }
        else if (commandSize_ && !commandConsumed_ && uint32_t(now - receiveAt_) >= config_.receiveTimeoutMs)
            response("RECEIVE_TIMEOUT", now);
    }
}
#endif
