#include <Arduino.h>
#include "../src/Utils/Logger.h"
#include "../src/Core/IoTCore.h"

CollectionCapabilityBuilder *builder = new CollectionCapabilityBuilder();

std::vector<Capability *> capabilities;
IoTCore *iotCore;

void setup()
{
    LOG_BEGIN(115200);
    LOG_PRINTLN("Setup");

    builder->addOperationalColorSensorCapability(0);
    iotCore = new IoTCore(builder->build());

    iotCore->setup();
}

void loop()
{
    iotCore->handle();
}
