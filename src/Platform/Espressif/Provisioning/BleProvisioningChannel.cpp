#include "BleProvisioningChannel.h"

#include <Arduino.h>
#include <cstring>

extern "C"
{
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
}

namespace iotsmartsys::core::provisioning
{

    BleProvisioningChannel *BleProvisioningChannel::s_instance = nullptr;

    static const char *TAG = "BleProv";

    static constexpr uint16_t WIFI_SERVICE_UUID = 0xABCD;
    static constexpr uint16_t WIFI_CONFIG_CHAR_UUID = 0xABCE;
    static constexpr uint16_t WIFI_STATUS_CHAR_UUID = 0xABCF;

    static constexpr uint16_t GATTS_APP_ID = 0x55;

    enum
    {
        IDX_SVC,
        IDX_CHAR_CONFIG,
        IDX_CHAR_VAL_CONFIG,
        IDX_CHAR_STATUS,
        IDX_CHAR_VAL_STATUS,
        IDX_CHAR_CFG_STATUS,
        HRS_IDX_NB
    };

    static uint16_t s_handleTable[HRS_IDX_NB] = {0};

    static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
    static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
    static const uint16_t client_characteristic_cfg_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

    static const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;
    static const uint8_t char_prop_notify_read = ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_READ;

    static const uint8_t status_init_val[] = "WAITING_CONFIG";
    static uint16_t s_ccc_default = 0x0000;

    static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] = {
        [IDX_SVC] = {
            {ESP_GATT_AUTO_RSP},
            {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(uint16_t), (uint8_t *)&WIFI_SERVICE_UUID}},
        [IDX_CHAR_CONFIG] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read_write}},
        [IDX_CHAR_VAL_CONFIG] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&WIFI_CONFIG_CHAR_UUID, ESP_GATT_PERM_WRITE, 96, 0, nullptr}},
        [IDX_CHAR_STATUS] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_notify_read}},
        [IDX_CHAR_VAL_STATUS] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&WIFI_STATUS_CHAR_UUID, ESP_GATT_PERM_READ, sizeof(status_init_val), sizeof(status_init_val), (uint8_t *)status_init_val}},
        [IDX_CHAR_CFG_STATUS] = {{ESP_GATT_AUTO_RSP},{ESP_UUID_LEN_16, (uint8_t*)&client_characteristic_cfg_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,sizeof(uint16_t), sizeof(uint16_t), (uint8_t*)&s_ccc_default},
},
    };

    static esp_ble_adv_params_t s_advParams = {
        .adv_int_min = 0x20,
        .adv_int_max = 0x40,
        .adv_type = ADV_TYPE_IND,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY};

    static void build_ble_device_name(char *out, size_t out_len)
    {
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);

        const char *model = "ESP";
        switch (chip_info.model)
        {
        case CHIP_ESP32:
            model = "ESP32";
            break;
        case CHIP_ESP32S2:
            model = "ESP32S2";
            break;
        case CHIP_ESP32S3:
            model = "ESP32S3";
            break;
        case CHIP_ESP32C3:
            model = "ESP32C3";
            break;
        // case CHIP_ESP32C6:
        //     model = "ESP32C6";
        //     break;
        case CHIP_ESP32H2:
            model = "ESP32H2";
            break;
        default:
            model = "ESP";
            break;
        }

        uint64_t mac = 0;
#if defined(ESP32)
        mac = ESP.getEfuseMac();
#else
        mac = esp_efuse_mac_get_default();
