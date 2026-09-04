#include <Arduino.h>

#include "SmartSysApp.h"

static iotsmartsys::SmartSysApp app;

void setup()
{
    Serial.begin(115200);
    app.setup();
}

void loop()
{
    app.handle();
}
