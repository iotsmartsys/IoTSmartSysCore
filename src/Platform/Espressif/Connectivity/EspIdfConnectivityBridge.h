// Only compile ESP-IDF connectivity bridge on ESP32 targets
#if defined(ESP32)

#pragma once

#include "Contracts/Connectivity/ConnectivityGate.h"
#include "esp_event.h"

namespace iotsmartsys::platform::espressif
{

  class EspIdfConnectivityBridge
  {
  public:
    // Requer: esp_event_loop_create_default() já exista ou será criado aqui
    static esp_err_t start();

    // opcional: se você quiser parar/desregistrar depois
    static esp_err_t stop();

  private:
    static void wifiHandler(void *arg, esp_event_base_t base, int32_t id, void *data);
    static void ipHandler(void *arg, esp_event_base_t base, int32_t id, void *data);

    static bool _started;
  };

} // namespace iotsmartsys::platform::espressif

#endif // ESP32