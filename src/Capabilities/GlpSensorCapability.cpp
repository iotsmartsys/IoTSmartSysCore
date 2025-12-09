#include "GlpSensorCapability.h"
#include "Utils/Logger.h"

const int NUM_SAMPLES = 20; // Amostras para média
int cleanAirBase = 0;       // Valor médio em "ar limpo" (referência)
bool calibrated = false;

GlpSensorCapability::GlpSensorCapability(int pinAO, int pinDO)
    : Capability(GLP_SENSOR_TYPE, GLP_SENSOR_LEVEL_NONE), pinAO(pinAO), pinDO(pinDO)
{
    setCallback([this](String *state)
                {
                    LOG_INFO("");
                    LOG_INFO("Callback da " + this->capability_name + " chamado com o estado: " + *state); });
}

GlpSensorCapability::GlpSensorCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value)
{
}

void GlpSensorCapability::setup()
{
    levelPercent = 0.0;
    detected = false;

    pinMode(pinAO, INPUT);
    pinMode(pinDO, INPUT_PULLUP);

    LOG_PRINT(F("MQ-2 + ESP32 - Inicializando..."));
    LOG_PRINTLN(F("Aquecendo sensor (~20 segundos)..."));
    delay(20000); // para exemplo; o ideal seria bem mais tempo para estabilizar melhor

    LOG_PRINTLN(F("Calibrando em 'ar limpo' (rápido)..."));
    cleanAirBase = readAnalogAverage();
    calibrated = true;

    LOG_PRINT(F("Valor base (cleanAirBase): "));
    LOG_PRINTLN(cleanAirBase);

    LOG_PRINTLN(F("Iniciando leitura contínua..."));
}

void GlpSensorCapability::handle()
{
    int analogValue = readAnalogAverage();
    int doState = digitalRead(pinDO);

    // Converte para "nível relativo" em relação ao ar limpo
    // Se analogValue == cleanAirBase  => 0%
    // Se aumentar bastante, o valor relativo aumenta
    int relative = 0;
    if (calibrated && analogValue > cleanAirBase)
    {
        relative = analogValue - cleanAirBase;
    }

    // Você pode normalizar isso num "0–100%" se quiser
    // Aqui é só um exemplo simples:
    // assume que +2000 acima do base já é 100%
    float relativePercent = map(constrain(relative, 0, 2000), 0, 2000, 0, 100);
    levelPercent = relativePercent;

    // Limiar de alarme (ajuste na prática/testes)
    const int THRESHOLD_PERCENT = 40;

    bool gasAlert = (relativePercent >= THRESHOLD_PERCENT) || (doState == LOW);
    detected = gasAlert;

    if (relativePercent < 20)
    {
        levelState = GLP_SENSOR_LEVEL_NONE;
    }
    else if (relativePercent < 50)
    {
        levelState = GLP_SENSOR_LEVEL_LOW;
    }
    else if (relativePercent < 80)
    {
        levelState = GLP_SENSOR_LEVEL_MEDIUM;
    }
    else
    {
        levelState = GLP_SENSOR_LEVEL_HIGH;
    }

    if (lastLevel != levelState)
    {
        updateState(levelState);
        lastLevel = levelState;
    }

    // Log na serial
    LOG_PRINT(F("ADC: "));
    LOG_PRINT(analogValue);
    LOG_PRINT(F(" | base: "));
    LOG_PRINT(cleanAirBase);
    LOG_PRINT(F(" | rel(%): "));
    LOG_PRINT(relativePercent);
    LOG_PRINT(F(" | DO: "));
    LOG_PRINT(doState == LOW ? "ATIVADO" : "normal");
    LOG_PRINT(F(" | ALERTA: "));
    LOG_PRINTLN(gasAlert ? "SIM" : "nao");
    LOG_PRINTLN("Level: " + levelState);
}

bool GlpSensorCapability::isDetected()
{
    return detected;
}

float GlpSensorCapability::getLevelPercent()
{
    return levelPercent;
}

String GlpSensorCapability::getLevelString()
{
    return levelState;
}

// Lê várias vezes o ADC e faz média simples
int GlpSensorCapability::readAnalogAverage()
{
    long sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        int val = analogRead(pinAO);
        sum += val;
        delay(10);
    }
    return (int)(sum / NUM_SAMPLES);
}