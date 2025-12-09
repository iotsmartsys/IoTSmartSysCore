#include <Arduino.h>
#include "IoTSmartSysCore.h"
#include "Capabilities/GlpSensorCapability.h"
 
#define LED_BUILTIN 2
GlpSensorCapability *glpSensor = new GlpSensorCapability(34, 27); // pinos AO e DO
void setup()
{
    Serial.begin(115200);
    Serial.println("Iniciando exemplo de sensor de gás GLP (MQ-2)");
    pinMode(LED_BUILTIN, OUTPUT);

    glpSensor->setup();
}

void loop()
{

    glpSensor->handle();

    Serial.print("Nível de GLP : ");
    Serial.print(glpSensor->getLevelPercent());
    Serial.println("%");
    Serial.print("Estado do sensor: ");
    Serial.println(glpSensor->getLevelString());
    Serial.print("-----------------------");

    if (glpSensor->isDetected())
    {
        digitalWrite(LED_BUILTIN, HIGH); // Acende LED se gás detectado
    }
    else
    {
        digitalWrite(LED_BUILTIN, LOW); // Apaga LED se tudo ok
    }

    delay(1000);
}
