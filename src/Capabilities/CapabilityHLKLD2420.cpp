#include "CapabilityHLKLD2420.h"
/*
CapabilityHLKLD2420::CapabilityHLKLD2420(HardwareSerial &uart, int8_t rxPin, int8_t txPin, int8_t gpioOutPin)
: _uart(uart), _rxPin(rxPin), _txPin(txPin), _gpioOutPin(gpioOutPin) {}

bool CapabilityHLKLD2420::begin(uint32_t tryBaud1, uint32_t tryBaud2, bool waitForRadar) {
  _uart.begin(tryBaud1, SERIAL_8N1, _rxPin, _txPin);
  if (tryInitAtBaud(tryBaud1, waitForRadar)) {
    _r.baud = tryBaud1;
  } else {
    _uart.end();
    delay(10);
    _uart.begin(tryBaud2, SERIAL_8N1, _rxPin, _txPin);
    if (!tryInitAtBaud(tryBaud2, waitForRadar)) {
      LD2420_LOG("[LD2420] Falha ao iniciar em %lu e %lu\\n", tryBaud1, tryBaud2);
      return false;
    }
    _r.baud = tryBaud2;
  }

  if (_gpioOutPin >= 0)
    pinMode(_gpioOutPin, INPUT_PULLDOWN);

  LD2420_LOG("[LD2420] Iniciado baud %lu\\n", _r.baud);
  return true;
}

bool CapabilityHLKLD2420::tryInitAtBaud(uint32_t baud, bool waitForRadar) {
  bool ok = _radar.begin(_uart, waitForRadar);
  if (ok) {
    _r.connected = true;
    _radar.requestFirmwareVersion();
  }
  return ok;
}

void CapabilityHLKLD2420::loop() {
  _radar.read();

  _r.presence   = _radar.presenceDetected();
  _r.moving     = _radar.movingTargetDetected();
  _r.stationary = _radar.stationaryTargetDetected();
  _r.distMovingCm      = _radar.movingTargetDistance();
  _r.distStationaryCm  = _radar.stationaryTargetDistance();
  _r.energyMoving      = _radar.movingTargetEnergy();
  _r.energyStationary  = _radar.stationaryTargetEnergy();
  _r.lastUpdateMs      = millis();

  pollGpioOut();
  publishIfChanged();
}

void CapabilityHLKLD2420::pollGpioOut() {
  if (_gpioOutPin < 0) return;
  bool pinHigh = digitalRead(_gpioOutPin);
  if (pinHigh && !_r.presence) _r.presence = true;
  if (!pinHigh && _r.presence) _r.presence = false;
}

void CapabilityHLKLD2420::publishIfChanged() {
  if (!_onUpdate) return;
  if (memcmp(&_r, &_lastPublished, sizeof(Readings)) != 0) {
    _onUpdate(_r);
    _lastPublished = _r;
  }
}

bool CapabilityHLKLD2420::setMaxGates(uint8_t movingGate, uint8_t stationaryGate, uint16_t inactivityMs) {
  return _radar.setMaxValues(movingGate, stationaryGate, inactivityMs);
}

bool CapabilityHLKLD2420::setGateSensitivity(uint8_t gate, uint8_t moving, uint8_t stationary) {
  return _radar.setGateSensitivityThreshold(gate, moving, stationary);
}
  */