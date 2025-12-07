# IoTPrivateHome — ESP32/ESP8266 com IoTCore

Biblioteca C++ para Arduino/PlatformIO que centraliza Wi‑Fi, transporte de mensagens (MQTT ou ESP‑NOW), gateway BLE opcional, OTA e um conjunto de “Capabilities” reutilizáveis (sensores/atuadores). O `IoTCore` orquestra conectividade, publica estados e executa comandos.

## O que há de novo no IoTCore

- Failover dinâmico de MQTT: primário (LAN, sem TLS) e secundário (externo, TLS) com migração automática quando o primário retornar.
- ESP‑NOW melhorado: anúncio de peers, assinatura automática de comando via MQTT quando em modo central, e ponte opcional de estados para MQTT.
- BLE Gateway: notificação incremental de estados e recebimento de comandos via GATT usando `NimBLE`.
- Builder de capabilities no `IoTCore` para facilitar composição de sensores/atores.

## Instalação (PlatformIO)

- `platformio.ini` — dependências e flags:

```
lib_deps =
  marceloc312/IoTPrivateHome
  knolleary/PubSubClient
  h2zero/NimBLE-Arduino         ; se usar BLE
  crankyoldgit/IRremoteESP8266  ; se usar IR

build_flags =
  -D TRANSPORT_MQTT             ; ou -D TRANSPORT_ESP_NOW
  -D NODE_BLE_ENABLED           ; habilita BLE Gateway (opcional)
  -D ESP_NOW_ENABLED            ; utilidades ESP‑NOW (opcional)
  -D OTA_DISABLED               ; desativa OTA (opcional)
  -D BH1750_ENABLED             ; sensor BH1750 (opcional)
  -D DHT_ENABLED                ; sensores DHTxx (opcional)
  -D DS18B20_ENABLED            ; sensor DS18B20 (opcional)
  -D RGB_ENABLED                ; NeoPixel (opcional)
  -D IRREMOTE_ENABLED           ; IR AC/Umidificador (opcional)
```

### Credenciais e brokers (recomendado via `private.ini`)

- Copie `private.ini.example` para `private.ini` e sobrescreva credenciais de broker/wi‑fi via `-D SERVER_*` e `-D WIFI_PASSWORD`.
- O `MqttClientHandler` usa dois perfis: `PRIMARY` (LAN) e `SECONDARY` (externo/TLS). O failover ocorre automaticamente.

## Uso básico

```
#include <Arduino.h>
#include <IoTCore.h>

IoTCore *iot = new IoTCore();

void setup() {
  Serial.begin(115200);
  iot->addLightCapability(44, DigitalLogic::INVERSE);
  iot->setup();
}

void loop() {
  iot->handle();
}
```

Estados são publicados em `device/state` e comandos são recebidos via `device/<device_id>/command` usando JSON compacto: `{ "capability_name", "value", "device_id" }`.

## Transportes

- `TRANSPORT_MQTT` (padrão): anuncia dispositivo em `smarthome/discovery`, publica estados e assina comandos; lida com failover (primário/externo TLS) e retorno ao primário quando disponível.
- `TRANSPORT_ESP_NOW`: enviar/receber estados/comandos por ESP‑NOW. Com `ESP_NOW_ENABLED` ativo em modo MQTT, a ponte de estados para MQTT fica ativa por padrão (desabilitável por firmware).

## BLE Gateway (opcional)

- Ative com `-D NODE_BLE_ENABLED`.
- Serviço GATT com escrita de comando e característica de notificação de estados. O comando recebido é encaminhado ao transporte atual (MQTT ou ESP‑NOW).

## Adicionando Capabilities (exemplos)

Todas as APIs abaixo são métodos do `IoTCore` e retornam a instância para encadeamento/config extra quando necessário.

### Atores

- Light: `iot->addLightCapability(pin[, DigitalLogic])`
```
iot->addLightCapability(2, DigitalLogic::INVERSE).turnOn();
```

- LED: `iot->addLEDCapability("led", pin[, DigitalLogic])`
```
auto &led = iot->addLEDCapability("status", 2, DigitalLogic::NORMAL);
```

- Switch: `iot->addSwitchCapability(pin[, DigitalLogic])`
```
iot->addSwitchCapability(5, DigitalLogic::NORMAL);
```

- SwitchPlug: `iot->addSwitchPlugCapability(pin[, DigitalLogic])`
```
iot->addSwitchPlugCapability(26, DigitalLogic::INVERSE);
```

