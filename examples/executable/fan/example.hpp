#pragma once

#include <Arduino.h>
#include "SmartSysApp.h"

#ifndef ITS_MCB01_RELAY_PIN
#error "fan requires the MCB R1 pinout symbol ITS_MCB01_RELAY_PIN"
#endif

#ifndef EXAMPLE_FAN_ACTIVE_HIGH
#error "fan requires EXAMPLE_FAN_ACTIVE_HIGH to be defined as 0 or 1"
#endif

#ifndef EXAMPLE_BOARD_ID
#error "fan requires EXAMPLE_BOARD_ID to be defined by the board profile"
#endif

namespace executable_example
{
    static iotsmartsys::SmartSysApp app;
    static iotsmartsys::core::FanCapability *fan = nullptr;
}

void setup()
{
    Serial.begin(115200);
    Serial.printf("[example] id=fan board=%s fan_pin=%d active_high=%d capability=fan\n",
                  EXAMPLE_BOARD_ID,
                  ITS_MCB01_RELAY_PIN,
                  EXAMPLE_FAN_ACTIVE_HIGH);

    iotsmartsys::app::FanConfig fanConfig{
        ITS_MCB01_RELAY_PIN,
        EXAMPLE_FAN_ACTIVE_HIGH != 0,
        "fan"};
    executable_example::fan = executable_example::app.addFanCapability(fanConfig);
    if (executable_example::fan == nullptr)
        Serial.println("[example] addFanCapability failed; see the runtime log for the cause");

    executable_example::app.setup();
}

void loop()
{
    executable_example::app.handle();
}
