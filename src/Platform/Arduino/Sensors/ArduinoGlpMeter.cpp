#include "ArduinoGlpMeter.h"

namespace iotsmartsys::platform::arduino
{
#define UPDATE_STATE_FACTOR 1.0f

#ifndef HX711_OFFSET
#define HX711_OFFSET -600142
#endif
#ifndef HX711_SCALE_FACTOR
#define HX711_SCALE_FACTOR 20380.0f
#endif

    ArduinoGlpMeter::ArduinoGlpMeter(int pinAO, ILogger &logger)
        : dout_pin(pinAO), _logger(logger), tare_weight_kg(5.0), weight_capacity_kg(15.0)
    {
        scale = new HX711();
    }

    void ArduinoGlpMeter::setup()
    {
        scale->begin(dout_pin, sck_pin);
        scale->set_gain(128);

        // Calcula um offset médio em repouso
        const int N = 50;
        long soma = 0;

        for (int i = 0; i < N; i++)
        {
            while (!scale->is_ready())
            {
                // espera ficar pronto
            }
            soma += scale->read();
            delay(20);
            _logger.info("ArduinoGlpMeter: Scale initialized and tared.");
        }
    }

    void ArduinoGlpMeter::handle()
    {

        static bool first = true;
        static float kgFiltrado = 0.0f;
        const float alpha = 0.9f;

        // Estado do cronometro de estabilizacao
        static bool timerStarted = false;
        static bool timeReported = false;
        static unsigned long startTimeMs = 0;

        if (scale->is_ready())
        {
            const int N = 10;
            long soma = 0;
            for (int i = 0; i < N; i++)
            {
                soma += scale->read();
                delay(10);
            }
            long media = soma / N;
            long delta = media - HX711_OFFSET;

            // Converte para kg
            float kg = (float)delta / HX711_SCALE_FACTOR;

            if (first)
            {
                kgFiltrado = kg;
                first = false;
            }
            else
            {
                kgFiltrado = alpha * kgFiltrado + (1.0f - alpha) * kg;
            }

            // Inicia o cronometro na primeira leitura valida
            if (!timerStarted)
            {
                timerStarted = true;
                startTimeMs = millis();
            }

            _logger.debug("ArduinoGlpMeter", "Raw reading: %ld", media);
            _logger.debug("ArduinoGlpMeter", "Delta: %ld", delta);
            _logger.debug("ArduinoGlpMeter", "Kg: %.3f", kgFiltrado);

            static float lastKgStable = 0.0f;
            static bool kgIsStable = false;
            const float STABILITY_THRESHOLD = 0.005f;

            if (fabs(kgFiltrado - lastKgStable) < STABILITY_THRESHOLD)
            {
                kgIsStable = true;
            }
            else
            {
                kgIsStable = false;
                lastKgStable = kgFiltrado;
            }

            if (!kgIsStable)
            {
                return;
            }

            actualKg = kgFiltrado - tare_weight_kg;
            actualPercent = actualKg / weight_capacity_kg * 100.0f;
        }

        // Só chama updateState quando a variação de peso for maior ou igual ao fator definido
        if (fabs(actualKg - lastKg) >= UPDATE_STATE_FACTOR)
        {
            lastKg = actualKg;
            lastPercent = actualPercent;
        }
    }

    float ArduinoGlpMeter::getKg() const
    {
        return actualKg;
    }

    float ArduinoGlpMeter::getPercent() const
    {
        return actualPercent;
    }

    std::string ArduinoGlpMeter::getLevelString() const
    {
        return std::to_string(actualPercent);
    }

} // namespace iotsmartsys::platform::arduino