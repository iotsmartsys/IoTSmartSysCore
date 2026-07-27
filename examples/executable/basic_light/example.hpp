#pragma once

#include <Arduino.h>
#include "SmartSysApp.h"

#ifndef ITS_MCB01_RELAY_PIN
#error "basic_light requires the MCB R1 pinout symbol ITS_MCB01_RELAY_PIN"
#endif

#ifndef EXAMPLE_LIGHT_ACTIVE_HIGH
#error "basic_light requires EXAMPLE_LIGHT_ACTIVE_HIGH to be defined as 0 or 1"
#endif

#ifndef EXAMPLE_BOARD_ID
#error "basic_light requires EXAMPLE_BOARD_ID to be defined by the board profile"
#endif

namespace executable_example
{
    static iotsmartsys::SmartSysApp app;
}

void setup()
{
    Serial.begin(115200);
    Serial.printf("[example] id=basic_light board=%s light_pin=%d active_high=%d\n",
                  EXAMPLE_BOARD_ID, ITS_MCB01_RELAY_PIN, EXAMPLE_LIGHT_ACTIVE_HIGH);

    iotsmartsys::app::LightConfig lightConfig{
        ITS_MCB01_RELAY_PIN,
        EXAMPLE_LIGHT_ACTIVE_HIGH != 0,
        "basic_light"};
    executable_example::app.addLightCapability(lightConfig);

    executable_example::app.configureFactoryResetButton(iotsmartsys::app::PushButtonConfig{
        ITS_MCB01_BOOT_PIN});
    executable_example::app.setup();
}

void loop()
{
    executable_example::app.handle();
}
