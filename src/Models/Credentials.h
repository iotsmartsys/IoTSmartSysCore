#pragma once

#include <Arduino.h>
#include <vector>

struct MqttCredentials
{
public:
    const char *mqtt_server;
    int mqtt_port;
    const char *mqtt_user;
    const char *mqtt_password;

    MqttCredentials(const char *mqtt_server, int mqtt_port, const char *mqtt_user, const char *mqtt_password)
        : mqtt_server(mqtt_server), mqtt_port(mqtt_port), mqtt_user(mqtt_user), mqtt_password(mqtt_password) {}
    
    MqttCredentials()
        : mqtt_server(nullptr), mqtt_port(0), mqtt_user(nullptr), mqtt_password(nullptr) {}
};

struct WifiCredentials
{
public:
    const char *ssid;
    const char *password;

    WifiCredentials(const char *ssid, const char *password)
        : ssid(ssid), password(password) {}
    
    WifiCredentials(){
        ssid = nullptr;
        password = nullptr;
    }
};
