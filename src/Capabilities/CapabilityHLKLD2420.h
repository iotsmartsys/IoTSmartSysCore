#pragma once

/*#include <Arduino.h>
#include <ld2410.h>

#ifndef LD2420_LOG
  #define LD2420_LOG(...) do { Serial.printf(__VA_ARGS__); } while(0)
#endif

class CapabilityHLKLD2420 {
public:
  struct Readings {
    bool connected{false};
    bool presence{false};
    bool moving{false};
    bool stationary{false};
    uint16_t distMovingCm{0};
    uint16_t distStationaryCm{0};
    uint8_t energyMoving{0};
    uint8_t energyStationary{0};
    uint32_t lastUpdateMs{0};
    uint32_t baud{0};
  };

  using OnUpdate = std::function<void(const Readings&)>;

  CapabilityHLKLD2420(HardwareSerial &uart = Serial2,
                      int8_t rxPin = 16,
                      int8_t txPin = 17,
                      int8_t gpioOutPin = -1);

  bool begin(uint32_t tryBaud1 = 256000, uint32_t tryBaud2 = 115200, bool waitForRadar = true);
  void loop();

  // Acesso aos dados
  const Readings& data() const { return _r; }
  bool presence() const { return _r.presence; }

  void onUpdate(OnUpdate cb) { _onUpdate = std::move(cb); }

  // Tuning opcional
  bool setMaxGates(uint8_t movingGate, uint8_t stationaryGate, uint16_t inactivityMs);
  bool setGateSensitivity(uint8_t gate, uint8_t moving, uint8_t stationary);

private:
  bool tryInitAtBaud(uint32_t baud, bool waitForRadar);
  void pollGpioOut();
  void publishIfChanged();

  ld2410 _radar;
  HardwareSerial &_uart;
  int8_t _rxPin, _txPin, _gpioOutPin;
  Readings _r, _lastPublished;
  OnUpdate _onUpdate{};
};
*/