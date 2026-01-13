# IoTSmartSysCore - Core IoT para ESP32/ESP8266 (Arduino/PlatformIO)

Biblioteca core da plataforma IoTSmartSys para dispositivos IoT. Fornece o `SmartSysApp`, que orquestra WiFi, MQTT, SettingsManager (cache NVS + API), provisioning e OTA (opcional), alem de um conjunto de capabilities para sensores/atuadores.

## Principais recursos

- `SmartSysApp` como ponto central do runtime (setup/handle) e builder de capabilities.
- MQTT via `EspIdfMqttClient`, com topics configuraveis por settings.
- SettingsManager com cache em NVS + fetch da API e aplicacao automatica.
- Provisioning com canais opcionais: Web Portal e BLE.
- OTA opcional via `OTAManager` (habilitado por flag).
- TransportHub com canal MQTT e opcionalmente serial (UART).
- Sensores e utilitarios (DHT, DS18B20, BH1750, HC-SR04, GLP, display ST7789).

## Instalacao (PlatformIO)

Este repo ja inclui `platformio.ini` e configs em `configs/*.ini`. As dependencias sao resolvidas pelo PlatformIO via `library.json`.

Exemplo de flags em `platformio.ini` (ajuste conforme seu alvo):

```
build_flags =
  -DWEB_PORTAL_PROVISIONING_CHANNEL_ENABLE=1
  -DWEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE=0
  ; -DBLE_PROVISIONING_CHANNEL_ENABLE=1
  ; -DOTA_DISABLED=1
  ; -DBH1750_ENABLED
  ; -DST7789_170x320_ENABLED
```

Observacao: `platformio.ini` carrega `configs/*.ini` e `private.ini` via `extra_configs`.

## Uso basico

```
#include <Arduino.h>
#include "SmartSysApp.h"

static iotsmartsys::SmartSysApp app;

void setup() {
  iotsmartsys::app::LightConfig light{};
  light.capability_name = "luz_sala";
  light.GPIO = PIN_TEST;
  light.highIsOn = false;
  app.addLightCapability(light);

  app.setup();
}

void loop() {
  app.handle();
}
```

Exemplos adicionais em `examples/basic_usage`, `examples/composite` e `examples/configuration_portal`.

## Capabilities expostas pelo SmartSysApp

- Alarm, DoorSensor, ClapSensor
- Light, LED, Switch, Valve
- PirSensor, PushButton, TouchButton
- TemperatureSensor, HumiditySensor, Luminosity
- WaterLevelPercent, WaterLevelLiters, HeightWaterLevel
- GlpSensor, GlpMeterPercent, GlpMeterKg
- OperationalColorSensor

As configs vivem em `src/App/Builders/Configs/HardwareConfig.h` e algumas capabilities exigem instancias de sensores (ex.: `IWaterLevelSensor`, `IGlpSensor`, `ITemperatureSensor`).

## MQTT e topics

Os topics padrao sao configurados em `MqttSettings`:

- `smarthome/discovery` (announce)
- `device/{device_id}/command` (command)
- `device/state` (notify)

O `SmartSysApp` publica o announce ao conectar e assina o topic de comando com o `clientId` vindo dos settings.

## Provisioning

Quando nao ha settings validas (ou `in_config_mode`), o app entra em modo de configuracao e sobe os canais habilitados:

- Web Portal (opcional, com suporte a captive portal via `WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE`)
- BLE (opcional, via `BLE_PROVISIONING_CHANNEL_ENABLE`)

## Transportes

- MQTT e o transporte principal.
- UART serial opcional via `configureSerialTransport()` e `SerialTransportChannel`.

## Estrutura do projeto

- `src/SmartSysApp.*`: entrypoint do runtime e builder de capabilities.
- `src/App`: builders, configuracoes e servicos de app.
- `src/Core`: capabilities, settings, provisioning, comandos, transportes.
- `src/Platform`: integracoes Arduino/Espressif (MQTT, provisioning, NVS).
- `src/Infra`: OTA, factories de sensores e display ST7789.
- `configs/*.ini`: ambientes base do PlatformIO.
- `platformio.ini`: entry dos ambientes + `private.ini`.

## Licenca

MIT — ver `library.json`.
