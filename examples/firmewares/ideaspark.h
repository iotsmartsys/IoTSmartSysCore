#include <Arduino.h>
#include "Core/IoTCore.h"
#ifdef ST7789_170x320_ENABLED
#include <Arduino.h>
#include <HX711.h>

#include "display/Display_ST7789_170_320.h"

#define DOUT 19
#define SCK  5

HX711 scale;

long offset =  -598025; // -599624 -600142
const float SCALE = 20380.0f;// Peso esperado apenas para testes de tempo de estabilizacao
const float EXPECTED_WEIGHT_KG = 5.20f;     // ajuste para o peso que voce colocou na balanca
const float WEIGHT_TOLERANCE_KG = 0.05f;    // faixa de +/- 50 g considerada "proximo" do peso correto

void setup() {
    Serial.begin(115200);
    setup_ST7789_170x320();
    delay(2000);

    displayMessage("Iniciando HX711...");
    scale.begin(DOUT, SCK);
    scale.set_gain(128);

    // Calcula um offset médio em repouso
    const int N = 50;
    long soma = 0;
    displayMessage("Calculando offset, nao encoste no sensor...");
    for (int i = 0; i < N; i++) {
        while (!scale.is_ready()) {
            // espera ficar pronto
        }
        soma += scale.read();
        delay(20);
    }
    offset = soma / N;
    displayMessage("Offset = ");
    displayMessage(String(offset));
    displayMessage("Agora aperte / coloque peso para ver a variacao.");
}

void loop() {
    // Estado do filtro e medicao
    static bool first = true;
    static float kgFiltrado = 0.0f;
    const float alpha = 0.9f;

    // Estado do cronometro de estabilizacao
    static bool timerStarted = false;
    static bool timeReported = false;
    static unsigned long startTimeMs = 0;

    if (scale.is_ready()) {
        const int N = 10;
        long soma = 0;
        for (int i = 0; i < N; i++) {
            soma += scale.read();
            delay(10);
        }
        long media = soma / N;
        long delta = media - offset;

        // Converte para kg
        float kg = (float)delta / SCALE;

        if (first) {
            kgFiltrado = kg;
            first = false;
        } else {
            kgFiltrado = alpha * kgFiltrado + (1.0f - alpha) * kg;
        }

        // Inicia o cronometro na primeira leitura valida
        if (!timerStarted) {
            timerStarted = true;
            startTimeMs = millis();
        }

        // Verifica quando o valor filtrado chega proximo do peso esperado
        if (timerStarted && !timeReported) {
            float diff = kgFiltrado - EXPECTED_WEIGHT_KG;
            if (diff < 0) diff = -diff;
            if (diff <= WEIGHT_TOLERANCE_KG) {
                unsigned long elapsed = millis() - startTimeMs;
                String msg = "Estab(ms): " + String(elapsed);
                displayMessage(msg);
                Serial.print("[INFO] Tempo para estabilizar: ");
                Serial.print(elapsed);
                Serial.println(" ms");
                timeReported = true;
            }
        }

        displayMessage("Bruto: " + String(media));
        displayMessage("Delta: " + String(delta));
        displayMessage("Kg: " + String(kgFiltrado, 3));
    }

    delay(200);
}

#endif
