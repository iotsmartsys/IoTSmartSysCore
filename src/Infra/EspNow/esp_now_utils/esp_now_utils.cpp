#include <Arduino.h>
#include "Infra/Utils/Logger.h"

#ifdef ESP_NOW_ENABLED

#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#ifdef ESP32
#include <esp_system.h>
#endif
#include "esp_now_utils/esp_now_utils.h"

struct_message incomingData;
struct_message outgoingData;

uint8_t peerMacs[MAX_PEERS][6];
int peerCount = 0;
char peerDeviceIds[MAX_PEERS][32];

espnow_command_cb g_espnow_command_cb = nullptr;
bool g_has_central_mac = false;
uint8_t g_central_mac[6] = {0};

static bool parseMac(const char *str, uint8_t mac[6])
{
    if (!str)
        return false;
    int v[6];
    if (sscanf(str, "%x:%x:%x:%x:%x:%x", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]) == 6)
    {
        for (int i = 0; i < 6; ++i)
            mac[i] = (uint8_t)v[i];
        return true;
    }
    return false;
}

static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (!mac_addr)
        return;
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    LOG_PRINTF("ESP-NOW envio para %s: %s\n", macStr,
               status == ESP_NOW_SEND_SUCCESS ? "SUCESSO" : "FALHA");
}

extern "C" void espnow_set_command_callback(espnow_command_cb cb)
{
    g_espnow_command_cb = cb;
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingDataRaw, int len)
{
    LOG_PRINTLN(">>> Mensagem recebida via ESP-NOW <<<");
    struct_message receivedData;
    memcpy(&receivedData, incomingDataRaw, min(len, (int)sizeof(receivedData)));
    char did[4 + 12 + 1];
    strcpy(did, "esp-");

    snprintf(did + 4, 13, "%02x%02x%02x%02x%02x%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    String deviceId = String(did);
    String capabilityName = String(receivedData.capability_name);
    String value = String(receivedData.value);
    String type = String(receivedData.type);
    String payload = "{\"device_id\":\"" + deviceId + "\", \"capability_name\":\"" + capabilityName + "\", \"value\":\"" + value + "\", \"type\":\"" + type + "\"}";
    LOG_PRINT("Payload: ");
    LOG_PRINTLN(payload);
    String mac_addr_str = String(mac_addr[0], HEX) + ":" + String(mac_addr[1], HEX) + ":" + String(mac_addr[2], HEX) + ":" + String(mac_addr[3], HEX) + ":" + String(mac_addr[4], HEX) + ":" + String(mac_addr[5], HEX);
    delay(250);
    LOG_PRINT("Mac Address: ");
    LOG_PRINTLN(mac_addr_str);

    sendNotify(payload);

    if (g_espnow_command_cb)
    {
        g_espnow_command_cb(receivedData);
    }

    bool exists = false;
    int idx = -1;
    for (int i = 0; i < peerCount; i++)
    {
        if (memcmp(peerMacs[i], mac_addr, 6) == 0)
        {
            exists = true;
            idx = i;
            break;
        }
    }

    if (!exists && peerCount < MAX_PEERS)
    {
        idx = peerCount;
        memcpy(peerMacs[idx], mac_addr, 6);
        peerCount++;

        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, mac_addr, 6);
        peerInfo.channel = WiFi.channel();
        peerInfo.encrypt = false;
#ifdef ESP32
        peerInfo.ifidx = WIFI_IF_STA;
#endif

        if (esp_now_add_peer(&peerInfo) == ESP_OK)
        {
            LOG_PRINTLN("Novo peer registrado com sucesso!");

            espnow_on_peer_registered(deviceId.c_str());
        }
        else
        {
            LOG_PRINTLN("Falha ao registrar o peer.");
        }
    }

    if (idx >= 0)
    {
        memset(peerDeviceIds[idx], 0, sizeof(peerDeviceIds[idx]));
        strncpy(peerDeviceIds[idx], deviceId.c_str(), sizeof(peerDeviceIds[idx]) - 1);
    }
    else if (exists)
    {
        LOG_PRINTLN("Peer já registrado.");
    }
    else
    {
        LOG_PRINTLN("Lista de peers cheia, não é possível adicionar.");
    }
}

void setup_esp_now()
{

#ifdef ESP32
    esp_reset_reason_t rr = esp_reset_reason();
    LOG_PRINTF("Reset reason: %d\n", (int)rr);
#endif

    WiFi.mode(WIFI_STA);

    esp_wifi_set_ps(WIFI_PS_NONE);

#ifdef ESPNOW_CHANNEL
    {
        uint8_t ch = (uint8_t)ESPNOW_CHANNEL;
        if (ch < 1 || ch > 13)
        {
            LOG_PRINTF("ESPNOW_CHANNEL inválido (%d). Usando canal 1.\n", ch);
            ch = 1;
        }
        esp_err_t err = esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
        if (err == ESP_OK)
        {
            LOG_PRINTF("ESP-NOW ajustado para canal: %d\n", ch);
        }
        else
        {
            LOG_PRINTF("Falha ao ajustar canal (%d), err=0x%x\n", ch, (unsigned)err);
        }
    }
#endif

    uint8_t current_channel = WiFi.channel();
    LOG_PRINT("Canal atual do ESP-NOW: ");
    LOG_PRINTLN(current_channel);

    if (esp_now_init() != ESP_OK)
    {
        LOG_PRINTLN("Erro ao inicializar ESP-NOW");
        return;
    }
    LOG_PRINTLN("ESP-NOW inicializado com sucesso");
    LOG_PRINTLN("Mac Address: ");
    LOG_PRINTLN(WiFi.macAddress());

    esp_now_register_recv_cb(OnDataRecv);
    esp_now_register_send_cb(OnDataSent);

#ifdef ESPNOW_CENTRAL_MAC
    {
        uint8_t mac[6];
        if (parseMac(ESPNOW_CENTRAL_MAC, mac))
        {
            memcpy(g_central_mac, mac, 6);
            g_has_central_mac = true;
            esp_now_peer_info_t p = {};
            memcpy(p.peer_addr, mac, 6);

            p.channel = WiFi.channel();
            p.encrypt = false;
#ifdef ESP32
            p.ifidx = WIFI_IF_STA;
#endif
            esp_err_t r = esp_now_add_peer(&p);
            if (r == ESP_OK || r == ESP_ERR_ESPNOW_EXIST)
            {
                LOG_PRINT("Peer CENTRAL registrado: ");
                LOG_PRINTLN(ESPNOW_CENTRAL_MAC);

                bool already = false;
                for (int i = 0; i < peerCount; ++i)
                {
                    if (memcmp(peerMacs[i], mac, 6) == 0)
                    {
                        already = true;
                        break;
                    }
                }
                if (!already && peerCount < MAX_PEERS)
                {
                    memcpy(peerMacs[peerCount], mac, 6);

                    snprintf(peerDeviceIds[peerCount], sizeof(peerDeviceIds[peerCount]),
                             "%02X:%02X:%02X:%02X:%02X:%02X",
                             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                    peerCount++;
                }
            }
            else
            {
                LOG_PRINTF("Falha ao registrar CENTRAL: 0x%X\n", (unsigned)r);
            }
        }
        else
        {
            LOG_PRINTLN("ESPNOW_CENTRAL_MAC inválido");
        }
    }
#endif
}

#endif
