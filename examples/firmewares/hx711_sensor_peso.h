#include <Arduino.h>
#include <HX711.h>

#define DOUT 4
#define SCK  5

HX711 scale;

long offset = 0;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("Iniciando HX711...");
    scale.begin(DOUT, SCK);
    scale.set_gain(128);

    // Calcula um offset médio em repouso
    const int N = 50;
    long soma = 0;
    Serial.println("Calculando offset, nao encoste no sensor...");
    for (int i = 0; i < N; i++) {
        while (!scale.is_ready()) {
            // espera ficar pronto
        }
        soma += scale.read();
        delay(20);
    }
    offset = soma / N;
    Serial.print("Offset = ");
    Serial.println(offset);
    Serial.println("Agora aperte / coloque peso para ver a variacao.");
}

void loop() {
    if (scale.is_ready()) {
        const int N = 10;
        long soma = 0;
        for (int i = 0; i < N; i++) {
            soma += scale.read();
            delay(10);
        }
        long media = soma / N;

        long delta = media - offset; // diferença em relação ao zero
        Serial.print("Bruto: ");
        Serial.print(media);
        Serial.print("  Delta: ");
        Serial.println(delta);
    } else {
        Serial.println("HX711 nao pronto");
    }

    delay(200);
}