#endif

        uint32_t suffix = (uint32_t)(mac & 0xFFFFFFULL);
        snprintf(out, out_len, "%s-%06lX", model, (unsigned long)suffix);
    }

    BleProvisioningChannel::BleProvisioningChannel() = default;

    void BleProvisioningChannel::begin()
    {
        if (_active)
        {
            return;
        }

        s_instance = this;
        _active = true;
        _notifyEnabled = false;
        _connId = 0xFFFF;
        _gattsIf = ESP_GATT_IF_NONE;

        initBleStack();

        esp_ble_gap_register_callback(&BleProvisioningChannel::gapEventHandler);
        esp_ble_gatts_register_callback(&BleProvisioningChannel::gattsEventHandler);
        esp_ble_gatts_app_register(GATTS_APP_ID);

        sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] Aguardando configuracao via BLE...");
    }

    void BleProvisioningChannel::loop()
    {
    }

    void BleProvisioningChannel::stop()
    {
        if (!_active)
        {
            return;
        }

        esp_ble_gap_stop_advertising();
        _active = false;
        s_instance = nullptr;
        sendStatus(ProvisioningStatus::Idle, "[BLE] Canal BLE parado");
    }

    void BleProvisioningChannel::sendStatus(ProvisioningStatus status, const char *msg)
    {
        if (_statusCb)
        {
            _statusCb(status, msg);
        }

        if (!msg)
        {
            return;
        }

        Serial.println(msg);

        if (_gattsIf == ESP_GATT_IF_NONE)
        {
            return;
        }

        if (s_handleTable[IDX_CHAR_VAL_STATUS] == 0)
        {
            return;
        }

        esp_ble_gatts_set_attr_value(
            s_handleTable[IDX_CHAR_VAL_STATUS],
            (uint16_t)strlen(msg),
            (const uint8_t *)msg);

        if (_connId != 0xFFFF && _notifyEnabled)
        {
            esp_ble_gatts_send_indicate(
                _gattsIf,
                _connId,
                s_handleTable[IDX_CHAR_VAL_STATUS],
                (uint16_t)strlen(msg),
                (uint8_t *)msg,
                false);
        }
    }

    void BleProvisioningChannel::onConfigWrite(const uint8_t *data, uint16_t len)
    {
        if (!data || len == 0 || len > 95)
        {
            sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:INVALID_LENGTH");
            return;
        }

        char payload[96];
        memcpy(payload, data, len);
        payload[len] = '\0';

        sendStatus(ProvisioningStatus::ReceivingData, "[BLE] CONFIG_RECEIVED");
        parseAndStore(payload);

        if (_wifiSsidStorage.empty() || _wifiPasswordStorage.empty())
        {
            sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:INVALID_FORMAT");
            return;
        }

        if (_configCb)
        {
            DeviceConfig cfg;
            cfg.wifi.ssid = _wifiSsidStorage.c_str();
            cfg.wifi.password = _wifiPasswordStorage.c_str();
            cfg.deviceApiKey = _deviceApiKeyStorage.empty() ? nullptr : _deviceApiKeyStorage.c_str();
            cfg.basicAuth = _basicAuthStorage.empty() ? nullptr : _basicAuthStorage.c_str();
            cfg.source = ProvisioningSource::Ble;

            _configCb(cfg);
        }

        sendStatus(ProvisioningStatus::Applied, "[BLE] APPLIED");
    }

    void BleProvisioningChannel::parseAndStore(const char *payload)
    {
        _wifiSsidStorage.clear();
        _wifiPasswordStorage.clear();
        _deviceApiKeyStorage.clear();
        _basicAuthStorage.clear();

        if (!payload)
        {
            return;
        }

        std::string p(payload);

        auto nextPart = [](const std::string &s, size_t &pos) -> std::string
        {
            if (pos == std::string::npos)
            {
                return {};
            }
            size_t start = pos;
            size_t end = s.find('|', start);
            if (end == std::string::npos)
            {
                pos = std::string::npos;
                return s.substr(start);
            }
            pos = end + 1;
            return s.substr(start, end - start);
        };

        size_t pos = 0;
        _wifiSsidStorage = nextPart(p, pos);
        _wifiPasswordStorage = nextPart(p, pos);
        _deviceApiKeyStorage = nextPart(p, pos);
        _basicAuthStorage = nextPart(p, pos);
    }

    void BleProvisioningChannel::initBleStack()
    {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            nvs_flash_erase();
            nvs_flash_init();
        }

        esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        ret = esp_bt_controller_init(&bt_cfg);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
        {
            ESP_LOGE(TAG, "bt_controller_init failed: %s", esp_err_to_name(ret));
        }

        ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
        {
            ESP_LOGE(TAG, "bt_controller_enable failed: %s", esp_err_to_name(ret));
        }

        ret = esp_bluedroid_init();
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
        {
            ESP_LOGE(TAG, "bluedroid_init failed: %s", esp_err_to_name(ret));
        }

        ret = esp_bluedroid_enable();
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
        {
            ESP_LOGE(TAG, "bluedroid_enable failed: %s", esp_err_to_name(ret));
        }
    }

    void BleProvisioningChannel::startAdvertising()
    {
        esp_ble_adv_data_t adv_data = {};
        adv_data.set_scan_rsp = false;
        adv_data.include_name = true;
        adv_data.include_txpower = true;
        adv_data.flag = ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT;

        esp_ble_gap_config_adv_data(&adv_data);
    }

    void BleProvisioningChannel::gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
    {
        if (!s_instance)
        {
            return;
        }

        switch (event)
        {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            esp_ble_gap_start_advertising(&s_advParams);
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
            {
                s_instance->sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:ADV_START_FAILED");
            }
            break;
        default:
            break;
        }
    }

    void BleProvisioningChannel::gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
    {
        if (!s_instance)
        {
            return;
        }

        switch (event)
        {
        case ESP_GATTS_REG_EVT:
        {
            s_instance->_gattsIf = gatts_if;
            char dev_name[32];
            build_ble_device_name(dev_name, sizeof(dev_name));
            esp_ble_gap_set_device_name(dev_name);
            startAdvertising();
            esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HRS_IDX_NB, 0);
            break;
        }

        case ESP_GATTS_CREAT_ATTR_TAB_EVT:
        {
            if (param->add_attr_tab.status == ESP_GATT_OK && param->add_attr_tab.num_handle == HRS_IDX_NB)
            {
                memcpy(s_handleTable, param->add_attr_tab.handles, sizeof(s_handleTable));
                esp_ble_gatts_start_service(s_handleTable[IDX_SVC]);
                s_instance->sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] READY");
            }
            else
            {
                s_instance->sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:ATTR_TAB");
            }
            break;
        }

        case ESP_GATTS_CONNECT_EVT:
        {
            s_instance->_connId = param->connect.conn_id;
            s_instance->sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] CONNECTED");
            break;
        }

        case ESP_GATTS_DISCONNECT_EVT:
        {
            s_instance->_connId = 0xFFFF;
            s_instance->_notifyEnabled = false;
            esp_ble_gap_start_advertising(&s_advParams);
            s_instance->sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] DISCONNECTED");
            break;
        }

        case ESP_GATTS_WRITE_EVT:
        {
            const auto &w = param->write;

            if (w.handle == s_handleTable[IDX_CHAR_VAL_CONFIG])
            {
                s_instance->onConfigWrite(w.value, w.len);
            }
            else if (w.handle == s_handleTable[IDX_CHAR_CFG_STATUS] && w.len == 2)
            {
                uint16_t v = (uint16_t)(w.value[1] << 8) | (uint16_t)(w.value[0]);
                s_instance->_notifyEnabled = (v == 0x0001);
            }
            break;
        }

        default:
            break;
        }
    }

} // namespace iotsmartsys::core::provisioning