- Valve: `iot->addValveCapability("valve", pin[, DigitalLogic])`
```
auto &valve = iot->addValveCapability("valve", 27);
```

- RGB (NeoPixel, `-D RGB_ENABLED`): `iot->addRGBLightCapability(pin, r, g, b)`
```
iot->addRGBLightCapability(48, 255, 0, 128);
```

- IR (AC e Umidificador, `-D IRREMOTE_ENABLED`):
```
iot->addAirConditionerCapability(4);
iot->addAirHumidifierCapability(4);
```

### Sensores

- Porta: `iot->addDoorSensorCapability(pin)`
```
iot->addDoorSensorCapability(14);
```

- PIR (presença): `iot->addPirSensorCapability(pin[, toleranciaSeg])`
```
iot->addPirSensorCapability(33, 5);
```

- Distância HC‑SR04: `iot->addHC_SR04DistanceSensorCapability(trig, echo[, min, max])`
```
iot->addHC_SR04DistanceSensorCapability(12, 13, 2, 300);
```

- Luminosidade BH1750 (`-D BH1750_ENABLED`):
```
iot->addLuminositySensorCapability(5 /*factor*/, 2 /*s*/);
```

- Umidade/Temperatura DHT (`-D DHT_ENABLED`):
```
#include <DHT.h>
auto dht = new DHT(23, DHT22);
iot->addHumiditySensorCapability(dht, 1);
iot->addTemperatureSensorCapability(dht, 1);
```

- Temperatura DS18B20 (`-D DS18B20_ENABLED`):
```
iot->addTemperatureSensorCapability(15, 1);
```

- IR Proximidade: `iot->addIRProximitySensorCapability("prox", pin)`
```
iot->addIRProximitySensorCapability("prox", 32);
```

- Clap (palma): `iot->addClapSensorCapability("clap", pin[, tolerancia])`
```
iot->addClapSensorCapability("clap", 25, 1);
```

- Nível de água (sonda por níveis): crie um `SondaWaterLevelSensor` e associe a capability
```
const int pins[] = {18, 19, 21, 0, 0, 0, 0, 0, 0, 0};
const int percents[] = {25, 50, 100, 0,0,0,0,0,0,0};
auto *sonda = new SondaWaterLevelSensor(pins, percents, 3, 1000);
iot->addWaterLevelPercentageCapability("tank_percent", sonda);
iot->addWaterLevelLitersCapability("tank_liters", sonda);
```

- Nível de água (ultrassônico): crie um `UltrassonicWaterLevelSensor`
```
auto *uw = new UltrassonicWaterLevelSensor(WaterLevelRecipentType::Circle1000L);
iot->addWaterHeightCapability("tank_height", uw);
```

- Vazão água Hall + Válvula:
```
auto &valve = iot->addValveCapability("valve", 27);
iot->addWaterFlowHallSensorCapability("flow", 34, &valve);
```

- Bateria (ADC): usar construtor direto e registrar manualmente:
```
auto *bat = new BatteryLevelCapability("battery", 34);
iot->addCapability(bat);
```

## Combinações úteis (receitas)

- Luz com presença (PIR → Light):
```
auto &light = iot->addLightCapability(2);
auto &pir = iot->addPirSensorCapability(33, 5);
// No loop do device (após iot->handle), a própria capability publica estados.
// Configure automações no servidor para acender a luz ao detectar presença.
```

- Porta + Alarme:
```
auto &alarm = iot->addAlarmCapability(26);
iot->addDoorSensorCapability(14);
// Acione o alarme via comando quando a porta abrir (automação no backend).
```

- Tanque + Válvula + Vazão (corte automático):
```
auto &valve = iot->addValveCapability("valve", 27);
iot->addWaterFlowHallSensorCapability("flow", 34, &valve);
// Crie regra no servidor: se litros acumulados > X ou fluxo contínuo > Y min → fechar válvula.
```

- RGB + Clap (efeitos):
```
iot->addRGBLightCapability(48, 0, 0, 255);
iot->addClapSensorCapability("clap", 25);
```

## Estrutura

- `src/Core`: `IoTCore`, OTA, BLE, executores.
- `src/Transports`: `IMessageClient`, MQTT adapter, ESP‑NOW.
- `src/Capabilities`: sensores/atores, estados/comandos.
- `src/Mqtt`: handler MQTT (anúncio, failover).
- `src/esp_now_utils`: callbacks/peer/bridge.

## Licença

MIT — ver `library.json`.
