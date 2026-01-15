#pragma once

#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32C6

// Safe GPIO pins for ESP32-C6 module
#define ESP32_GPIO04 4
#define ESP32_GPIO05 5

// RX and TX
#define ESP32_UART0_RX ESP32_GPIO04
#define ESP32_UART0_TX ESP32_GPIO05

#endif // CONFIG_IDF_TARGET_ESP32C6