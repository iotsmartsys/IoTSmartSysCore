#include <Arduino.h>

#include "SmartSysApp.h"

static iotsmartsys::SmartSysApp app;

void setup()
{
    iotsmartsys::app::LightConfig lightCfg_43;
    lightCfg_43.capability_name = "luz_sala_43";
    lightCfg_43.GPIO = PIN_TEST;
    lightCfg_43.highIsOn = false;
    app.addLightCapability(lightCfg_43);

    iotsmartsys::app::LightConfig lightCfg_44;
    lightCfg_44.capability_name = "luz_sala_44";
    lightCfg_44.GPIO = PIN_TEST + 1;
    lightCfg_44.highIsOn = false;
    app.addLightCapability(lightCfg_44);

    app.setup();
}

void loop()
{
    app.handle();
}
