
#pragma once

#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32
// Pin boot for ESP32-WROOM-32 module nodemcu
#define ESP32_BOOT_PIN 0

// Safe GPIO pins for ESP32-WROOM-32 module

#define ESP32_GPIO2 2

#ifndef LED_BUILTIN
#define LED_BUILTIN ESP32_GPIO2
#endif

#define ESP32_SAFE_GPIO4 4
#define ESP32_SAFE_GPIO5 5
#define ESP32_SAFE_GPIO13 13
#define ESP32_SAFE_GPIO14 14
// #define ESP32_SAFE_GPIO15 15 should be used with caution, as it can affect boot mode if pulled low during startup
#define ESP32_SAFE_GPIO16 16
#define ESP32_SAFE_GPIO17 17
#define ESP32_SAFE_GPIO18 18
#define ESP32_SAFE_GPIO19 19
#define ESP32_SAFE_GPIO21 21
#define ESP32_SAFE_GPIO22 22
#define ESP32_SAFE_GPIO23 23
#define ESP32_SAFE_GPIO25 25
#define ESP32_SAFE_GPIO26 26
#define ESP32_SAFE_GPIO27 27

// RX2 and TX2 are generally safe for use
#define ESP32_UART2_RX 16
#define ESP32_UART2_TX 17

// LED pin definition (if applicable)
#define ESP32_LED_PIN 2 // On-board LED is usually on GPIO2

// ADC1 (trusted) pins
#define ESP32_ADC1_GPIO32 32
#define ESP32_ADC1_GPIO33 33
//  (input only)
#define ESP32_ADC1_GPIO34 34
#define ESP32_ADC1_GPIO35 35
#define ESP32_ADC1_GPIO36 36
#define ESP32_ADC1_GPIO39 39

// I2C default pins
#define ESP32_SDA ESP32_SAFE_GPIO21
#define ESP32_SCL ESP32_SAFE_GPIO22

#endif // CONFIG_IDF_TARGET_ESP32