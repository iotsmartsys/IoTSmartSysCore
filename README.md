# IoTSmartSysCore - Core IoT para ESP32 (Arduino/PlatformIO)

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

## Transporte BLE de comandos (V1 / especificação 0.2)

O controle BLE é opt-in, separado do BLE de provisioning. Ative
`IOTSMARTSYS_BLE_COMMAND_ENABLED=1` em um firmware Arduino/ESP32 com Bluedroid
BLE. O environment `esp32_dev_ble_control` compila essa integração sem incluir
chave e sem habilitá-la em runtime por padrão. O build verificado é ESP32
clássico; outros SoCs/stacks precisam de qualificação própria.

Antes de `SmartSysApp::setup()`, forneça `BluetoothControlConfig` a
`configureBluetoothControl(config)`:

- `sharedSecret` e `sharedSecretSize`: buffer privado de exatamente 32 bytes,
  com a chave aleatória distribuída ao app e aos dispositivos; a API copia o
  buffer e não depende do lifetime do chamador;
- `localControlsConfigured = true`: declaração de que o firmware integra os
  eventos físicos distintos de admissão e revogação;
- `deviceInfoEnabled`: `true` por padrão, opcional;
- prazos em milissegundos: `pairingWindowMs`, `securityTimeoutMs`,
  `authTimeoutMs`, `receiveTimeoutMs`, `sessionTimeoutMs` e
  `disconnectTimeoutMs`. Todos devem ser positivos e menores que `2^31`.

A configuração retorna `false` quando a feature está desabilitada, após setup
ou quando os parâmetros são inválidos. Não use PIN numérico, senha de rede ou
chave de demonstração. O repositório não fornece nem gera a chave privada.

Após setup, traduza os eventos físicos do firmware em `openPairingWindow()` e
`revokeBleBonds()`. Essas chamadas solicitam trabalho cooperativo e retornam
`Pending` quando aceitas; acompanhe `bluetoothControlResult()`. O gesto de
factory reset permanece independente. Consulte `bluetoothControlState()`,
`bluetoothPairingWindowOpen()` e `bluetoothHasAuthorizedPeer()` para indicação
local. A janela inicia fechada, dura 60 s e não substitui uma central já
vinculada (`AlreadyBound`); revogue antes de admitir outra.

O transporte anuncia o UUID Control no advertising e o `deviceId` ASCII
integral de 1–13 bytes em Service Data (`0x21`) no scan response. O app deve
aguardar Service Data durante o scan. Após conectar, habilita os CCCDs Auth e
Response, conclui bonding criptografado, lê o desafio de 17 bytes e envia o
HMAC de 32 bytes. Só após Auth positivo envia o JSON lógico existente seguido
de LF. Cada conexão aceita um comando de capability, de até 1024 bytes sem LF;
MTU 23 é suportado pelo framing. `{"ack":true}` confirma encaminhamento, sem
confirmar efeito físico. Não repita automaticamente após perda da resposta.

Control não funciona durante provisioning, não exige broker/Wi-Fi no ramo
operacional e usa o mesmo dispatcher de capabilities, no ciclo de transporte.
Uma stack BLE já inicializada por outro proprietário torna Control
indisponível; não há reinicialização nem erase global. A autorização fica no
namespace privado `ble_control`, separada das chaves de bonding da stack.
Falhas de armazenamento bloqueiam Control até recuperação por reinicialização;
uma revogação que falhou não pode ser anunciada como durável.

O segredo comum não impede extração de app/firmware, e Just Works não promete
proteção contra MITM/relay. Capturas, interoperabilidade Swift, funcionamento
físico e falhas de energia ainda dependem das validações de aceite descritas em
[BLE-COMMAND-TRANSPORT.md](docs/specs/BLE-COMMAND-TRANSPORT.md).
