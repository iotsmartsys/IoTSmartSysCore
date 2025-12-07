#include "IoTSmartSysCore.h"
#include "Utils/Logger.h"

Property &IoTCore::addProperty(const String &name, const String &value)
{
    Property *prop = new Property(name, value);
    properties.push_back(prop);
    return *prop;
}

void IoTCore::addDefaultProperties()
{
    addProperty("build_id", getBuildIdentifier());
    addProperty("version", IOT_PRIVATE_HOME_VERSION);
    addProperty("wifi_ssid", WiFi.SSID());
    addProperty("wifi_signal", String(WiFi.RSSI()));
}
