#include "BleProvisioningChannel.h"

#include <cstring>
#include <string>
#include <vector>

extern "C"
{
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp32-hal-bt.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "soc/soc_caps.h"
}

namespace iotsmartsys::core::provisioning
{

    BleProvisioningChannel *BleProvisioningChannel::s_instance = nullptr;

    static const char *TAG = "BleProv";

    // 128-bit UUIDs (BLE uses little-endian byte order for UUIDs)
    // Service UUID: 581DD1C9-0D5F-4174-921E-65AE482C6DE6
    static const uint8_t WIFI_SERVICE_UUID128[16] = {
        0xE6, 0x6D, 0x2C, 0x48, 0xAE, 0x65, 0x1E, 0x92,
        0x74, 0x41, 0x5F, 0x0D, 0xC9, 0xD1, 0x1D, 0x58};

    // Config Characteristic UUID: 581DD1C9-0D5F-4174-921E-65AE482C6DE7
    static const uint8_t WIFI_CONFIG_CHAR_UUID128[16] = {
        0xE7, 0x6D, 0x2C, 0x48, 0xAE, 0x65, 0x1E, 0x92,
        0x74, 0x41, 0x5F, 0x0D, 0xC9, 0xD1, 0x1D, 0x58};

    // Status Characteristic UUID: 581DD1C9-0D5F-4174-921E-65AE482C6DE8
    static const uint8_t WIFI_STATUS_CHAR_UUID128[16] = {
        0xE8, 0x6D, 0x2C, 0x48, 0xAE, 0x65, 0x1E, 0x92,
        0x74, 0x41, 0x5F, 0x0D, 0xC9, 0xD1, 0x1D, 0x58};

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

    static uint8_t s_status_value[96] = "WAITING_CONFIG";
    static uint16_t s_ccc_default = 0x0000;

