#pragma once
#include "Config/BuildConfig.h"
#if IOTSMARTSYS_BLE_COMMAND_ENABLED
#include "sdkconfig.h"
#if !defined(CONFIG_BT_BLUEDROID_ENABLED) || !defined(CONFIG_BT_BLE_ENABLED)
#error "BLE Control requires an Arduino ESP32 target with Bluedroid BLE enabled"
#endif
#include <atomic>
#include <memory>
#include "Contracts/Transports/ITransportChannel.h"
#include "Core/Commands/BluetoothDispatcher.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "nvs.h"

namespace iotsmartsys::platform::arduino
{
    // Incoming control channel; deliberately not registered as a Raw hub channel.
    class BluetoothTransportChannel final : public core::ITransportChannel,
                                            private core::IBluetoothControlPort
    {
    public:
        BluetoothTransportChannel(core::ILogger &logger, core::ICommandParser &parser,
                                  core::ITransportDispatcher &common, core::CapabilityManager &capabilities,
                                  const core::BluetoothControlConfig &config);
        ~BluetoothTransportChannel() override;
        bool begin(const core::TransportConfig &config) override;
        void start() override;
        void stop() override;
        void handle() override;
        bool isConnected() const override { return linkAlive_.load(); }
        bool publish(const char *, const void *, std::size_t, bool) override { return false; }
        bool republish(const core::TransportMessageView &) override { return false; }
        bool subscribe(const char *) override { return false; }
        const char *getName() const override { return "ble"; }
        void setOnMessage(core::TransportOnMessageFn, void *) override {}
        void setOnConnected(core::TransportOnConnectedFn cb, void *user) override { onConnected_ = cb; connectedUser_ = user; }
        void setOnDisconnected(core::TransportOnDisconnectedFn cb, void *user) override { onDisconnected_ = cb; disconnectedUser_ = user; }
        core::BluetoothControlResult openPairingWindow();
        core::BluetoothControlResult revokeBleBonds();
        core::BluetoothControlResult result() const { return result_.load(); }
        core::BluetoothControlState state() const;
        bool pairingWindowOpen() const { return windowVisible_.load(); }
        bool hasAuthorizedPeer() const { return boundVisible_.load(); }

    private:
        enum class EventKind : uint8_t { Registered, Table, ServiceStarted, AdvReady, ScanReady,
            AdvStarted, RejectedConnect, Connect, Disconnect, Mtu, Read, Write, SecurityRequest,
            SecurityComplete, Identity, Removed, Congestion, Failure };
        struct Event
        {
            EventKind kind{};
            uint32_t generation{0}, transaction{0};
            uint16_t connection{0}, handle{0}, offset{0}, size{0};
            uint8_t address[6]{}, identity[6]{}, addressType{0}, authMode{0};
            int status{0};
            bool needResponse{false}, prepared{false};
            esp_gatt_if_t interfaceId{ESP_GATT_IF_NONE};
            uint8_t data[514]{};
        };
        struct PeerRecord
        {
            uint8_t state{0}; // 0 empty, 1 candidate, 2 authorized, 3 revoking
            uint8_t addressType{0};
            uint8_t identity[6]{};
            uint8_t bondAddress[6]{};
        };
        static void gapCallback(esp_gap_ble_cb_event_t, esp_ble_gap_cb_param_t *);
        static void gattCallback(esp_gatts_cb_event_t, esp_gatt_if_t, esp_ble_gatts_cb_param_t *);
        static BluetoothTransportChannel *instance_;
        static portMUX_TYPE callbackMux_;
        void enqueue(Event &event);
        void process(const Event &event);
        void configureGatt();
        void advertise();
        void access(const Event &event);
        void answer(const Event &event, core::BluetoothAccess status, const uint8_t *data = nullptr, std::size_t size = 0);
        void fail(core::BluetoothControlResult result);
        bool loadRecord();
        bool saveRecord(const PeerRecord &record);
        bool resolvePeer(const uint8_t *address, PeerRecord &peer);
        bool hasBond(const uint8_t *address);
        bool recordMatches(const PeerRecord &peer) const;
        void beginRemoval();
        void finishRemoval();
        bool windowOpen() const;
        bool sessionAllowed() const override;
        uint32_t nowMs() const override;
        bool randomNonce(uint8_t *, std::size_t) override;
        bool verifyProof(const char *, const uint8_t *, const uint8_t *) override;
        core::BluetoothControlResult authorizePeer(uint32_t authStartedAt) override;
        bool validateCommandJson(const char *, std::size_t) override;
        bool notifyAuth(bool) override;
        bool notifyResponse(const char *, std::size_t) override;
        void closeSession() override;
        void reportFailure(core::BluetoothControlResult) override;
        void transmit();
        core::ILogger &logger_;
        core::ICommandParser &parser_;
        core::ITransportDispatcher &common_;
        core::CapabilityManager &capabilities_;
        core::BluetoothControlConfig config_;
        uint8_t secret_[32]{};
        char deviceId_[14]{};
        bool configured_{false}, stackOwned_{false}, tableReady_{false}, serviceReady_{false};
        bool advReady_{false}, scanReady_{false}, advertising_{false}, removing_{false};
        std::atomic<bool> storageFailed_{false};
        bool candidate_{false}, secure_{false}, authNotify_{false}, responseNotify_{false}, congested_{false};
        std::atomic<bool> running_{false}, blocked_{false}, linkAlive_{false}, fatalQueue_{false};
        std::atomic<bool> openRequested_{false}, revokeRequested_{false}, stopRequested_{false}, startRequested_{false}, stopped_{false};
        std::atomic<bool> windowVisible_{false}, boundVisible_{false};
        std::atomic<uint32_t> callbackGeneration_{0};
        std::atomic<uint16_t> callbackConnection_{0xffff};
        std::atomic<core::BluetoothControlResult> result_{core::BluetoothControlResult::Disabled};
        uint32_t generation_{0}, windowAt_{0}, removalAt_{0}, startAt_{0}, closeAt_{0};
        bool closingLink_{false};
        uint16_t connection_{0xffff}, mtu_{23}, handles_[13]{};
        uint8_t remote_[6]{};
        esp_gatt_if_t interface_{ESP_GATT_IF_NONE};
        nvs_handle_t nvs_{0};
        PeerRecord record_{};
        QueueHandle_t events_{nullptr};
        std::unique_ptr<core::BluetoothDispatcher> dispatcher_;
        char response_[129]{};
        std::size_t responseSize_{0}, responseOffset_{0};
        core::TransportOnConnectedFn onConnected_{nullptr}; void *connectedUser_{nullptr};
        core::TransportOnDisconnectedFn onDisconnected_{nullptr}; void *disconnectedUser_{nullptr};
    };
}
#endif
