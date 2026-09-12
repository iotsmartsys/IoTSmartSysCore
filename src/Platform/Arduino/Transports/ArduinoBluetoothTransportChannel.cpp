#include "ArduinoBluetoothTransportChannel.h"
#if IOTSMARTSYS_BLE_COMMAND_ENABLED
#include <Arduino.h>
#include <algorithm>
#include <cstring>
#include <vector>
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_system.h"
#include "mbedtls/md.h"
extern "C" {
#include "mbedtls/constant_time.h"
}
#include "mbedtls/aes.h"
#include "cJSON.h"

namespace iotsmartsys::platform::arduino
{
    using namespace core;
    namespace
    {
        enum Attribute { Service, InfoDecl, Info, CommandDecl, Command, ResponseDecl, Response,
                         ResponseCccd, AuthDecl, Auth, AuthCccd, Count };
        uint8_t uuids[5][16] = {
            {1,0,0x21,0x7b,0x8a,0x4d,0x63,0x9d,0x65,0x4c,0x4b,0x6f,0,0x10,0x8f,0x9d},
            {1,0,0x21,0x7b,0x8a,0x4d,0x63,0x9d,0x65,0x4c,0x4b,0x6f,1,0x10,0x8f,0x9d},
            {1,0,0x21,0x7b,0x8a,0x4d,0x63,0x9d,0x65,0x4c,0x4b,0x6f,2,0x10,0x8f,0x9d},
            {1,0,0x21,0x7b,0x8a,0x4d,0x63,0x9d,0x65,0x4c,0x4b,0x6f,3,0x10,0x8f,0x9d},
            {1,0,0x21,0x7b,0x8a,0x4d,0x63,0x9d,0x65,0x4c,0x4b,0x6f,4,0x10,0x8f,0x9d}};
        void wipe(void *data, size_t n) { auto p = static_cast<volatile uint8_t *>(data); while (n--) *p++ = 0; }
        bool equal(const uint8_t *a, const uint8_t *b, size_t n)
        { return mbedtls_ct_memcmp(a, b, n) == 0; }
        bool resolves(const uint8_t *address, const uint8_t *irk)
        {
            if ((address[0] & 0xc0) != 0x40) return false;
            // Bluedroid stores the IRK little endian; Bluetooth ah uses the low 24 bits.
            uint8_t key[16], plain[16]{}, cipher[16]{};
            for (size_t i = 0; i < 16; ++i) key[i] = irk[15-i];
            memcpy(plain + 13, address, 3);
            mbedtls_aes_context aes; mbedtls_aes_init(&aes);
            const bool ok = mbedtls_aes_setkey_enc(&aes, key, 128) == 0 &&
                mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, plain, cipher) == 0 && equal(cipher + 13, address + 3, 3);
            mbedtls_aes_free(&aes); wipe(key, sizeof(key)); wipe(cipher, sizeof(cipher)); return ok;
        }
        bool validTree(const cJSON *node, unsigned depth = 0)
        {
            if (depth > 16) return false;
            for (auto item = node->child; item; item = item->next)
            {
                if (cJSON_IsObject(node))
                    for (auto other = item->next; other; other = other->next)
                        if (!strcmp(item->string, other->string)) return false;
                if (!validTree(item, depth + 1)) return false;
            }
            return true;
        }
        bool utf8(const char *s, size_t size)
        {
            for (size_t i = 0; i < size;)
            {
                uint32_t c = static_cast<uint8_t>(s[i++]);
                if (c < 0x80) continue;
                unsigned more; uint32_t minimum;
                if (c >= 0xc2 && c <= 0xdf) { more = 1; minimum = 0x80; c &= 31; }
                else if (c >= 0xe0 && c <= 0xef) { more = 2; minimum = 0x800; c &= 15; }
                else if (c >= 0xf0 && c <= 0xf4) { more = 3; minimum = 0x10000; c &= 7; }
                else return false;
                if (i + more > size) return false;
                while (more--) { uint8_t b = s[i++]; if ((b & 0xc0) != 0x80) return false; c = (c << 6) | (b & 63); }
                if (c < minimum || c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff)) return false;
            }
            return true;
        }
    }

    BluetoothTransportChannel *BluetoothTransportChannel::instance_ = nullptr;
    portMUX_TYPE BluetoothTransportChannel::callbackMux_ = portMUX_INITIALIZER_UNLOCKED;

    BluetoothTransportChannel::BluetoothTransportChannel(ILogger &logger, ICommandParser &parser,
        ITransportDispatcher &common, CapabilityManager &capabilities, const BluetoothControlConfig &config)
        : logger_(logger), parser_(parser), common_(common), capabilities_(capabilities), config_(config)
    {
        if (config.sharedSecret && config.sharedSecretSize == sizeof(secret_)) memcpy(secret_, config.sharedSecret, sizeof(secret_));
        else config_.sharedSecretSize = 0;
        config_.sharedSecret = nullptr;
    }
    BluetoothTransportChannel::~BluetoothTransportChannel()
    {
        blocked_ = true;
        if (stackOwned_) { esp_ble_gap_stop_advertising(); closeSession(); }
        portENTER_CRITICAL(&callbackMux_);
        if (instance_ == this) instance_ = nullptr;
        portEXIT_CRITICAL(&callbackMux_);
        if (events_) vQueueDelete(events_);
        if (nvs_) nvs_close(nvs_);
        wipe(secret_, sizeof(secret_));
    }
    bool BluetoothTransportChannel::begin(const TransportConfig &config)
    {
        if (configured_ || running_) return false;
        const size_t n = config.clientId ? strlen(config.clientId) : 0;
        bool valid = n && n <= 13 && config_.sharedSecretSize == 32 && config_.localControlsConfigured;
        for (size_t i = 0; i < n; ++i) valid &= uint8_t(config.clientId[i]) <= 0x7f;
        for (auto timeout : {config_.pairingWindowMs, config_.securityTimeoutMs, config_.authTimeoutMs,
             config_.receiveTimeoutMs, config_.sessionTimeoutMs, config_.disconnectTimeoutMs})
            valid &= timeout > 0 && timeout < 0x80000000UL;
        if (!valid) { result_ = BluetoothControlResult::InvalidConfiguration; return false; }
        memcpy(deviceId_, config.clientId, n);
        events_ = xQueueCreate(16, sizeof(Event));
        if (!events_) { result_ = BluetoothControlResult::StackUnavailable; return false; }
        dispatcher_.reset(new BluetoothDispatcher(*this, parser_, common_, capabilities_, logger_, config_, deviceId_));
        configured_ = true; result_ = BluetoothControlResult::Ok; return true;
    }
    void BluetoothTransportChannel::start()
    {
        if (!configured_) return;
        if (stackOwned_) { if (stopped_ || stopRequested_) startRequested_ = true; return; }
        if (running_) return;
        if (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_IDLE ||
            esp_bluedroid_get_status() != ESP_BLUEDROID_STATUS_UNINITIALIZED)
        { fail(BluetoothControlResult::StackUnavailable); return; }
        portENTER_CRITICAL(&callbackMux_);
        bool available = !instance_; if (available) instance_ = this;
        portEXIT_CRITICAL(&callbackMux_);
        if (!available) { fail(BluetoothControlResult::StackUnavailable); return; }
        if (nvs_open("ble_control", NVS_READWRITE, &nvs_) != ESP_OK || !loadRecord())
        { fail(BluetoothControlResult::StorageError); return; }
        esp_bt_controller_config_t bt = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        if (esp_bt_controller_init(&bt) != ESP_OK || esp_bt_controller_enable(ESP_BT_MODE_BLE) != ESP_OK ||
            esp_bluedroid_init() != ESP_OK || esp_bluedroid_enable() != ESP_OK)
        { fail(BluetoothControlResult::StackUnavailable); return; }
        stackOwned_ = true;
        uint8_t mode = ESP_LE_AUTH_REQ_SC_BOND, io = ESP_IO_CAP_NONE, onlySc = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_ENABLE;
        uint8_t keySize = 16, keys = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
        if (esp_ble_gap_register_callback(gapCallback) != ESP_OK || esp_ble_gatts_register_callback(gattCallback) != ESP_OK ||
            esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &mode, 1) != ESP_OK ||
            esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &io, 1) != ESP_OK ||
            esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &onlySc, 1) != ESP_OK ||
            esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &keySize, 1) != ESP_OK ||
            esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &keys, 1) != ESP_OK ||
            esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &keys, 1) != ESP_OK)
        { fail(BluetoothControlResult::StackUnavailable); return; }
        running_ = true; startAt_ = millis();
        if (esp_ble_gatts_app_register(0x4953) != ESP_OK) { fail(BluetoothControlResult::StackUnavailable); return; }
        if (record_.state == 1 || record_.state == 3) beginRemoval();
    }
    void BluetoothTransportChannel::stop() { blocked_ = true; startRequested_ = false; stopRequested_ = true; }
    BluetoothControlState BluetoothTransportChannel::state() const
    {
        if (!configured_) return result_ == BluetoothControlResult::Disabled ? BluetoothControlState::Disabled : BluetoothControlState::Unavailable;
        if (stopped_) return BluetoothControlState::Disabled;
        if (!running_ || blocked_) return BluetoothControlState::Unavailable;
        return linkAlive_ ? dispatcher_->state() : BluetoothControlState::Advertising;
    }
    BluetoothControlResult BluetoothTransportChannel::openPairingWindow()
    {
        if (!running_ || stopped_) return BluetoothControlResult::Disabled;
        if (boundVisible_) return BluetoothControlResult::AlreadyBound;
        if (blocked_ || linkAlive_ || openRequested_.exchange(true)) return BluetoothControlResult::Busy;
        result_ = BluetoothControlResult::Pending; return BluetoothControlResult::Pending;
    }
    BluetoothControlResult BluetoothTransportChannel::revokeBleBonds()
    {
        if (!running_ || stopped_) return BluetoothControlResult::Disabled;
        if (storageFailed_) return BluetoothControlResult::StorageError;
        blocked_ = true; windowVisible_ = false; revokeRequested_ = true;
        result_ = BluetoothControlResult::Pending; return BluetoothControlResult::Pending;
    }
    bool BluetoothTransportChannel::windowOpen() const
    { return windowVisible_ && uint32_t(millis() - windowAt_) < config_.pairingWindowMs; }
    bool BluetoothTransportChannel::sessionAllowed() const
    { return running_ && linkAlive_ && !blocked_ && !revokeRequested_ && !stopRequested_ && !stopped_ && (candidate_ ? windowOpen() : record_.state == 2); }
    void BluetoothTransportChannel::fail(BluetoothControlResult reason)
    {
        const bool changed = !blocked_ || result_ != reason;
        blocked_ = true; windowVisible_ = false; result_ = reason;
        if (reason == BluetoothControlResult::StorageError) storageFailed_ = true;
        if (changed) logger_.warn("BLE", "Control unavailable (%u)", unsigned(reason));
        if (stackOwned_) { esp_ble_gap_stop_advertising(); closeSession(); }
    }
    bool BluetoothTransportChannel::loadRecord()
    {
        uint8_t data[15]{}; size_t n = sizeof(data);
        auto error = nvs_get_blob(nvs_, "peer", data, &n);
        if (error == ESP_ERR_NVS_NOT_FOUND) return true;
        if (error != ESP_OK || n != sizeof(data) || data[0] != 1 || data[1] > 3) return false;
        record_.state = data[1]; record_.addressType = data[2];
        memcpy(record_.identity, data + 3, 6); memcpy(record_.bondAddress, data + 9, 6);
        boundVisible_ = record_.state == 2; return true;
    }
    bool BluetoothTransportChannel::saveRecord(const PeerRecord &record)
    {
        uint8_t data[15] = {1, record.state, record.addressType};
        memcpy(data + 3, record.identity, 6); memcpy(data + 9, record.bondAddress, 6);
        if (nvs_set_blob(nvs_, "peer", data, sizeof(data)) != ESP_OK || nvs_commit(nvs_) != ESP_OK)
        { fail(BluetoothControlResult::StorageError); return false; }
        record_ = record; boundVisible_ = record.state == 2; return true;
    }
    bool BluetoothTransportChannel::resolvePeer(const uint8_t *address, PeerRecord &peer)
    {
        int count = esp_ble_get_bond_device_num();
        if (count <= 0 || count > 32) return false;
        std::vector<esp_ble_bond_dev_t> bonds(count);
        bool found = false;
        if (esp_ble_get_bond_device_list(&count, bonds.data()) == ESP_OK)
            for (int i = 0; i < count; ++i)
            {
                auto &bond = bonds[i]; auto &pid = bond.bond_key.pid_key;
                bool identity = bond.bond_key.key_mask & ESP_BLE_ID_KEY_MASK;
                if (equal(address, bond.bd_addr, 6) || (identity &&
                    (equal(address, pid.static_addr, 6) || resolves(address, pid.irk))))
                {
                    peer.state = 2; peer.addressType = identity ? pid.addr_type : BLE_ADDR_TYPE_PUBLIC;
                    memcpy(peer.identity, identity ? pid.static_addr : bond.bd_addr, 6);
                    memcpy(peer.bondAddress, bond.bd_addr, 6); found = identity; break;
                }
            }
        wipe(bonds.data(), bonds.size() * sizeof(bonds[0])); return found;
    }
    bool BluetoothTransportChannel::hasBond(const uint8_t *address)
    {
        PeerRecord peer;
        if (resolvePeer(address, peer)) return true;
        int count = esp_ble_get_bond_device_num();
        if (count < 0 || count > 32) return true; // Unreadable ownership fails closed.
        std::vector<esp_ble_bond_dev_t> bonds(count); bool present = false;
        if (count && esp_ble_get_bond_device_list(&count, bonds.data()) != ESP_OK) present = true;
        else for (int i = 0; i < count; ++i) present |= equal(bonds[i].bd_addr, address, 6);
        wipe(bonds.data(), bonds.size() * sizeof(esp_ble_bond_dev_t)); return present;
    }
    bool BluetoothTransportChannel::recordMatches(const PeerRecord &peer) const
    { return record_.addressType == peer.addressType && equal(record_.identity, peer.identity, 6); }
    void BluetoothTransportChannel::beginRemoval()
    {
        blocked_ = true; windowVisible_ = false; openRequested_ = false;
        if (storageFailed_) return;
        if (record_.state == 0) { blocked_ = false; result_ = BluetoothControlResult::Ok; return; }
        if (record_.state != 3) { auto record = record_; record.state = 3; if (!saveRecord(record)) return; }
        if (linkAlive_) { closeSession(); return; }
        if (removing_) return;
        // Journal precedes deletion; only the Control-owned peer may be removed.
        PeerRecord peer;
        if (resolvePeer(record_.bondAddress, peer))
        {
            // Bond storage may replace the connection RPA with the identity address.
            peer.state = 3;
            if (!saveRecord(peer)) return;
        }
        else
        {
            // A candidate may have disconnected before the stack stored any keys.
            int count = esp_ble_get_bond_device_num();
            if (count < 0 || count > 32) { fail(BluetoothControlResult::StorageError); return; }
            std::vector<esp_ble_bond_dev_t> bonds(count); bool present = false;
            if (count && esp_ble_get_bond_device_list(&count, bonds.data()) != ESP_OK)
            { fail(BluetoothControlResult::StorageError); return; }
            for (int i = 0; i < count; ++i) present |= equal(bonds[i].bd_addr, record_.bondAddress, 6);
            wipe(bonds.data(), bonds.size() * sizeof(esp_ble_bond_dev_t));
            if (!present) { finishRemoval(); return; }
        }
        removing_ = true; removalAt_ = millis();
        if (esp_ble_remove_bond_device(record_.bondAddress) != ESP_OK) { removing_ = false; fail(BluetoothControlResult::StorageError); }
    }
    void BluetoothTransportChannel::finishRemoval()
    {
        removing_ = false;
        if (saveRecord(PeerRecord{})) { blocked_ = false; candidate_ = false; if (result_ == BluetoothControlResult::Pending) result_ = BluetoothControlResult::Ok; advertise(); }
    }
    bool BluetoothTransportChannel::randomNonce(uint8_t *nonce, size_t n) { esp_fill_random(nonce, n); return true; }
    uint32_t BluetoothTransportChannel::nowMs() const { return millis(); }
    bool BluetoothTransportChannel::verifyProof(const char *id, const uint8_t *nonce, const uint8_t *proof)
    {
        static const char domain[] = "IoTSmartSys-Control-Auth-v1";
        uint8_t message[sizeof(domain) + 1 + 13 + 16]{}, tag[32]{};
        const size_t n = strlen(id); size_t offset = sizeof(domain);
        memcpy(message, domain, sizeof(domain)); message[offset++] = n;
        memcpy(message + offset, id, n); offset += n; memcpy(message + offset, nonce, 16); offset += 16;
        bool ok = mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), secret_, 32, message, offset, tag) == 0 && equal(tag, proof, 32);
        wipe(tag, sizeof(tag)); wipe(message, sizeof(message)); return ok;
    }
    BluetoothControlResult BluetoothTransportChannel::authorizePeer(uint32_t authStartedAt)
    {
        if (!secure_ || !sessionAllowed()) return BluetoothControlResult::AuthFailed;
        if (uint32_t(millis() - authStartedAt) >= config_.authTimeoutMs) return BluetoothControlResult::AuthTimeout;
        if (!candidate_) return record_.state == 2 ? BluetoothControlResult::Ok : BluetoothControlResult::AuthFailed;
        PeerRecord peer;
        if (!windowOpen()) return BluetoothControlResult::AuthTimeout;
        if (!resolvePeer(remote_, peer)) return BluetoothControlResult::AuthFailed;
        if (!saveRecord(peer)) return BluetoothControlResult::AuthStorageError;
        if (!windowOpen() || uint32_t(millis() - authStartedAt) >= config_.authTimeoutMs || !sessionAllowed())
        {
            beginRemoval();
            return storageFailed_ ? BluetoothControlResult::AuthStorageError : BluetoothControlResult::AuthTimeout;
        }
        candidate_ = false; windowVisible_ = false; result_ = BluetoothControlResult::Ok;
        return BluetoothControlResult::Ok;
    }
    void BluetoothTransportChannel::reportFailure(BluetoothControlResult reason)
    { result_ = reason; logger_.warn("BLE", "Session failed (%u)", unsigned(reason)); }
    void BluetoothTransportChannel::closeSession()
    {
        if (dispatcher_) dispatcher_->invalidate();
        if (linkAlive_ && !closingLink_)
        { closingLink_ = true; closeAt_ = millis(); esp_ble_gap_disconnect(remote_); }
    }
    bool BluetoothTransportChannel::notifyAuth(bool success)
    {
        uint8_t result[2] = {1, uint8_t(success ? 0 : 1)};
        if (!linkAlive_ || !authNotify_) return false;
        if (congested_ || esp_ble_gatts_send_indicate(interface_, connection_, handles_[Auth], 2, result, false) != ESP_OK)
        { reportFailure(BluetoothControlResult::NotifyFailed); return false; }
        return true;
    }
    bool BluetoothTransportChannel::notifyResponse(const char *json, size_t n)
    {
        if (!linkAlive_ || !responseNotify_ || responseSize_ || n > 128) return false;
        memcpy(response_, json, n); responseSize_ = n; responseOffset_ = 0; return true;
    }
    void BluetoothTransportChannel::transmit()
    {
        if (!responseSize_ || congested_ || !linkAlive_ || blocked_) return;
        size_t n = std::min(responseSize_ - responseOffset_, size_t(mtu_ - 3));
        if (esp_ble_gatts_send_indicate(interface_, connection_, handles_[Response], n,
            reinterpret_cast<uint8_t *>(response_ + responseOffset_), false) != ESP_OK)
        { responseSize_ = 0; reportFailure(BluetoothControlResult::NotifyFailed); closeSession(); return; }
        responseOffset_ += n;
        if (responseOffset_ == responseSize_) { responseSize_ = 0; wipe(response_, sizeof(response_)); }
    }
    bool BluetoothTransportChannel::validateCommandJson(const char *json, size_t size)
    {
        if (!size || size > 1024 || !utf8(json, size)) return false;
        // Bound nesting before allocating the cJSON tree; reject embedded NUL escapes.
        bool string = false, escape = false; unsigned depth = 0;
        size_t stringStart = 0;
        const char *fields[] = {"device_id", "capability_name", "value", "type", "args", "args1", "args1value"};
        unsigned firstDepth[7]{};
        for (size_t i = 0; i < size; ++i)
        {
            char c = json[i];
            if (string)
            {
                if (escape) { if (c == 'u' && i + 4 < size && !memcmp(json + i + 1, "0000", 4)) return false; escape = false; }
                else if (uint8_t(c) < 0x20) return false;
                else if (c == '\\') escape = true;
                else if (c == '"')
                {
                    string = false; size_t next = i + 1;
                    while (next < size && strchr(" \t\r", json[next])) ++next;
                    if (next < size && json[next] == ':')
                        for (size_t f = 0; f < 7; ++f)
                            if (!firstDepth[f] && i - stringStart == strlen(fields[f]) &&
                                !memcmp(json + stringStart, fields[f], i - stringStart)) firstDepth[f] = depth;
                }
            }
            else if (c == '"') { string = true; stringStart = i + 1; }
            else if (c == '-' || (c >= '0' && c <= '9'))
            {
                // cJSON accepts some non-JSON numeric spellings; enforce the grammar.
                size_t j = i;
                if (json[j] == '-' && ++j == size) return false;
                if (json[j] == '0') ++j;
                else if (json[j] >= '1' && json[j] <= '9')
                    while (j < size && json[j] >= '0' && json[j] <= '9') ++j;
                else return false;
                if (j < size && json[j] == '.')
                {
                    size_t first = ++j;
                    while (j < size && json[j] >= '0' && json[j] <= '9') ++j;
                    if (j == first) return false;
                }
                if (j < size && (json[j] == 'e' || json[j] == 'E'))
                {
                    ++j; if (j < size && (json[j] == '+' || json[j] == '-')) ++j;
                    size_t first = j;
                    while (j < size && json[j] >= '0' && json[j] <= '9') ++j;
                    if (j == first) return false;
                }
                if (j < size && !strchr(" ,]}\t\r", json[j])) return false;
                i = j - 1;
            }
            else if (c == '{' || c == '[') { if (++depth > 16) return false; }
            else if (c == '}' || c == ']') { if (!depth) return false; --depth; }
        }
        for (auto seen : firstDepth) if (seen > 1) return false;
        const char *end = nullptr;
        cJSON *root = cJSON_ParseWithLengthOpts(json, size + 1, &end, true);
        if (!root) return false;
        bool ok = cJSON_IsObject(root) && validTree(root);
        // Compare with the existing parser, so nested/shadow fields and unsupported
        // escapes cannot change the command seen by the shared dispatcher.
        for (auto key : {"device_id", "capability_name", "value"}) ok &= cJSON_IsString(cJSON_GetObjectItemCaseSensitive(root, key));
        auto type = cJSON_GetObjectItemCaseSensitive(root, "type");
        ok &= !type || cJSON_IsString(type);
        auto args = cJSON_GetObjectItemCaseSensitive(root, "args");
        ok &= !args || cJSON_IsArray(args);
        if (ok)
        {
            std::unique_ptr<DeviceCommand> cmd(parser_.parseCommand(json, size, false));
            ok = cmd && cmd->device_id == cJSON_GetObjectItemCaseSensitive(root, "device_id")->valuestring &&
                cmd->capability_name == cJSON_GetObjectItemCaseSensitive(root, "capability_name")->valuestring &&
                cmd->value == cJSON_GetObjectItemCaseSensitive(root, "value")->valuestring &&
                cmd->type == (type ? type->valuestring : CommandTypeStrings::CAPABILITY_STR);
            size_t i = 0;
            for (auto item = args ? args->child : nullptr; ok && item; item = item->next, ++i)
            {
                auto key = cJSON_GetObjectItemCaseSensitive(item, "key"), value = cJSON_GetObjectItemCaseSensitive(item, "value");
                ok = cJSON_IsObject(item) && cJSON_IsString(key) && cJSON_IsString(value) && i < cmd->args.size() &&
                    cmd->args[i].first == key->valuestring && cmd->args[i].second == value->valuestring;
            }
            if (ok) ok = i == cmd->args.size();
        }
        cJSON_Delete(root); return ok;
    }
    void BluetoothTransportChannel::configureGatt()
    {
        static uint16_t primary = ESP_GATT_UUID_PRI_SERVICE, declaration = ESP_GATT_UUID_CHAR_DECLARE, cccd = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
        static uint8_t read = ESP_GATT_CHAR_PROP_BIT_READ, write = ESP_GATT_CHAR_PROP_BIT_WRITE,
            notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY, auth = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        static uint8_t zero[2]{};
        esp_gatts_attr_db_t db[Count]{}; size_t count = 0;
        auto add = [&](uint8_t response, uint16_t uuidSize, uint8_t *uuid, uint16_t perm, uint16_t max, uint16_t len, uint8_t *value)
        { db[count].attr_control.auto_rsp = response; db[count++].att_desc = {uuidSize, uuid, perm, max, len, value}; };
        auto declare = [&](uint8_t *property)
        { add(ESP_GATT_AUTO_RSP, 2, reinterpret_cast<uint8_t *>(&declaration), ESP_GATT_PERM_READ, 1, 1, property); };
        auto descriptor = [&]()
        { add(ESP_GATT_RSP_BY_APP, 2, reinterpret_cast<uint8_t *>(&cccd), ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, 2, 2, zero); };
        add(ESP_GATT_AUTO_RSP, 2, reinterpret_cast<uint8_t *>(&primary), ESP_GATT_PERM_READ, 16, 16, uuids[0]);
        if (config_.deviceInfoEnabled)
        {
            declare(&read);
            add(ESP_GATT_RSP_BY_APP, 16, uuids[1], ESP_GATT_PERM_READ, 13, strlen(deviceId_), reinterpret_cast<uint8_t *>(deviceId_));
        }
        declare(&write); add(ESP_GATT_RSP_BY_APP, 16, uuids[2], ESP_GATT_PERM_WRITE_ENCRYPTED, 512, 0, nullptr);
        declare(&notify); add(ESP_GATT_RSP_BY_APP, 16, uuids[3], 0, 128, 0, nullptr); descriptor();
        declare(&auth); add(ESP_GATT_RSP_BY_APP, 16, uuids[4], ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED, 32, 0, nullptr); descriptor();
        if (esp_ble_gatts_create_attr_tab(db, interface_, count, 0) != ESP_OK) fail(BluetoothControlResult::StackUnavailable);
        uint8_t adv[21] = {2, 1, 6, 17, 7}; memcpy(adv + 5, uuids[0], 16);
        uint8_t scan[31]{}; size_t n = strlen(deviceId_);
        scan[0] = 17 + n; scan[1] = 0x21; memcpy(scan + 2, uuids[0], 16); memcpy(scan + 18, deviceId_, n);
        if (esp_ble_gap_config_adv_data_raw(adv, sizeof(adv)) != ESP_OK ||
            esp_ble_gap_config_scan_rsp_data_raw(scan, 18 + n) != ESP_OK) fail(BluetoothControlResult::StackUnavailable);
    }
    void BluetoothTransportChannel::advertise()
    {
        if (!running_ || stopped_ || blocked_ || advertising_ || linkAlive_ || connection_ != 0xffff || !serviceReady_ || !advReady_ || !scanReady_) return;
        esp_ble_adv_params_t params{};
        params.adv_int_min = 0x40; params.adv_int_max = 0x80; params.adv_type = ADV_TYPE_IND;
        params.own_addr_type = BLE_ADDR_TYPE_PUBLIC; params.channel_map = ADV_CHNL_ALL; params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;
        advertising_ = true;
        if (esp_ble_gap_start_advertising(&params) != ESP_OK) fail(BluetoothControlResult::StackUnavailable);
    }
    void BluetoothTransportChannel::enqueue(Event &event)
    { if (events_ && xQueueSend(events_, &event, 0) != pdTRUE) { fatalQueue_ = true; blocked_ = true; } }
    void BluetoothTransportChannel::gattCallback(esp_gatts_cb_event_t kind, esp_gatt_if_t interfaceId, esp_ble_gatts_cb_param_t *p)
    {
        portENTER_CRITICAL(&callbackMux_);
        auto self = instance_;
        if (!self) { portEXIT_CRITICAL(&callbackMux_); return; }
        Event e; e.interfaceId = interfaceId; e.generation = self->callbackGeneration_; bool emit = true;
        switch (kind)
        {
        case ESP_GATTS_REG_EVT: e.kind = EventKind::Registered; e.status = p->reg.status; break;
        case ESP_GATTS_CREAT_ATTR_TAB_EVT:
            e.kind = EventKind::Table; e.status = p->add_attr_tab.status; e.size = p->add_attr_tab.num_handle;
            if (e.size <= Count) memcpy(e.data, p->add_attr_tab.handles, e.size * sizeof(uint16_t)); else e.status = ESP_GATT_ERROR;
            break;
        case ESP_GATTS_START_EVT: e.kind = EventKind::ServiceStarted; e.status = p->start.status; break;
        case ESP_GATTS_CONNECT_EVT:
            e.kind = EventKind::Connect; e.connection = p->connect.conn_id; memcpy(e.address, p->connect.remote_bda, 6);
            // Admission happens in the transport task. Never replace an active session.
            if (self->linkAlive_) { e.kind = EventKind::RejectedConnect; break; }
            e.generation = ++self->callbackGeneration_; self->callbackConnection_ = e.connection; self->linkAlive_ = true; break;
        case ESP_GATTS_DISCONNECT_EVT:
            e.kind = EventKind::Disconnect; e.connection = p->disconnect.conn_id;
            if (e.connection == self->callbackConnection_) self->linkAlive_ = false;
            break;
        case ESP_GATTS_MTU_EVT: e.kind = EventKind::Mtu; e.connection = p->mtu.conn_id; e.size = p->mtu.mtu; break;
        case ESP_GATTS_READ_EVT:
            e.kind = EventKind::Read; e.connection = p->read.conn_id; e.transaction = p->read.trans_id;
            e.handle = p->read.handle; e.offset = p->read.offset; e.needResponse = p->read.need_rsp; break;
        case ESP_GATTS_WRITE_EVT:
            e.kind = EventKind::Write; e.connection = p->write.conn_id; e.transaction = p->write.trans_id;
            e.handle = p->write.handle; e.offset = p->write.offset; e.needResponse = p->write.need_rsp; e.prepared = p->write.is_prep;
            e.size = p->write.len;
            if (e.size <= sizeof(e.data)) memcpy(e.data, p->write.value, e.size); else e.kind = EventKind::Failure;
            break;
        case ESP_GATTS_EXEC_WRITE_EVT:
            e.kind = EventKind::Write; e.connection = p->exec_write.conn_id; e.transaction = p->exec_write.trans_id;
            e.prepared = true; e.needResponse = true; break;
        case ESP_GATTS_CONGEST_EVT: e.kind = EventKind::Congestion; e.connection = p->congest.conn_id; e.status = p->congest.congested; break;
        default: emit = false; break;
        }
        if (emit) self->enqueue(e);
        portEXIT_CRITICAL(&callbackMux_);
    }
    void BluetoothTransportChannel::gapCallback(esp_gap_ble_cb_event_t kind, esp_ble_gap_cb_param_t *p)
    {
        portENTER_CRITICAL(&callbackMux_);
        auto self = instance_;
        if (!self) { portEXIT_CRITICAL(&callbackMux_); return; }
        Event e; e.generation = self->callbackGeneration_; bool emit = true;
        switch (kind)
        {
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT: e.kind = EventKind::AdvReady; e.status = p->adv_data_raw_cmpl.status; break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT: e.kind = EventKind::ScanReady; e.status = p->scan_rsp_data_raw_cmpl.status; break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT: e.kind = EventKind::AdvStarted; e.status = p->adv_start_cmpl.status; break;
        case ESP_GAP_BLE_SEC_REQ_EVT:
            e.kind = EventKind::SecurityRequest; memcpy(e.address, p->ble_security.ble_req.bd_addr, 6); break;
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            e.kind = EventKind::SecurityComplete; memcpy(e.address, p->ble_security.auth_cmpl.bd_addr, 6);
            e.status = p->ble_security.auth_cmpl.success ? 0 : 1; e.authMode = p->ble_security.auth_cmpl.auth_mode;
            e.addressType = p->ble_security.auth_cmpl.addr_type;
            if (e.status) self->blocked_ = true;
            break;
        case ESP_GAP_BLE_KEY_EVT:
            if (p->ble_security.ble_key.key_type == ESP_LE_KEY_PID)
            {
                e.kind = EventKind::Identity; memcpy(e.address, p->ble_security.ble_key.bd_addr, 6);
                auto &pid = p->ble_security.ble_key.p_key_value.pid_key;
                memcpy(e.identity, pid.static_addr, 6); e.addressType = pid.addr_type;
            }
            else emit = false;
            break;
        case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
            e.kind = EventKind::Removed; e.status = p->remove_bond_dev_cmpl.status; memcpy(e.address, p->remove_bond_dev_cmpl.bd_addr, 6); break;
        default: emit = false; break;
        }
        if (emit) self->enqueue(e);
        portEXIT_CRITICAL(&callbackMux_);
    }
    void BluetoothTransportChannel::answer(const Event &e, BluetoothAccess status, const uint8_t *data, size_t n)
    {
        if (!e.needResponse) return;
        esp_gatt_status_t code = ESP_GATT_OK;
        if (status == BluetoothAccess::NotAuthorized) code = ESP_GATT_INSUF_AUTHORIZATION;
        else if (status == BluetoothAccess::InvalidLength) code = ESP_GATT_INVALID_ATTR_LEN;
        else if (status == BluetoothAccess::NotReady) code = ESP_GATT_BUSY;
        esp_gatt_rsp_t response{}; response.attr_value.handle = e.handle; response.attr_value.len = n;
        if (n) memcpy(response.attr_value.value, data, n);
        if (esp_ble_gatts_send_response(interface_, e.connection, e.transaction, code, &response) != ESP_OK) closeSession();
    }
    void BluetoothTransportChannel::access(const Event &e)
    {
        if (e.offset || e.prepared || (e.kind == EventKind::Write && (!e.needResponse || e.size > mtu_ - 3)))
        { answer(e, BluetoothAccess::InvalidLength); closeSession(); return; }
        if (blocked_ || !sessionAllowed()) { answer(e, BluetoothAccess::NotAuthorized); closeSession(); return; }
        if (e.handle == handles_[Info] && config_.deviceInfoEnabled && e.kind == EventKind::Read)
        { answer(e, BluetoothAccess::Ok, reinterpret_cast<uint8_t *>(deviceId_), strlen(deviceId_)); return; }
        if (e.handle == handles_[AuthCccd] || e.handle == handles_[ResponseCccd])
        {
            bool &enabled = e.handle == handles_[AuthCccd] ? authNotify_ : responseNotify_;
            if (e.kind == EventKind::Read) { uint8_t value[2] = {uint8_t(enabled), 0}; answer(e, BluetoothAccess::Ok, value, 2); }
            else if (e.size == 2 && e.data[0] <= 1 && e.data[1] == 0)
            { enabled = e.data[0]; answer(e, BluetoothAccess::Ok); dispatcher_->notifications(authNotify_, responseNotify_); }
            else answer(e, BluetoothAccess::InvalidLength);
            return;
        }
        if (!secure_) { answer(e, BluetoothAccess::NotAuthorized); return; }
        if (e.handle == handles_[Auth])
        {
            if (e.kind == EventKind::Read)
            { uint8_t out[17]{}; auto result = dispatcher_->readChallenge(millis(), out); answer(e, result, out, result == BluetoothAccess::Ok ? sizeof(out) : 0); }
            else answer(e, dispatcher_->writeAuth(e.data, e.size, millis()));
        }
        else if (e.handle == handles_[Command] && e.kind == EventKind::Write)
            answer(e, dispatcher_->writeCommand(e.data, e.size, millis()));
        else answer(e, BluetoothAccess::NotAuthorized);
    }
    void BluetoothTransportChannel::process(const Event &e)
    {
        switch (e.kind)
        {
        case EventKind::Registered:
            if (e.status) { fail(BluetoothControlResult::StackUnavailable); return; }
            interface_ = e.interfaceId; configureGatt(); return;
        case EventKind::Table:
        {
            if (e.status || e.size != (config_.deviceInfoEnabled ? Count : Count - 2)) { fail(BluetoothControlResult::StackUnavailable); return; }
            size_t j = 0;
            for (size_t i = 0; i < Count; ++i)
                if (config_.deviceInfoEnabled || (i != Info && i != InfoDecl)) { memcpy(&handles_[i], e.data + j * 2, 2); ++j; }
            tableReady_ = true;
            if (esp_ble_gatts_start_service(handles_[Service]) != ESP_OK) fail(BluetoothControlResult::StackUnavailable);
            return;
        }
        case EventKind::ServiceStarted: serviceReady_ = !e.status; if (e.status) fail(BluetoothControlResult::StackUnavailable); return;
        case EventKind::AdvReady: advReady_ = !e.status; if (e.status) fail(BluetoothControlResult::StackUnavailable); return;
        case EventKind::ScanReady: scanReady_ = !e.status; if (e.status) fail(BluetoothControlResult::StackUnavailable); return;
        case EventKind::AdvStarted: if (e.status) fail(BluetoothControlResult::StackUnavailable); return;
        case EventKind::Removed:
            if (removing_ && equal(record_.bondAddress, e.address, 6))
            { if (e.status) { removing_ = false; fail(BluetoothControlResult::StorageError); } else finishRemoval(); }
            return;
        case EventKind::RejectedConnect: esp_ble_gap_disconnect(const_cast<uint8_t *>(e.address)); return;
        case EventKind::Failure: fail(BluetoothControlResult::ProtocolError); return;
        default: break;
        }
        if (e.kind == EventKind::Connect)
        {
            generation_ = e.generation; connection_ = e.connection; memcpy(remote_, e.address, 6);
            advertising_ = false; closingLink_ = false; candidate_ = secure_ = authNotify_ = responseNotify_ = congested_ = false;
            mtu_ = 23; dispatcher_->connected(millis());
            PeerRecord peer;
            if (blocked_ || revokeRequested_ || stopRequested_) { closeSession(); return; }
            if (record_.state == 2)
            { if (!resolvePeer(remote_, peer) || !recordMatches(peer)) { closeSession(); return; } }
            else if (record_.state == 0 && windowOpen() && !hasBond(remote_))
            {
                peer = PeerRecord{}; peer.state = 1; memcpy(peer.bondAddress, remote_, 6);
                if (!saveRecord(peer)) return;
                candidate_ = true;
            }
            else { closeSession(); return; }
            if (esp_ble_set_encryption(remote_, ESP_BLE_SEC_ENCRYPT_NO_MITM) != ESP_OK) closeSession();
            else if (onConnected_)
            { TransportConnectedView info{deviceId_, nullptr, 0}; onConnected_(connectedUser_, info); }
            return;
        }
        if (e.generation != generation_) return;
        if ((e.kind == EventKind::Read || e.kind == EventKind::Write || e.kind == EventKind::Mtu ||
             e.kind == EventKind::Congestion || e.kind == EventKind::Disconnect) && e.connection != connection_) return;
        switch (e.kind)
        {
        case EventKind::Disconnect:
            closingLink_ = false; secure_ = authNotify_ = responseNotify_ = false;
            dispatcher_->disconnected(); responseSize_ = responseOffset_ = 0; wipe(response_, sizeof(response_));
            connection_ = 0xffff;
            if (onDisconnected_) onDisconnected_(disconnectedUser_);
            if (record_.state == 1 || record_.state == 3) beginRemoval();
            else if (!storageFailed_ && !fatalQueue_ && !stopRequested_) blocked_ = false;
            break;
        case EventKind::Mtu: mtu_ = std::max<uint16_t>(23, std::min<uint16_t>(517, e.size)); break;
        case EventKind::Read: case EventKind::Write: if (linkAlive_ && !closingLink_) access(e); break;
        case EventKind::SecurityRequest:
            esp_ble_gap_security_rsp(const_cast<uint8_t *>(e.address), equal(remote_, e.address, 6) && sessionAllowed()); break;
        case EventKind::Identity:
            if (candidate_ && record_.state == 1 && equal(remote_, e.address, 6))
            { auto record = record_; record.addressType = e.addressType; memcpy(record.identity, e.identity, 6); saveRecord(record); }
            break;
        case EventKind::SecurityComplete:
        {
            if (!equal(remote_, e.address, 6)) break;
            PeerRecord peer;
            if (e.status || (e.authMode & ESP_LE_AUTH_REQ_SC_BOND) != ESP_LE_AUTH_REQ_SC_BOND ||
                !sessionAllowed() || !resolvePeer(remote_, peer) || (!candidate_ && !recordMatches(peer)))
            { reportFailure(BluetoothControlResult::SecurityFailed); closeSession(); break; }
            if (candidate_) { peer.state = 1; if (!saveRecord(peer)) break; }
            secure_ = true; dispatcher_->secured(); break;
        }
        case EventKind::Congestion: congested_ = e.status; break;
        default: break;
        }
    }
    void BluetoothTransportChannel::handle()
    {
        if (!running_) return;
        if (fatalQueue_) { fail(BluetoothControlResult::ProtocolError); return; }
        if (stopRequested_.exchange(false))
        { stopped_ = true; blocked_ = true; windowVisible_ = false; advertising_ = false; esp_ble_gap_stop_advertising(); closeSession(); }
        if (startRequested_ && stopped_ && !linkAlive_ && record_.state != 1 && record_.state != 3 &&
            !storageFailed_)
        { startRequested_ = false; stopped_ = false; blocked_ = false; }
        if (revokeRequested_.exchange(false)) beginRemoval();
        if (openRequested_.exchange(false) && !blocked_ && !linkAlive_ && record_.state == 0)
        { windowAt_ = millis(); windowVisible_ = true; result_ = BluetoothControlResult::Ok; }
        if (windowVisible_ && !windowOpen()) { windowVisible_ = false; if (candidate_) closeSession(); }
        Event event;
        // Process one write before any following write can affect the same command.
        for (unsigned i = 0; i < 8 && xQueueReceive(events_, &event, 0) == pdTRUE; ++i)
        { process(event); if (linkAlive_ && !closingLink_) dispatcher_->handle(millis()); }
        if (linkAlive_ && !closingLink_) dispatcher_->handle(millis());
        transmit();
        if (record_.state == 3 && !linkAlive_ && !removing_ && !storageFailed_) beginRemoval();
        if (removing_ && uint32_t(millis() - removalAt_) >= config_.securityTimeoutMs) fail(BluetoothControlResult::StorageError);
        if (closingLink_ && linkAlive_ && uint32_t(millis() - closeAt_) >= config_.disconnectTimeoutMs)
            fail(BluetoothControlResult::StackUnavailable);
        if ((!tableReady_ || !serviceReady_ || !advReady_ || !scanReady_) && uint32_t(millis() - startAt_) >= config_.securityTimeoutMs)
            fail(BluetoothControlResult::StackUnavailable);
        advertise();
    }
}
#endif
