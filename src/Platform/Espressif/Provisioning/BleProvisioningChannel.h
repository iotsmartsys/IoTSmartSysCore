#pragma once

#include <string>
#include "Core/Provisioning/IProvisioningChannel.h"

extern "C"
{
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
}

namespace iotsmartsys::core::provisioning
{

    /// @brief Canal de provisionamento via BLE GATT Server (ESP-IDF Bluedroid) integrado ao framework Arduino.
    class BleProvisioningChannel : public IProvisioningChannel
    {
    public:
        /// @brief Cria uma instância do canal BLE.
        BleProvisioningChannel();

        /// @brief Inicializa o stack BLE e inicia advertising.
        void begin() override;

        /// @brief Processamento periódico (não bloqueante).
        void loop() override;

        /// @brief Encerra advertising/conexão BLE (sem desligar o stack).
        void stop() override;

        /// @brief Indica se o canal está ativo.
        bool isActive() const override { return _active; }

        /// @brief Define o callback chamado quando uma nova configuração é recebida.
        void onConfigReceived(ConfigCallback cb) override { _configCb = cb; }

        /// @brief Define o callback chamado quando o status do canal muda.
        void onStatusChanged(StatusCallback cb) override { _statusCb = cb; }

        /// @brief Prioridade do canal BLE.
        uint8_t priority() const override { return 100; }

    private:
        bool _active = false;

        bool _notifyEnabled = false;
        uint16_t _connId = 0xFFFF;
        esp_gatt_if_t _gattsIf = ESP_GATT_IF_NONE;

        ConfigCallback _configCb = nullptr;
        StatusCallback _statusCb = nullptr;

        std::string _wifiSsidStorage;
        std::string _wifiPasswordStorage;
        std::string _deviceApiKeyStorage;
        std::string _basicAuthStorage;

        void sendStatus(ProvisioningStatus status, const char *msg);
        void onConfigWrite(const uint8_t *data, uint16_t len);
        void parseAndStore(const char *payload);

        static void gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
        static void gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

        static BleProvisioningChannel *s_instance;

        static void initBleStack();
        static void startAdvertising();
    };

} // namespace iotsmartsys::core::provisioning