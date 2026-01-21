#pragma once

#ifdef ESP32
#include "sdkconfig.h"
#endif

#if CONFIG_IDF_TARGET_ESP32S3
#include "Platform/Espressif/Pinouts/ESP32_S3_Pinouts.h"
#elif CONFIG_IDF_TARGET_ESP32
#include "Platform/Espressif/Pinouts/ESP32_WROOM_32_Pinouts.h"
#elif CONFIG_IDF_TARGET_ESP32_C6
#include "Platform/Espressif/Pinouts/ESP32_C6_Pinouts.h"
#elif CONFIG_IDF_TARGET_ESP32_C3
#include "Platform/Espressif/Pinouts/ESP32_C3_Pinouts.h"
#elif ESP8266 || defined(ARDUINO_ARCH_ESP8266)
#include "Platform/Espressif/Pinouts/ESP8266_Pinouts.h"
#endif

// #include "Platform/Espressif/Pinouts/SmartSys_ESP32_S3_Dev_Board_Pinouts.h"

#if defined(IOTSMARTSYS_MCB01) && (IOTSMARTSYS_BOARD_REV == 1)
#include "Platform/Espressif/Pinouts/SmartSys_MCB01_Pinouts.h"
#else
// #error "Board IoTSmartSys nao suportada ou revisao desconhecida"
#endif
// #ifndef CONFIG_IDF_TARGET_ESP32S3 && LED_BUILTIN
// #define LED_BUILTIN ESP32_LED_BUILTIN
// #endif