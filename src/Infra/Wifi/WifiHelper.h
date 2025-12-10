#pragma once

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <vector>

void setupWifi(const char *ssid, const char *password);
bool connectToWifi(const char *ssid, const char *password);
void maintainWiFiConnection(const char *ssid, const char *password);

std::vector<std::string> getAvailableSSIDs();
String getDeviceId();
String getMacAddress();
