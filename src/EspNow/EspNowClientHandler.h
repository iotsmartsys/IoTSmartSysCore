#pragma once

#include <vector>
#include <Arduino.h>
#include "Transports/IMessageClient.h"
#include "Capabilities/Capability.h"
#include "Capabilities/CapabilityState.h"
#include "Capabilities/CapabilityCommand.h"
#include "Core/CommandExecutor.h"

#ifdef ESP32
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#endif

#ifdef ESP_NOW_ENABLED
#include "esp_now_utils/esp_now_utils.h"
#endif

class EspNowClientHandler : public IMessageClient {
public:
    EspNowClientHandler(const char *device_name, const std::vector<Capability *> &capabilities);
    ~EspNowClientHandler() override = default;

    void setup() override;
    void handle() override;

    void sendState(CapabilityState state) override;
    void sendMessage(const char *topic, String payload) override; 

    bool isPowerOn() override;
    bool isConnected() override; 

    
    void setPowerOn(bool on) { power_on = on; }

private:
    const char *device_name;
    String device_id;
    std::vector<Capability *> capabilities;
    bool power_on = true;
    bool initialized = false;

    
#ifdef ESP_NOW_ENABLED
    struct QueueMsg { struct_message msg; };
    static constexpr uint8_t QUEUE_SIZE = 10;
    QueueMsg queue[QUEUE_SIZE];
    uint8_t qHead = 0, qTail = 0, qCount = 0;

    void enqueue(const struct_message &m);
    bool dequeue(struct_message &out);

    
    void flushQueueToPeers();
    void sendToAllPeers(const struct_message &m);
#endif

public:
#ifdef ESP_NOW_ENABLED
    
    void onReceiveMessage(const struct_message &msg);
#else
    void flushQueueToPeers() {}
#endif
};
