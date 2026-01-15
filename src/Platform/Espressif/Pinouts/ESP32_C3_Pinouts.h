#pragma once

/*Pin safes */

#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32C3

#define ESP32_SAFE_GPIO3 3
#define ESP32_SAFE_GPIO4 4
#define ESP32_SAFE_GPIO5 5
#define ESP32_SAFE_GPIO6 6
#define ESP32_SAFE_GPIO7 7
#define ESP32_SAFE_GPIO10 10

// I2C default pins
#define ESP32_SDA 8
#define ESP32_SCL 9

/* ADC1 (trusted) pins  */
#define ESP32_ADC1_GPIO0 0
#define ESP32_ADC1_GPIO1 1
#define ESP32_ADC1_GPIO2 2
#define ESP32_ADC1_GPIO3 ESP32_SAFE_GPIO3
#define ESP32_ADC1_GPIO4 ESP32_SAFE_GPIO4

#endif // CONFIG_IDF_TARGET_ESP32C3