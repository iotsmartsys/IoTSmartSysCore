#if defined(IOTSMARTSYS_DEFAULT_RUNTIME_APP) && !defined(APP_EXAMPLE_RUNNER)

#include <Arduino.h>

#include "SmartSysApp.h"

namespace
{
    iotsmartsys::SmartSysApp app;
}

void setup()
{
    Serial.begin(115200);
    app.setup();
}

void loop()
{
    app.handle();
}

#endif
