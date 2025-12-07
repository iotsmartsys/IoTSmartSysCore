#pragma once

#include <Arduino.h>

void setACMode(String mode, uint8_t temp = 0);

String getACMode(uint64_t code);