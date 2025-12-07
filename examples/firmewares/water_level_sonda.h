#include <Arduino.h>
#include "Core/IoTCore.h"
#include "Utils/Logger.h"
#include "Builders/CapabilityBuilder.h"
#include "Sensors/Modules/SondaWaterLevelSensor.h"

CollectionCapabilityBuilder *builder = new CollectionCapabilityBuilder();

std::vector<Capability *> capabilities;
IoTCore *iotCore;

const int pins[] = {4, 5, 6, 7, 15};
const int percents_equivalent[] = {10, 40, 60, 80, 100};
SondaWaterLevelSensor waterLevelSensor(pins, percents_equivalent, 5, 1000);
void setup()
{
    LOG_BEGIN(115200);
    LOG_PRINTLN("Setup");
    builder->addWaterLevelPercentageCapability(&waterLevelSensor);
    iotCore = new IoTCore(builder->build());

    iotCore->setup();
    waterLevelSensor.setup();
}

void loop()
{
    waterLevelSensor.handle();
    LOG_PRINTLN("Nível de água: " + String(waterLevelSensor.getLevelPercent()) + "%");
    
    iotCore->handle();
}
