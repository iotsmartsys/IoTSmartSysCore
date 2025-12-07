
#include "IoTCore.h"
#include "Utils/Logger.h"

Capability arCondicionadoCapability("luminosidadeEscritorio", "Temperature Sensor", "-1");
Capability powerCapability("power", "Power", "off");
std::vector<Capability> capabilities = {arCondicionadoCapability, powerCapability};
std::vector<Property> properties;

IoTCore *iotCore = new IoTCore(capabilities);
float luminosidadeEscritorio = 0;
float lastLuminosidadeEscritorio = 0;

void define_value_luminosidade_aleatoria_a_cada_10_segundos();
void setup()
{
    LOG_BEGIN(115200);

    pinMode(2, OUTPUT);
    
    iotCore->setup();
}

void loop()
{
    define_value_luminosidade_aleatoria_a_cada_10_segundos();

    iotCore->handle();
}

void command_exec(CapabilityCommand command)
{
    LOG_PRINTLN("Command: " + command.command + " Value: " + command.value);
    if (command.command == "power")
    {
        command.value.toLowerCase();
        if (command.value == "on")
        {
            LOG_PRINTLN("Ligando a LED");
            digitalWrite(2, HIGH);
        }
        else
        {
            LOG_PRINTLN("Desligando a LED");
            digitalWrite(2, LOW);
        }
    }
}

long lastTime = 0;

void define_value_luminosidade_aleatoria_a_cada_10_segundos()
{
    if (millis() - lastTime < 10000)
    {
        return;
    }

    luminosidadeEscritorio = random(0, 100);
    lastTime = millis();
    if (luminosidadeEscritorio != lastLuminosidadeEscritorio)
    {
        lastLuminosidadeEscritorio = luminosidadeEscritorio;

        CapabilityState state("luminosidadeEscritorio", String(luminosidadeEscritorio));
        iotCore->sendState(state);
    }

    LOG_PRINTLN("Luminosidade do escritório: " + String(luminosidadeEscritorio));
}
