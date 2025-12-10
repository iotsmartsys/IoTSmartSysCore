
#include "Core/IoTCore.h"

#ifdef ESP_NOW_ENABLED
#include "esp_now_utils/esp_now_utils.h"
#include "Capabilities/CapabilityState.h"

void sendStateEspNow(IoTCore &iotCore, DeviceAnnouncement announcement)
{
    iotCore.sendDeviceIncoming(announcement);
}
#endif 