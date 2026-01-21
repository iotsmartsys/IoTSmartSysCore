#include <cmath>

#include "ArduinoGlpSensor.h"

namespace iotsmartsys::platform::arduino
{
    ArduinoGlpSensor::ArduinoGlpSensor(int pinAO, int pinDO, ILogger &logger) : pinAO(pinAO), pinDO(pinDO), _logger(logger), detected(false), levelPercent(0.0f)
    {
    }

    void ArduinoGlpSensor::setup()
    {
        levelPercent = 0.0;
        detected = false;

        pinMode(pinAO, INPUT);
        pinMode(pinDO, INPUT_PULLUP);

        _logger.debug("ArduinoGlpSensor", "MQ-2 + ESP32 - Inicializando...");
        _logger.debug("ArduinoGlpSensor", "Aquecendo sensor (~20 segundos)...");
        delay(20000); // para exemplo; o ideal seria bem mais tempo para estabilizar melhor

        _logger.debug("ArduinoGlpSensor", "Calibrando em 'ar limpo' (rápido)...");
        cleanAirBase = readAnalogAverage();
        calibrated = true;

        _logger.debug("ArduinoGlpSensor", "Valor base (cleanAirBase): %d", cleanAirBase);
    }

    void ArduinoGlpSensor::handle()
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
            levelState = GlpSensorLevelStrings::UNDETECTED;
        }
        else if (relativePercent < 50)
        {
            levelState = GlpSensorLevelStrings::GLP_SENSOR_LEVEL_LOW;
        }
        else if (relativePercent < 80)
        {
            levelState = GlpSensorLevelStrings::GLP_SENSOR_LEVEL_MEDIUM;
        }
        else
        {
            levelState = GlpSensorLevelStrings::GLP_SENSOR_LEVEL_HIGH;
        }

        bool stateChanged = false;
        if (lastLevel != levelState)
        {
            stateChanged = true;
            lastLevel = levelState;
        }

        if (lastReportedPercent < 0.0f || fabs(levelPercent - lastReportedPercent) > 0.01f)
        {
            stateChanged = true;
            lastReportedPercent = levelPercent;
        }

        if (lastReportedDetected != detected)
        {
            stateChanged = true;
            lastReportedDetected = detected;
        }

        if (stateChanged)
        {
            lastStateReadMillis_ = millis();
        }

        // Log na serial
        _logger.warn("ArduinoGlpSensor", "ADC: %d, DO: %s, Nível GLP: %.2f%% (%s)", analogValue, (doState == LOW) ? "ALERTA" : "NORMAL", levelPercent, levelState.c_str());
    }

    float ArduinoGlpSensor::getLevelPercent()
    {
        return levelPercent;
    }

    bool ArduinoGlpSensor::isDetected()
    {
        return detected;
    }

    std::string ArduinoGlpSensor::getLevelString()
    {
        return levelState;
    }

    long ArduinoGlpSensor::lastStateReadMillis() const
    {
        return lastStateReadMillis_;
    }

    int ArduinoGlpSensor::readAnalogAverage()
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
} // namespace iotsmartsys::platform::arduino
