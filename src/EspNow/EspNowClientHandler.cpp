#include "EspNowClientHandler.h"
#include "Wifi/WifiHelper.h"
#include "Utils/Logger.h"

#ifdef ESP_NOW_ENABLED


static EspNowClientHandler *g_espNowHandler = nullptr;


extern "C" void espnow_set_command_callback(void (*cb)(const struct_message &));

static void on_espnow_cmd_cb(const struct_message &msg) {
    if (g_espNowHandler) {
        g_espNowHandler->onReceiveMessage(msg);
    }
}

#endif 

EspNowClientHandler::EspNowClientHandler(const char *device_name, const std::vector<Capability *> &capabilities)
    : device_name(device_name), capabilities(capabilities) {}

void EspNowClientHandler::setup() {
#ifdef ESP_NOW_ENABLED
    
    WiFi.mode(WIFI_STA);

    device_id = getDeviceId();

    
    setup_esp_now();

    
    g_espNowHandler = this;
    espnow_set_command_callback(on_espnow_cmd_cb);

    initialized = true;
#else
    initialized = false;
#endif
}

void EspNowClientHandler::handle() {
#ifdef ESP_NOW_ENABLED
    if (!initialized) return;
    flushQueueToPeers();
#endif
}

void EspNowClientHandler::sendState(CapabilityState state) {
#ifdef ESP_NOW_ENABLED
    if (!initialized) return;

    
    struct_message m{};
    String dev = state.device_id.length() ? state.device_id : device_id;
    strncpy(m.device_id, dev.c_str(), sizeof(m.device_id) - 1);
    strncpy(m.capability_name, state.capability_name.c_str(), sizeof(m.capability_name) - 1);
    strncpy(m.value, state.value.c_str(), sizeof(m.value) - 1);
    strncpy(m.type, state.type.c_str(), sizeof(m.type) - 1);

    enqueue(m);
#else
    (void)state;
#endif
}

void EspNowClientHandler::sendMessage(const char *topic, String payload) {
#ifdef ESP_NOW_ENABLED
    if (!initialized) return;

    
    String t = topic ? String(topic) : String("");
    if (!t.endsWith("/command")) return;

    
    CapabilityCommand cmd;
    if (!cmd.fromJson(payload.c_str())) return;

    struct_message m{};
    
    strncpy(m.device_id, cmd.device_id.c_str(), sizeof(m.device_id) - 1);
    strncpy(m.capability_name, cmd.capability_name.c_str(), sizeof(m.capability_name) - 1);
    strncpy(m.value, cmd.value.c_str(), sizeof(m.value) - 1);
    m.type[0] = '\0';

    
    int targetIdx = -1;
    for (int i = 0; i < peerCount; ++i) {
        if (strncmp(peerDeviceIds[i], m.device_id, sizeof(peerDeviceIds[i])) == 0) {
            targetIdx = i; break;
        }
    }

    if (targetIdx >= 0) {
        esp_now_send(peerMacs[targetIdx], reinterpret_cast<const uint8_t *>(&m), sizeof(m));
    } else {
        static uint8_t broadcastAddress[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        esp_now_send(broadcastAddress, reinterpret_cast<const uint8_t *>(&m), sizeof(m));
    }
#else
    (void)payload;
#endif
}

bool EspNowClientHandler::isPowerOn() { return power_on; }

bool EspNowClientHandler::isConnected() { return initialized; }


#ifdef ESP_NOW_ENABLED
void EspNowClientHandler::enqueue(const struct_message &m) {
    if (qCount >= QUEUE_SIZE) {
        
        qHead = (qHead + 1) % QUEUE_SIZE;
        qCount--;
    }
    queue[qTail].msg = m;
    qTail = (qTail + 1) % QUEUE_SIZE;
    qCount++;
}

bool EspNowClientHandler::dequeue(struct_message &out) {
    if (qCount == 0) return false;
    out = queue[qHead].msg;
    qHead = (qHead + 1) % QUEUE_SIZE;
    qCount--;
    return true;
}

void EspNowClientHandler::flushQueueToPeers() {
    struct_message m;
    while (dequeue(m)) {
        LOG_PRINTLN("[ESP-NOW] Enviando mensagem enfileirada...");
        sendToAllPeers(m);
    }
}

void EspNowClientHandler::sendToAllPeers(const struct_message &m) {
    
#ifdef ESP_NOW_ENABLED
    if (g_has_central_mac) {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 g_central_mac[0], g_central_mac[1], g_central_mac[2],
                 g_central_mac[3], g_central_mac[4], g_central_mac[5]);
        LOG_PRINT("[ESP-NOW] Enviando direto para CENTRAL: ");
        LOG_PRINTLN(macStr);
        esp_err_t rc = esp_now_send(g_central_mac, reinterpret_cast<const uint8_t *>(&m), sizeof(m));
        if (rc != ESP_OK) {
            LOG_PRINTF("[ESP-NOW] esp_now_send(direto) erro imediato: 0x%X\n", (unsigned)rc);
        }
        return;
    }
#endif
    LOG_PRINTLN("[ESP-NOW] MAC da CENTRAL não disponível. Nenhum envio realizado.");
}

void EspNowClientHandler::onReceiveMessage(const struct_message &msg) {
    
    CapabilityCommand command;
    command.device_id = String(msg.device_id);
    command.capability_name = String(msg.capability_name);
    command.value = String(msg.value);

    applyCommandToCapabilities(command, const_cast<std::vector<Capability*>&>(capabilities), power_on, nullptr);
}
#endif 
