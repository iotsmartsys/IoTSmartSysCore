#pragma once

#if defined(NODE_BLE_ENABLED)

#include <Arduino.h>
#include "pins.h"
#include "Transports/IMessageClient.h"

void setupBLEGateway();
void handleBLEGateway(IMessageClient *transport);
void notifyBLEGateway(byte *payload);
bool isBLEConnected();

#endif 
