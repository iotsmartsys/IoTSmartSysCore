// Central compile unit for platform-specific implementations.

#include "Platform/Providers/PlatformServiceRegistrar.cppinc"

#if defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)

#include "Platform/Arduino/Logging/ArduinoSerialLogger.cppinc"
#include "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.cppinc"
#include "Platform/Arduino/Provisioning/ProvisioningManager.cppinc"
#include "Platform/Arduino/Provisioning/WebPortalProvisioningChannel.cppinc"
#include "Platform/Arduino/Transports/ArduinoSerialTransportChannel.cppinc"
#include "Platform/Arduino/Firmware/ArduinoFirmwareRuntimeInfo.cppinc"
#include "Platform/Arduino/Interpreters/PhilcoAirConditionerInterpreter.cppinc"
#include "Platform/Arduino/Interpreters/ValveHardwareCommandInterpreter.cppinc"
#include "Platform/Arduino/Sensors/DHTSensor.cppinc"
#include "Platform/Arduino/Sensors/DS18B20TemperatureSensor.cppinc"
#include "Platform/Arduino/Sensors/SensorUltrassonicHCSR04.cppinc"
#include "Platform/Arduino/Sensors/ArduinoGlpSensor.cppinc"
#include "Platform/Arduino/Sensors/ArduinoIRCommandSensor.cppinc"
#include "Platform/Arduino/Sensors/Bh1750LuminositySensor.cppinc"
#include "Platform/Arduino/Sensors/ArduinoGlpMeter.cppinc"
#include "Platform/Arduino/Sensors/ArduinoUltrassonicWaterLevelSensor.cppinc"

#include "Platform/Arduino/System/Esp8266SystemControl.cppinc"
#include "Platform/Arduino/Providers/Esp8266RandomProvider.cppinc"
#include "Platform/Arduino/Providers/Esp8266PlatformServiceRegistrar.cppinc"

#elif defined(ARDUINO_ARCH_ESP32) || (defined(ESP32) && !defined(ESP8266))

#include "Platform/Arduino/Logging/ArduinoSerialLogger.cppinc"
#include "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.cppinc"
#include "Platform/Arduino/Provisioning/ProvisioningManager.cppinc"
#include "Platform/Arduino/Provisioning/WebPortalProvisioningChannel.cppinc"
#include "Platform/Arduino/Transports/ArduinoSerialTransportChannel.cppinc"
#include "Platform/Arduino/Firmware/ArduinoFirmwareRuntimeInfo.cppinc"
#include "Platform/Arduino/Interpreters/PhilcoAirConditionerInterpreter.cppinc"
#include "Platform/Arduino/Interpreters/ValveHardwareCommandInterpreter.cppinc"
#include "Platform/Arduino/Sensors/DHTSensor.cppinc"
#include "Platform/Arduino/Sensors/DS18B20TemperatureSensor.cppinc"
#include "Platform/Arduino/Sensors/SensorUltrassonicHCSR04.cppinc"
#include "Platform/Arduino/Sensors/ArduinoGlpSensor.cppinc"
#include "Platform/Arduino/Sensors/ArduinoIRCommandSensor.cppinc"
#include "Platform/Arduino/Sensors/Bh1750LuminositySensor.cppinc"
#include "Platform/Arduino/Sensors/ArduinoGlpMeter.cppinc"
#include "Platform/Arduino/Sensors/ArduinoUltrassonicWaterLevelSensor.cppinc"

#include "Platform/Espressif/Arduino/Connectivity/ArduinoConnectivityBridge.cppinc"
#include "Platform/Espressif/Provisioning/BleProvisioningChannel.cppinc"
#include "Platform/Espressif/Mqtt/EspIdfMqttClient.cppinc"
#include "Platform/Espressif/System/EspressifSystemControl.cppinc"
#include "Platform/Espressif/Providers/EspressifPlatformServiceRegistrar.cppinc"
#include "Platform/Espressif/Providers/EspressifRandomProvider.cppinc"
#include "Platform/Espressif/Providers/DeviceIdentityProvider.cppinc"
#include "Platform/Espressif/Connectivity/EspIdfMqttBridge.cppinc"
#include "Platform/Espressif/Connectivity/EspIdfConnectivityBridge.cppinc"
#include "Platform/Espressif/Parsers/EspIdfCommandParser.cppinc"
#include "Platform/Espressif/Parsers/EspIdFirmwareManifestParser.cppinc"
#include "Platform/Espressif/Firmware/EspIdfFirmwareRuntimeInfo.cppinc"
#include "Platform/Espressif/Settings/EspIdfSettingsFetcher.cppinc"
#include "Platform/Espressif/Settings/EspIdfSettingsParser.cppinc"
#include "Platform/Espressif/Settings/Providers/EspIdfNvsSettingsProvider.cppinc"

#else
#error "Unsupported platform: expected ARDUINO_ARCH_ESP8266/ESP8266 or ARDUINO_ARCH_ESP32/ESP32"
#endif
