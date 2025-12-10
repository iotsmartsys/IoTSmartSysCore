#pragma once

#include <Arduino.h>

#ifdef ESP_NOW_ENABLED

#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>

typedef struct struct_message
{
    char device_id[32];
    char capability_name[32];
    char value[32];
    char type[32];
} struct_message;

#define MAX_PEERS 20


extern struct_message incomingData;
extern struct_message outgoingData;
extern uint8_t peerMacs[MAX_PEERS][6];
extern int peerCount;
extern char peerDeviceIds[MAX_PEERS][32];


void sendNotify(String payload);
void setEspNowMqttBridge(bool active);
bool isEspNowMqttBridgeEnabled();


void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len);
void setup_esp_now();


typedef void (*espnow_command_cb)(const struct_message &);
extern espnow_command_cb g_espnow_command_cb;
extern "C" void espnow_set_command_callback(espnow_command_cb cb);


extern bool g_has_central_mac;
extern uint8_t g_central_mac[6];


extern "C" void espnow_on_peer_registered(const char *device_id);

#endif 
