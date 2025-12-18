#pragma once

#ifdef ARDUINO_ARCH_ESP32

namespace iotsmartsys::platform::espressif::arduino
{

    class ArduinoConnectivityBridge
    {
    public:
        // Registra WiFi.onEvent(...) e passa a atualizar o ConnectivityGate
        static void start();
    };

} // namespace iotsmartsys::platform::espressif::arduino

#endif // ARDUINO_ARCH_ESP32