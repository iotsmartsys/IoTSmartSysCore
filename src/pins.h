#pragma once

#include "Platform/Espressif/Pinouts/ESP32_S3_Pinouts.h"
#include "Platform/Espressif/Pinouts/ESP32_WROOM_32_Pinouts.h"
#include "Platform/Espressif/Pinouts/ESP32_C6_Pinouts.h"
#include "Platform/Espressif/Pinouts/ESP32_C3_Pinouts.h"
#include "Platform/Espressif/Pinouts/ESP8266_Pinouts.h"
#include "Platform/Espressif/Pinouts/SmartSys_ESP32_S3_Dev_Board_Pinouts.h"

#if defined(IOTSMARTSYS_MCB01) && (IOTSMARTSYS_BOARD_REV == 1)
#include "Platform/Espressif/Pinouts/SmartSys_MCB01_Pinouts.h"
#else
// #error "Board IoTSmartSys nao suportada ou revisao desconhecida"
#endif
#ifndef LED_BUILTIN
#define LED_BUILTIN ESP32_LED_BUILTIN
#endif