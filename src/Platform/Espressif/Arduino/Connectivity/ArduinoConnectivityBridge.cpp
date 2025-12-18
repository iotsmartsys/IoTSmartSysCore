#ifdef ARDUINO_ARCH_ESP32

#include "ArduinoConnectivityBridge.h"

#include <WiFi.h>
#include "Contracts/Connectivity/ConnectivityGate.h"

using Gate = iotsmartsys::core::ConnectivityGate;

namespace iotsmartsys::platform::espressif::arduino
{

    static bool s_started = false;

    static void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
    {
        (void)info;
        auto &gate = Gate::instance();

        switch (event)
        {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            gate.setBits(Gate::WIFI_CONNECTED);
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            gate.setBits(Gate::IP_READY);
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            // Quando cai o Wi-Fi, rede e MQTT deixam de ser válidos
            gate.clearBits(Gate::WIFI_CONNECTED | Gate::IP_READY | Gate::MQTT_CONNECTED);
            break;

        // Nem todas as versões expõem esse evento; se não compilar, remova.
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            gate.clearBits(Gate::IP_READY | Gate::MQTT_CONNECTED);
            break;

        default:
            break;
        }
    }

    void ArduinoConnectivityBridge::start()
    {
        if (s_started)
            return;
        s_started = true;

        WiFi.onEvent(onWiFiEvent);
    }

} // namespace iotsmartsys::platform::espressif::arduino

#endif // ARDUINO_ARCH_ESP32