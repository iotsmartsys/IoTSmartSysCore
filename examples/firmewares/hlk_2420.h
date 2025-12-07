#include <Arduino.h>
#include <ld2410.h>

ld2410 radar;

void tryAt(uint32_t baud) {
  Serial.printf("Tentando baud %lu...\n", baud);
  Serial2.begin(baud, SERIAL_8N1, 16 /*RX2*/, 17 /*TX2*/);
  if (radar.begin(Serial2, true)) {
    Serial.printf("OK em %lu\n", baud);
    radar.requestFirmwareVersion();
  } else {
    Serial.printf("Falhou em %lu\n", baud);
    Serial2.end();
    delay(50);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  tryAt(256000);
  if (!radar.presenceDetected()) {
    tryAt(115200);
  }
}

void loop() {
  radar.read();
  static uint32_t last = 0;
  if (millis() - last > 500) {
    last = millis();
    Serial.printf("presence=%d moving=%d stationary=%d dM=%ucm E=%u dS=%ucm E=%u\n",
      radar.presenceDetected(),
      radar.movingTargetDetected(),
      radar.stationaryTargetDetected(),
      radar.movingTargetDistance(),
      radar.movingTargetEnergy(),
      radar.stationaryTargetDistance(),
      radar.stationaryTargetEnergy());
  }
}