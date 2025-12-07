#include <Arduino.h>
#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#include "display/Display_ST7789_170_320.h"
// pins:
const int HX711_dout = 19; // mcu > HX711 dout pin
const int HX711_sck = 5;   // mcu > HX711 sck pin

// HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_calVal_eepromAdress = 0;
unsigned long t = 0;

void setup()
{
    Serial.begin(115200);
    setup_ST7789_170x320();
    delay(10);
    Serial.println();
    Serial.println("Starting...");

    LoadCell.begin();
    LoadCell.setReverseOutput();
    unsigned long stabilizingtime = 2000;
    boolean _tare = true;
    LoadCell.start(stabilizingtime, _tare);
    if (LoadCell.getTareTimeoutFlag())
    {
        Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    }
    else
    {
        Serial.println("Tare OK. Aguarde estabilidade...");
    }

    // --- Calibração automática com peso conhecido ---
    const float knownMassKg = 5.70f; // ajuste aqui se usar outro peso conhecido

    // Para a rotina de calibração, comece com fator 47.017544
    LoadCell.setCalFactor(47.017544f);
    Serial.print("Iniciando calibracao com massa conhecida: ");
    Serial.print(knownMassKg, 2);
    Serial.println(" kg");

    // Garante que temos um conjunto de dados estável com o peso já colocado
    LoadCell.refreshDataSet(); // bloqueante, preenche o dataset

    // Calcula novo fator de calibracao baseado na massa conhecida
    float newCalFactor = LoadCell.getNewCalibration(knownMassKg);
    Serial.print("Novo CalFactor calculado: ");
    Serial.println(newCalFactor, 6);

    // Aplica o novo fator de calibracao
    LoadCell.setCalFactor(newCalFactor);
    Serial.println("Calibracao concluida. CalFactor aplicado.");

    //   while (!LoadCell.update());
    //   Serial.print("Calibration value: ");
    //   Serial.println(LoadCell.getCalFactor());
    //   Serial.print("HX711 measured conversion time ms: ");
    //   Serial.println(LoadCell.getConversionTime());
    //   Serial.print("HX711 measured sampling rate HZ: ");
    //   Serial.println(LoadCell.getSPS());
    //   Serial.print("HX711 measured settlingtime ms: ");
    //   Serial.println(LoadCell.getSettlingTime());
    //   Serial.println("Note that the settling time may increase significantly if you use delay() in your sketch!");
    //   if (LoadCell.getSPS() < 7) {
    //     Serial.println("!!Sampling rate is lower than specification, check MCU>HX711 wiring and pin designations");
    //   }
    //   else if (LoadCell.getSPS() > 100) {
    //     Serial.println("!!Sampling rate is higher than specification, check MCU>HX711 wiring and pin designations");
    //   }
}

void loop()
{
    static bool firstSample = true;
    static float pesoKgFiltrado = 0.0f;
    const float alpha = 0.9f;           // fator do filtro EMA (0..1, mais perto de 1 = mais suave)
    const unsigned long printInterval = 500; // ms
    static unsigned long lastPrint = 0;

    if (LoadCell.update())
    {
        // Com o CalFactor já ajustado, o valor retornado JÁ está em kg
        float pesoKg = LoadCell.getData();

        // Filtro exponencial para estabilizar
        if (firstSample)
        {
            pesoKgFiltrado = pesoKg;
            firstSample = false;
        }
        else
        {
            pesoKgFiltrado = alpha * pesoKgFiltrado + (1.0f - alpha) * pesoKg;
        }

        // Imprime no Serial a cada intervalo definido
        unsigned long now = millis();
        if (now - lastPrint >= printInterval)
        {
            lastPrint = now;
            Serial.print("Peso (kg): ");
            Serial.println(pesoKgFiltrado, 3); // 3 casas decimais
            displayMessage("Peso bruto (kg): " + String(pesoKg, 3));
            displayMessage("Peso (kg): " + String(pesoKgFiltrado, 3));
        }

        // Aqui você poderá, no futuro, publicar `pesoKgFiltrado` via MQTT
        // ou repassar para o seu sistema IoTPrivateHome.
    }
}