    static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] = {
        [IDX_SVC] = {
            {ESP_GATT_AUTO_RSP},
            {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, ESP_UUID_LEN_128, ESP_UUID_LEN_128, (uint8_t *)WIFI_SERVICE_UUID128}},
        [IDX_CHAR_CONFIG] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read_write}},
        [IDX_CHAR_VAL_CONFIG] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_128, (uint8_t *)WIFI_CONFIG_CHAR_UUID128, ESP_GATT_PERM_WRITE, 96, 0, nullptr}},
        [IDX_CHAR_STATUS] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_notify_read}},
        [IDX_CHAR_VAL_STATUS] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_128, (uint8_t *)WIFI_STATUS_CHAR_UUID128, ESP_GATT_PERM_READ, sizeof(s_status_value), (uint16_t)strlen((const char *)s_status_value), (uint8_t *)s_status_value}},
        [IDX_CHAR_CFG_STATUS] = {
            {ESP_GATT_AUTO_RSP},
            {ESP_UUID_LEN_16, (uint8_t *)&client_characteristic_cfg_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(uint16_t), sizeof(uint16_t), (uint8_t *)&s_ccc_default},
        },
    };

    static esp_ble_adv_params_t s_advParams = {
        .adv_int_min = 0x20,
        .adv_int_max = 0x40,
        .adv_type = ADV_TYPE_IND,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY};

    static bool s_adv_cfg_done = false;
    static bool s_scan_rsp_cfg_done = false;
    static char s_adv_name[32] = "IoTSmartSysSetup";
    static bool s_pending_adv_start_log = true;

    static std::string s_rx_buffer;
    static size_t s_rx_expected_len = 0;
    static bool s_rx_in_progress = false;

    // Wi‑Fi SSID list TX state (sent to the app via the status characteristic)
    static bool s_send_ssids_requested = false;
    static bool s_sending_ssids = false;
    static std::vector<std::string> s_pending_ssids;
    static size_t s_pending_ssid_index = 0;

    static void build_ble_device_name(char *out, size_t out_len)
    {
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);

        const char *model = "ESP";
        switch (chip_info.model)
        {
        case CHIP_ESP32:
            model = "esp32";
            break;
        case CHIP_ESP32S2:
            model = "esp32s2";
            break;
        case CHIP_ESP32S3:
            model = "esp32s3";
            break;
        case CHIP_ESP32C3:
            model = "esp32c3";
            break;
        // case CHIP_ESP32C6:
        //     model = "ESP32C6";
        //     break;
        case CHIP_ESP32H2:
            model = "esp32h2";
            break;
        default:
            model = "ESP";
            break;
        }

        uint8_t mac[6] = {0};
        esp_efuse_mac_get_default(mac);

        uint32_t suffix = ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | mac[5];
        snprintf(out, out_len, "%s-%06lX", model, (unsigned long)suffix);
    }

    static bool starts_with(const char *s, const char *prefix)
    {
        if (!s || !prefix)
        {
            return false;
        }

        while (*prefix)
        {
            if (*s++ != *prefix++)
            {
                return false;
            }
        }
        return true;
    }

    BleProvisioningChannel::BleProvisioningChannel(core::ILogger &logger, core::WiFiManager &wifiManager)
        : _logger(logger), _wifiManager(wifiManager)
    {
    }

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

        if (!initBleStack())
        {
            _active = false;
            s_instance = nullptr;
            sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:INIT_BLE");
            return;
        }

        esp_err_t rc_gap_cb = esp_ble_gap_register_callback(&BleProvisioningChannel::gapEventHandler);
        _logger.info(TAG, "begin: esp_ble_gap_register_callback -> %s", esp_err_to_name(rc_gap_cb));

        esp_err_t rc_gatts_cb = esp_ble_gatts_register_callback(&BleProvisioningChannel::gattsEventHandler);
        _logger.info(TAG, "begin: esp_ble_gatts_register_callback -> %s", esp_err_to_name(rc_gatts_cb));

        esp_err_t rc_app = esp_ble_gatts_app_register(GATTS_APP_ID);
        _logger.info(TAG, "begin: esp_ble_gatts_app_register -> %s", esp_err_to_name(rc_app));

        sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] Aguardando configuracao via BLE...");
    }

    void BleProvisioningChannel::loop()
    {
        if (!_active)
        {
            return;
        }

        // Start sending the SSID list only after the app connects AND enables notifications.
        if (s_send_ssids_requested && _connId != 0xFFFF && _notifyEnabled)
        {
            s_send_ssids_requested = false;

            // Uses WifiManager::getAvailableSSIDs() (per your requirement)
            s_pending_ssids = _wifiManager.getAvailableSSIDs();
            s_pending_ssid_index = 0;
            s_sending_ssids = true;

            char msg[96];
            snprintf(msg, sizeof(msg), "[BLE] WIFI_LIST_BEGIN|%u", (unsigned)s_pending_ssids.size());
            sendStatus(ProvisioningStatus::WaitingUserInput, msg);
        }

        // Send one SSID per loop tick to avoid spamming BLE in a single callback.
        if (s_sending_ssids && _connId != 0xFFFF && _notifyEnabled)
        {
            if (s_pending_ssid_index < s_pending_ssids.size())
            {
                std::string ssid = s_pending_ssids[s_pending_ssid_index];

                // Sanitize delimiters/newlines so the app parser doesn't break.
                for (char &c : ssid)
                {
                    if (c == '|')
                        c = '/';
                    if (c == '\n' || c == '\r')
                        c = ' ';
                }

                char msg[96];
                snprintf(msg, sizeof(msg), "[BLE] WIFI_SSID|%u|%s", (unsigned)s_pending_ssid_index, ssid.c_str());
                sendStatus(ProvisioningStatus::WaitingUserInput, msg);

                s_pending_ssid_index++;
            }
            else
            {
                sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] WIFI_LIST_END");
                s_pending_ssids.clear();
                s_pending_ssid_index = 0;
                s_sending_ssids = false;
            }
        }
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

        _logger.info(TAG, "%s", msg);

        if (_gattsIf == ESP_GATT_IF_NONE)
        {
            return;
        }

        if (s_handleTable[IDX_CHAR_VAL_STATUS] == 0)
        {
            return;
        }

        uint16_t msg_len = (uint16_t)strlen(msg);
        const uint16_t max_len = (uint16_t)sizeof(s_status_value);
        if (msg_len > max_len)
        {
            msg_len = max_len;
        }

        esp_ble_gatts_set_attr_value(
            s_handleTable[IDX_CHAR_VAL_STATUS],
            msg_len,
            (const uint8_t *)msg);

        if (_connId != 0xFFFF && _notifyEnabled)
        {
            esp_ble_gatts_send_indicate(
                _gattsIf,
                _connId,
                s_handleTable[IDX_CHAR_VAL_STATUS],
                msg_len,
                (uint8_t *)msg,
                false);
        }
    }

    void BleProvisioningChannel::onConfigWrite(const uint8_t *data, uint16_t len)
    {
        _logger.debug(TAG, "onConfigWrite: len=%u data=%p", (unsigned)len, (const void *)data);
        if (!data || len == 0 || len > 96)
        {
            _logger.debug(TAG, "onConfigWrite: INVALID_LENGTH (len=%u)", (unsigned)len);
            sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:INVALID_LENGTH");
            return;
        }

        char payload[97];
        memcpy(payload, data, len);
        payload[len] = '\0';
        _logger.debug(TAG, "onConfigWrite: payload='%s' (len=%u)", payload, (unsigned)len);

        sendStatus(ProvisioningStatus::ReceivingData, "[BLE] CONFIG_RECEIVED");

        if (starts_with(payload, "BEGIN|"))
        {
            s_rx_buffer.clear();
            s_rx_in_progress = true;
            s_rx_expected_len = (size_t)strtoul(payload + 6, nullptr, 10);
            sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] RX_BEGIN");
            return;
        }

        if (starts_with(payload, "DATA|"))
        {
            if (!s_rx_in_progress)
            {
                sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:RX_NOT_STARTED");
                return;
            }

            const char *p = payload + 5;
            const char *sep = strchr(p, '|');
            if (!sep)
            {
                sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:RX_BAD_DATA");
                return;
            }

            const char *chunk = sep + 1;
            s_rx_buffer.append(chunk);

            if (s_rx_expected_len > 0 && s_rx_buffer.size() > s_rx_expected_len)
            {
                s_rx_in_progress = false;
                s_rx_buffer.clear();
                s_rx_expected_len = 0;
                sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:RX_TOO_LARGE");
                return;
            }

            sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] RX_DATA");
            return;
        }

        if (strcmp(payload, "END") == 0)
        {
            if (!s_rx_in_progress)
            {
                sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:RX_NOT_STARTED");
                return;
            }

            if (s_rx_expected_len > 0 && s_rx_buffer.size() != s_rx_expected_len)
            {
                s_rx_in_progress = false;
                s_rx_buffer.clear();
                s_rx_expected_len = 0;
                sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:RX_LEN_MISMATCH");
                return;
            }

            parseAndStore(s_rx_buffer.c_str());

            s_rx_in_progress = false;
            s_rx_buffer.clear();
            s_rx_expected_len = 0;
        }
        else
        {
            parseAndStore(payload);
        }

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
            cfg.deviceApiUrl = _deviceApiUrlStorage.empty() ? nullptr : _deviceApiUrlStorage.c_str();
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
        _deviceApiUrlStorage.clear();
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
        _deviceApiUrlStorage = nextPart(p, pos);
        _deviceApiKeyStorage = nextPart(p, pos);
        _basicAuthStorage = nextPart(p, pos);
    }

    bool BleProvisioningChannel::initBleStack()
    {
        auto *logger = s_instance ? &s_instance->_logger : nullptr;

        esp_err_t ret = nvs_flash_init();
        if (logger)
        {
            logger->info(TAG, "initBleStack: nvs_flash_init -> %s", esp_err_to_name(ret));
        }
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            nvs_flash_erase();
            nvs_flash_init();
        }

    #if SOC_BT_CLASSIC_SUPPORTED
        // Free Classic BT memory when only using BLE.
        esp_err_t rel = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
        if (logger)
        {
            logger->info(TAG, "initBleStack: mem_release(CLASSIC_BT) -> %s", esp_err_to_name(rel));
        }
    #endif

        esp_bt_controller_status_t ctl_status = esp_bt_controller_get_status();
        if (logger)
        {
            logger->info(TAG, "initBleStack: controller_status=%d", (int)ctl_status);
        }

        if (ctl_status == ESP_BT_CONTROLLER_STATUS_ENABLED)
        {
            if (logger)
            {
                logger->info(TAG, "initBleStack: controller enabled, disabling for reinit");
            }
            esp_bt_controller_disable();
            ctl_status = esp_bt_controller_get_status();
        }

        if (ctl_status == ESP_BT_CONTROLLER_STATUS_INITED)
        {
            if (logger)
            {
                logger->info(TAG, "initBleStack: controller inited, deinit to reset");
            }
            esp_bt_controller_deinit();
            ctl_status = esp_bt_controller_get_status();
        }

        if (ctl_status == ESP_BT_CONTROLLER_STATUS_IDLE)
        {
            esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
            ret = esp_bt_controller_init(&bt_cfg);
            if (logger)
            {
                logger->info(TAG, "initBleStack: bt_controller_init -> %s", esp_err_to_name(ret));
            }
            if (ret == ESP_ERR_INVALID_STATE)
            {
                // Try full release/deinit and retry once
                esp_bt_controller_disable();
                esp_bt_controller_deinit();
                esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
                ret = esp_bt_controller_init(&bt_cfg);
                if (logger)
                {
                    logger->info(TAG, "initBleStack: bt_controller_init retry -> %s", esp_err_to_name(ret));
                }
            }
            if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
            {
                if (logger)
                {
                    logger->error(TAG, "bt_controller_init failed: %s", esp_err_to_name(ret));
                }
                // Continue even if invalid state; do not return
            }
        }

        ctl_status = esp_bt_controller_get_status();
        if (logger)
        {
            logger->info(TAG, "initBleStack: controller_status(after init)=%d", (int)ctl_status);
        }

        if (ctl_status != ESP_BT_CONTROLLER_STATUS_ENABLED)
        {
            ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
            if (logger)
            {
                logger->info(TAG, "initBleStack: bt_controller_enable(BLE) -> %s", esp_err_to_name(ret));
            }
            if (ret == ESP_ERR_INVALID_ARG)
            {
                ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
                if (logger)
                {
                    logger->info(TAG, "initBleStack: bt_controller_enable(BTDM fallback) -> %s", esp_err_to_name(ret));
                }
            }
            if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
            {
                if (logger)
                {
                    logger->error(TAG, "bt_controller_enable failed: %s", esp_err_to_name(ret));
                }
                return false;
            }
        }
        else if (logger)
        {
            logger->info(TAG, "initBleStack: controller already enabled");
        }

        esp_bluedroid_status_t bd_status = esp_bluedroid_get_status();
        if (logger)
        {
            logger->info(TAG, "initBleStack: bluedroid_status=%d", (int)bd_status);
        }

        if (bd_status == ESP_BLUEDROID_STATUS_ENABLED)
        {
            if (logger)
            {
                logger->info(TAG, "initBleStack: bluedroid enabled, disabling for reinit");
            }
            esp_bluedroid_disable();
            bd_status = esp_bluedroid_get_status();
        }

        if (bd_status == ESP_BLUEDROID_STATUS_INITIALIZED)
        {
            if (logger)
            {
                logger->info(TAG, "initBleStack: bluedroid initialized, deinit to reset");
            }
            esp_bluedroid_deinit();
            bd_status = esp_bluedroid_get_status();
        }

        if (bd_status == ESP_BLUEDROID_STATUS_UNINITIALIZED)
        {
            ret = esp_bluedroid_init();
            if (logger)
            {
                logger->info(TAG, "initBleStack: bluedroid_init -> %s", esp_err_to_name(ret));
            }
            if (ret == ESP_ERR_INVALID_STATE)
            {
                esp_bluedroid_disable();
                esp_bluedroid_deinit();
                ret = esp_bluedroid_init();
                if (logger)
                {
                    logger->info(TAG, "initBleStack: bluedroid_init retry -> %s", esp_err_to_name(ret));
                }
            }
            if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
            {
                if (logger)
                {
                    logger->error(TAG, "bluedroid_init failed: %s", esp_err_to_name(ret));
                }
                return false;
            }
            bd_status = esp_bluedroid_get_status();
        }

        if (bd_status != ESP_BLUEDROID_STATUS_ENABLED)
        {
            ret = esp_bluedroid_enable();
            if (logger)
            {
                logger->info(TAG, "initBleStack: bluedroid_enable -> %s", esp_err_to_name(ret));
            }
            if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
            {
                if (logger)
                {
                    logger->error(TAG, "bluedroid_enable failed: %s", esp_err_to_name(ret));
                }
                return false;
            }
        }
        else if (logger)
        {
            logger->info(TAG, "initBleStack: bluedroid already enabled");
        }

        ret = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
        if (ret != ESP_OK)
        {
            if (logger)
            {
                logger->warn(TAG, "tx_power_set(ADV) failed: %s", esp_err_to_name(ret));
            }
        }

        ret = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
        if (ret != ESP_OK)
        {
            if (logger)
            {
                logger->warn(TAG, "tx_power_set(DEFAULT) failed: %s", esp_err_to_name(ret));
            }
        }

        esp_bt_controller_status_t final_ctl = esp_bt_controller_get_status();
        esp_bluedroid_status_t final_bd = esp_bluedroid_get_status();
        if (logger)
        {
            logger->info(TAG, "initBleStack: final controller_status=%d bluedroid_status=%d", (int)final_ctl, (int)final_bd);
        }

        if (final_ctl != ESP_BT_CONTROLLER_STATUS_ENABLED || final_bd != ESP_BLUEDROID_STATUS_ENABLED)
        {
            if (logger)
            {
                logger->warn(TAG, "initBleStack: BLE stack not fully enabled, trying Arduino btStart()");
            }
            if (btStarted())
            {
                btStop();
            }
            bool bt_ok = btStart();
            final_ctl = esp_bt_controller_get_status();
            final_bd = esp_bluedroid_get_status();
            if (final_bd == ESP_BLUEDROID_STATUS_UNINITIALIZED)
            {
                ret = esp_bluedroid_init();
                if (logger)
                {
                    logger->info(TAG, "initBleStack: bluedroid_init after btStart -> %s", esp_err_to_name(ret));
                }
                if (ret == ESP_OK || ret == ESP_ERR_INVALID_STATE)
                {
                    ret = esp_bluedroid_enable();
                    if (logger)
                    {
                        logger->info(TAG, "initBleStack: bluedroid_enable after btStart -> %s", esp_err_to_name(ret));
                    }
                }
                final_bd = esp_bluedroid_get_status();
            }
            if (logger)
            {
                logger->info(TAG, "initBleStack: btStart result=%d controller_status=%d bluedroid_status=%d", bt_ok ? 1 : 0, (int)final_ctl, (int)final_bd);
            }
        }

        const bool ready = (final_ctl == ESP_BT_CONTROLLER_STATUS_ENABLED) && (final_bd == ESP_BLUEDROID_STATUS_ENABLED);
        if (!ready && logger)
        {
            logger->error(TAG, "initBleStack: BLE stack not ready");
        }

        return ready;
    }

    void BleProvisioningChannel::startAdvertising()
    {
        s_adv_cfg_done = false;
        s_scan_rsp_cfg_done = false;

        const uint8_t adv_raw[] = {
            0x02, 0x01, 0x06,
            0x11, 0x07,
            0xE6, 0x6D, 0x2C, 0x48, 0xAE, 0x65, 0x1E, 0x92,
            0x74, 0x41, 0x5F, 0x0D, 0xC9, 0xD1, 0x1D, 0x58};

        esp_err_t err1 = esp_ble_gap_config_adv_data_raw((uint8_t *)adv_raw, sizeof(adv_raw));
        if (err1 == ESP_OK)
        {
            // OK: vai vir o evento ADV_DATA_RAW_SET_COMPLETE
        }
        else if (err1 == ESP_ERR_INVALID_STATE)
        {
            ESP_LOGW(TAG, "ADV raw already configured (INVALID_STATE). Marking done.");
            s_adv_cfg_done = true;
        }
        else
        {
            ESP_LOGE(TAG, "esp_ble_gap_config_adv_data_raw failed: %s", esp_err_to_name(err1));
            if (s_instance)
                s_instance->sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:ADV_DATA_CFG");
        }

        uint8_t scan_rsp_raw[2 + sizeof(s_adv_name)] = {0};
        size_t name_len = strnlen(s_adv_name, sizeof(s_adv_name) - 1);
        if (name_len > 29)
            name_len = 29;

        scan_rsp_raw[0] = (uint8_t)(name_len + 1);
        scan_rsp_raw[1] = 0x09;
        memcpy(&scan_rsp_raw[2], s_adv_name, name_len);

        esp_err_t err2 = esp_ble_gap_config_scan_rsp_data_raw(scan_rsp_raw, (uint16_t)(name_len + 2));
        if (err2 == ESP_OK)
        {
            // OK: vai vir o evento SCAN_RSP_DATA_RAW_SET_COMPLETE
        }
        else if (err2 == ESP_ERR_INVALID_STATE)
        {
            ESP_LOGW(TAG, "SCAN_RSP raw already configured (INVALID_STATE). Marking done.");
            s_scan_rsp_cfg_done = true;
        }
        else
        {
            ESP_LOGE(TAG, "esp_ble_gap_config_scan_rsp_data_raw failed: %s", esp_err_to_name(err2));
            if (s_instance)
                s_instance->sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:SCAN_RSP_CFG");
        }

        // Se ambos já estavam “prontos”, inicia advertising sem depender dos eventos.
        if (s_adv_cfg_done && s_scan_rsp_cfg_done)
        {
            esp_err_t st = esp_ble_gap_start_advertising(&s_advParams);
            ESP_LOGI(TAG, "esp_ble_gap_start_advertising => %s", esp_err_to_name(st));
        }
    }

    void BleProvisioningChannel::gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
    {
        if (!s_instance)
        {
            return;
        }

        switch (event)
        {
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
            s_adv_cfg_done = true;
            s_instance->_logger.info(TAG, "GAP: ADV_DATA_RAW_SET_COMPLETE status=%u", param->adv_data_cmpl.status);
            tryStartAdvertising();
            break;
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            s_adv_cfg_done = true;
            s_instance->_logger.info(TAG, "GAP: ADV_DATA_SET_COMPLETE status=%u", param->adv_data_cmpl.status);
            tryStartAdvertising();
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
            s_scan_rsp_cfg_done = true;
            s_instance->_logger.info(TAG, "GAP: SCAN_RSP_DATA_RAW_SET_COMPLETE status=%u", param->scan_rsp_data_cmpl.status);
            tryStartAdvertising();
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            s_scan_rsp_cfg_done = true;
            s_instance->_logger.info(TAG, "GAP: SCAN_RSP_DATA_SET_COMPLETE status=%u", param->scan_rsp_data_cmpl.status);
            tryStartAdvertising();
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
            {
                s_instance->sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:ADV_START_FAILED");
            }
            else
            {
                s_instance->sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] ADVERTISING");
                s_instance->_logger.info(TAG, "GAP: advertising started (interval=%u-%u units)", s_advParams.adv_int_min, s_advParams.adv_int_max);
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
            strncpy(s_adv_name, dev_name, sizeof(s_adv_name) - 1);
            s_adv_name[sizeof(s_adv_name) - 1] = '\0';
            s_instance->_logger.info(TAG, "GATTS: REG_EVT gatts_if=%u dev_name=%s", (unsigned)gatts_if, s_adv_name);
            esp_err_t rc_name = esp_ble_gap_set_device_name(s_adv_name);
            s_instance->_logger.info(TAG, "GATTS: set_device_name -> %s", esp_err_to_name(rc_name));
            startAdvertising();
            esp_err_t rc_attr = esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HRS_IDX_NB, 0);
            s_instance->_logger.info(TAG, "GATTS: create_attr_tab -> %s", esp_err_to_name(rc_attr));
            break;
        }

        case ESP_GATTS_CREAT_ATTR_TAB_EVT:
        {
            if (param->add_attr_tab.status == ESP_GATT_OK && param->add_attr_tab.num_handle == HRS_IDX_NB)
            {
                memcpy(s_handleTable, param->add_attr_tab.handles, sizeof(s_handleTable));
                esp_ble_gatts_start_service(s_handleTable[IDX_SVC]);
                s_instance->sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] READY");
                s_instance->_logger.info(TAG, "GATTS: attribute table created and service started");
            }
            else
            {
                s_instance->sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:ATTR_TAB");
                s_instance->_logger.error(TAG, "GATTS: create attr table failed status=%d num_handle=%d", param->add_attr_tab.status, param->add_attr_tab.num_handle);
            }
            break;
        }

        case ESP_GATTS_CONNECT_EVT:
        {
            s_instance->_connId = param->connect.conn_id;
            s_send_ssids_requested = true;
            s_instance->_logger.info(TAG, "GATTS: CONNECT conn_id=%u", (unsigned)param->connect.conn_id);
            s_instance->sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] CONNECTED");
            break;
        }

        case ESP_GATTS_DISCONNECT_EVT:
        {
            s_instance->_connId = 0xFFFF;
            s_instance->_notifyEnabled = false;

            s_rx_in_progress = false;
            s_rx_buffer.clear();
            s_rx_expected_len = 0;

            s_send_ssids_requested = false;
            s_sending_ssids = false;
            s_pending_ssids.clear();
            s_pending_ssid_index = 0;

            s_instance->_logger.info(TAG, "GATTS: DISCONNECT reason=%u", (unsigned)param->disconnect.reason);
            esp_ble_gap_start_advertising(&s_advParams);
            s_instance->sendStatus(ProvisioningStatus::WaitingUserInput, "[BLE] DISCONNECTED");
            break;
        }

        case ESP_GATTS_WRITE_EVT:
        {
            const auto &w = param->write;

            if (w.handle == s_handleTable[IDX_CHAR_VAL_CONFIG])
            {
                s_instance->_logger.info(TAG, "GATTS: WRITE config len=%u", (unsigned)w.len);
                s_instance->onConfigWrite(w.value, w.len);
            }
            else if (w.handle == s_handleTable[IDX_CHAR_CFG_STATUS] && w.len == 2)
            {
                uint16_t v = (uint16_t)(w.value[1] << 8) | (uint16_t)(w.value[0]);
                s_instance->_notifyEnabled = (v == 0x0001);
                s_instance->_logger.info(TAG, "GATTS: CCCD write value=0x%04X notifyEnabled=%d", v, s_instance->_notifyEnabled ? 1 : 0);
                if (s_instance->_notifyEnabled)
                {
                    s_send_ssids_requested = true;
                }
            }
            break;
        }

        default:
            break;
        }
    }

    void BleProvisioningChannel::tryStartAdvertising()
    {
        if (!s_adv_cfg_done || !s_scan_rsp_cfg_done)
        {
            if (s_pending_adv_start_log)
            {
                if (s_instance)
                {
                    s_instance->_logger.info(TAG, "tryStartAdvertising: waiting adv_cfg_done=%d scan_rsp_cfg_done=%d", s_adv_cfg_done, s_scan_rsp_cfg_done);
                }
                s_pending_adv_start_log = false;
            }
            return;
        }

        if (s_instance)
        {
            s_instance->_logger.info(TAG, "tryStartAdvertising: attempting start (adv_cfg_done=%d scan_rsp_cfg_done=%d)", s_adv_cfg_done, s_scan_rsp_cfg_done);
        }
        esp_err_t start_err = esp_ble_gap_start_advertising(&s_advParams);
        if (start_err != ESP_OK && start_err != ESP_ERR_INVALID_STATE)
        {
            if (s_instance)
            {
                s_instance->_logger.error(TAG, "esp_ble_gap_start_advertising failed: %s", esp_err_to_name(start_err));
                s_instance->sendStatus(ProvisioningStatus::Failed, "[BLE] ERROR:ADV_START_FAILED");
            }
        }
    }

} // namespace iotsmartsys::core::provisioning
