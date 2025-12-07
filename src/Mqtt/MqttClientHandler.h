#pragma once

#ifndef MQTTCLIENTHANDLER_H
#define MQTTCLIENTHANDLER_H

#include <PubSubClient.h>
#ifdef ESP32
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#else
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#endif
#include <Client.h>
#include "Capabilities/CapabilityCommand.h"
#include "Capabilities/Capability.h"
#include "Models/Credentials.h"
#include <Models/Property.h>
#include "Settings/Models/MqttSettings.h"

#define MQTT_PROTOCOL "MQTT"
void command_exec(CapabilityCommand command);
class AnnouncePayloadBuilder; // forward declaration for DI

class MqttClientHandler
{
public:
    // announceBuilder may be provided by IoTCore (preferred). If nullptr, MqttClientHandler will create a temporary builder when announcing.
    MqttClientHandler(const char *device_name, MqttSettings mqttSettings, const std::vector<Capability *> &capabilities, const std::vector<Property *> &properties, AnnouncePayloadBuilder *announceBuilder = nullptr);
    void setup();
    void handle();
    void sendState(CapabilityState state);
    void sendMqttMessage(const char *topic, String payload);
    bool subscribeDeviceCommand(const String &device_id);
    void subscribe(const char *topic);
    bool isPowerOn();
    bool isConnected();

private:

    String device_id;
    String mac_address;
    const char *device_name;
    MqttSettings mqttSettings;
    std::vector<Capability *> capabilities;
    std::vector<Property *> properties;
    bool power_on = true;

    WiFiClientSecure wifiSecureClient;
    WiFiClient wifiClient;
    PubSubClient client;

    enum class BrokerType
    {
        PRIMARY,
        SECONDARY
    };
    BrokerType currentBroker = BrokerType::PRIMARY;
    unsigned long lastReconnectAttempt = 0;
    unsigned long lastPrimaryProbe = 0;
    const unsigned long reconnectIntervalMs = 70000;
    bool reconnecting = false;

    AnnouncePayloadBuilder *announceBuilder = nullptr;

    void announceDevice();
    void selectBroker(BrokerType target);
    Client &getCurrentClient(const MqttConfig &mqttConfig);
    MqttConfig currentMqttConfig;
    MqttConfig getCurrentMqttConfig() const;
    bool connectCurrent();
    void updatePropertiesInfo(std::vector<Property *> &properties);
    bool subscribeToCommandTopic();

    MqttClientHandler(const MqttClientHandler &) = delete;
    MqttClientHandler &operator=(const MqttClientHandler &) = delete;

    void callbackMqtt(char *topic, byte *payload, unsigned int length);
};

#endif
