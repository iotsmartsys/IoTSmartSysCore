#include "GlpMeterCapability.h"
#if defined(GLP_METER_ENABLED)
#define UPDATE_STATE_FACTOR 1.0f // Variação mínima em kg para chamar updateState

GlpMeterCapability::GlpMeterCapability(String capability_name, HX711 *scale, int dout_pin, int sck_pin, float tare_weight_kg, float weight_capacity_kg)
    : Capability(capability_name, GLP_METER_TYPE, "0"), scale(scale), dout_pin(dout_pin), sck_pin(sck_pin), tare_weight_kg(tare_weight_kg), weight_capacity_kg(weight_capacity_kg)
{
    this->scale = scale;
}

void GlpMeterCapability::setup()
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
    }

    LOG_PRINT("Offset = ");
    LOG_PRINT(String(HX711_OFFSET));
    LOG_PRINT("Agora aperte / coloque peso para ver a variacao.");
}

void GlpMeterCapability::handle()
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

        LOG_INFO("Bruto: " + String(media));
        LOG_INFO("Delta: " + String(delta));
        LOG_INFO("Kg: " + String(kgFiltrado, 3));

        static float lastKgStable = 0.0f;
        static bool kgIsStable = false;
        const float STABILITY_THRESHOLD = 0.005f;

        if (fabs(kgFiltrado - lastKgStable) < STABILITY_THRESHOLD) {
            kgIsStable = true;
        } else {
            kgIsStable = false;
            lastKgStable = kgFiltrado;
        }

        if (!kgIsStable) {
            return;
        }

        actualKg = kgFiltrado - tare_weight_kg;
        actualPercent = actualKg / weight_capacity_kg * 100.0f;
    }

    // Só chama updateState quando a variação de peso for maior ou igual ao fator definido
    if (fabs(actualKg - lastKg) >= UPDATE_STATE_FACTOR) {
        lastKg = actualKg;
        lastPercent = actualPercent;
        updateState(String(actualPercent, 2));
    }
}

float GlpMeterCapability::getKg()
{
    return lastKg;
}

float GlpMeterCapability::getPercent()
{
    return lastPercent;
}

#endif
