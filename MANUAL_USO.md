# Manual de Utilizacao - IoTSmartSysCore

**Versao:** 1.0.0 | **Plataforma:** ESP32 (Arduino/PlatformIO) | **Atualizacao:** 26/11/2026

---

## 1. Introducao

Biblioteca framework IoT para ESP32/Arduino que fornece:
- **Conectividade:** Wi-Fi + MQTT com reconexao automatica
- **Capabilities:** Sensores e atuadores modulares
- **Metricas:** CPU, memoria, temperatura, rede
- **OTA:** Updates remotos com rollback
- **Provisionamento:** Web Portal, BLE, Serial
- **Persistencia:** Estados em flash (NVS)

### Requisitos
| Item | Especificacao |
|------|---------------|
| MCU | ESP32 (classic, S2, S3, C3, C6) |
| Flash | 4MB minimo (OTA) |
| RAM | 320KB livre |
| Wi-Fi | 2.4 GHz |

### Arquitetura Geral
```
main.cpp
  ├── setup() -> app.setup()
  │                 ├── ServiceManager (SettingsManager, DeviceIdentityProvider,
  │                 │                  CapabilityManager, TransportController)
  │                 └── FreeRTOS Tasks (networkTask, transportTask)
  └── loop() -> app.handle()  (processamento cooperativo)
```

---

## 2. Instalacao

### 2.1 Pré-requisitos
```bash
pip install platformio
```

### 2.2 Configuracao platformio.ini
```ini
[env:esp32_dev]
platform = platformio/espressif32@6.5.0
framework = arduino
board = esp32dev

build_flags =
  -DIOTSMARTSYS_OTA_ENABLED=1
  -DWEB_PORTAL_PROVISIONING_CHANNEL_ENABLE=1
  -DSERIAL_ENABLED=1
  -DIOTSMARTSYS_MAX_CAPABILITIES=8

board_build.partitions = partitions/ota_duplo_max_.csv
```

### 2.3 Flags Comuns
```ini
; OTA
-DIOTSMARTSYS_OTA_ENABLED=1

; Provisionamento
-DWEB_PORTAL_PROVISIONING_CHANNEL_ENABLE=1
-DBLE_PROVISIONING_CHANNEL_ENABLE=1

; Capabilities
-DIOTSMARTSYS_MAX_CAPABILITIES=12

; Sensores
-DDHT_SENSOR_ENABLED=1
-DBH1750_ENABLED=1
---

## 3. Primeiro Passo

### 3.1 Estrutura Minima
```cpp
#include <Arduino.h>
#include "SmartSysApp.h"

static iotsmartsys::SmartSysApp app;

void setup() {
  Serial.begin(115200);
  
  // Configurar ANTES de setup()
  app.configureLED({
    .capability_name = "led",
    .GPIO = LED_BUILTIN,
    .highIsOn = true
  });
  
  app.setup();
}

void loop() {
  app.handle();  // OBRIGATÓRIO - processamento cooperativo
}
```

### 3.2 Ordem de Configuracao
```cpp
void setup() {
  // 1. Botao reset
  app.configureFactoryResetButton({.GPIO = BOOT_PIN});
  
  // 2. LED
  app.configureLED({.GPIO = LED_BUILTIN, .highIsOn = false});
  
  // 3. Capabilities
  app.addLightCapability({.GPIO = 5, .highIsOn = true});
  
  // 4. Inicializa
  app.setup();
}
```

### 3.3 Verificacoes Iniciais
Monitor serial:
```
---------------------------------------------------------
App: IoT SmartSys Core Version: 1.0.0
App: Device ID: ESP32_ABC123
---------------------------------------------------------
```

---

## 5. Configuracao Inicial

### 5.1 Provisionamento
**Modo Normal:** Carrega settings do NVS -> Conecta Wi-Fi -> Conecta MQTT

**Modo Provisionamento:** Sem settings -> AP `IoTSmartSys-<id>` -> Portal Web/BLE

```ini
-DWEB_PORTAL_PROVISIONING_CHANNEL_ENABLE=1
-DBLE_PROVISIONING_CHANNEL_ENABLE=1
```

### 5.2 MQTT Settings
```cpp
struct MqttSettings {
  std::string broker;       // IP/host
  uint16_t port = 1883;
  std::string user;         // Opcional
  std::string password;     // Opcional
  std::string announce_topic = "smarthome/discovery";
  std::string command_topic = "device/command";
  std::string state_topic = "device/state";
};
```

### 5.3 Topicos MQTT
| Topico | Direcao | Descricao |
|--------|---------|-----------|
| `smarthome/discovery` | Publish | Announce ao conectar |
| `device/{id}/command` | Subscribe | Comandos |
| `device/state` | Publish | Estados |
| `device/metrics` | Publish | Metricas |

### 5.4 Identidade do Dispositivo
```cpp
std::string id = deviceIdentityProvider_.getDeviceID();
// Ex: "ESP32_ABC123DEF456"
```
Baseado em efuse, unico e persistente.

### 5.5 Botao Reset de Fabrica
```cpp
app.configureFactoryResetButton({
  .GPIO = BOOT_PIN,
  .debounce_ms = 50,
  .long_press_ms = 10000  // 10s = reset
});
```
**Comportamento:** <3s ignorado, 3-10s feedback, >10s reset NVS + reboot

### 5.6 LED de Status
```cpp
app.configureLED({
  .capability_name = "status",
  .GPIO = LED_BUILTIN,
  .highIsOn = false
});
```
**Modos:** Off, On, Slow Blink (desconectando), Fast Blink (provisionamento), Solid (conectado)

---

## 6. Capabilities

### 6.1 Conceito
Capabilities sao unidades funcionais:
- **Sensores:** Leitura (temperatura, movimento)
- **Atuadores:** Controle (rele, LED, buzzer)
- **Interfaces:** Botões, displays

### 6.2 Registro
```cpp
void setup() {
  // Antes de app.setup()
  app.addLightCapability({.GPIO = 5, .highIsOn = true});
  app.addPirSensorCapability({.GPIO = 15});
  app.setup();
}
```

### 6.3 Capabilities Disponiveis

**Atuadores:** Light, LED, Switch, Valve, Alarm
---

## 9. Configuracoes

### 9.1 Settings Manager
Arquitetura: Cache (RAM) <-> NVS (Flash) <-> API (opcional)

### 9.2 Atualizacao de Configuracoes
```cpp
settingsManager_.setUpdatedCallback(
    &SmartSysApp::onSettingsUpdatedThunk,
    this);

void onSettingsUpdated(const Settings &newSettings) {
  if (newSettings.hasChanges()) {
    settings_ = newSettings;
    applySettingsToRuntime(newSettings);
    
    // Se mudancas criticas: reiniciar
    if (needReboot(newSettings)) {
      systemCommandProcessor_.restartSafely();
    }
  }
}
```

### 9.3 Atualizacao via MQTT
```json
{
  "mqtt": {
    "broker": "mqtt.example.com",
    "port": 1883
  },
  "collect_interval_metrics": 30000
}
```

---

## 10. OTA

### 10.1 Habilitacao
```ini
-DIOTSMARTSYS_OTA_ENABLED=1
```

### 10.2 Manifesto de Firmware
```json
{
  "version": "1.2.3",
  "build": "20241126-abc123",
  "url": "https://updates.example.com/firmware.bin",
  "size": 1234567,
  "checksum": "sha256:abc123...",
  "min_version": "1.0.0",
  "target_envs": ["esp32_dev", "ESP32_MCB01"]
}
```

### 10.3 Fluxo de Atualizacao
---

## 13. Limitacoes

### 13.1 Plataforma
- **Suportado:** ESP32 (classic, S2, S3, C3, C6)
- **Nao suportado:** ESP8266
- **Wi-Fi:** 2.4 GHz apenas (5 GHz nao suportado)

### 13.2 Memoria
| Recurso | Consumo Aproximado |
|---------|-------------------|
| Base Framework | ~200KB heap |
| Cada Capability | ~5-20KB heap |
| OTA Stack | ~30KB heap + 1MB flash |
| Wi-Fi + MQTT | ~50KB heap |

**RAM minima recomendada:** 320KB livre

### 13.3 Limitacao de Capabilities
```ini
-DIOTSMARTSYS_MAX_CAPABILITIES=8  # Padrao
```
Exceder: Falha silenciosa (ultimas capacidades ignoradas)

### 13.4 Dependencias Externas
| Biblioteca | Versao |
|------------|--------|
| Arduino ESP32 | 2.x.x |
| adafruit/Adafruit Unified | 1.1.15 |
| adafruit/DHT | 1.4.6 |
| claws/BH1750 | 1.3.0 |
| milesburton/DallasTemperature | 4.0.4 |

### 13.5 Issues Conhecidos
| Issue | Impacto | Mitigacao |
|-------|---------|-----------|
| Desconexoes Wi-Fi | Perda temporaria | Reconexao automatica (max 30s) |
| Memory fragmentation | Heap fragmentado | Reinicio periodicda (24-48h) |
| OTA failure | Flash corrupto | Retry automatico, rollback |
| Watchdog timeout | Reset inesperado | Evitar delays > 5s |

---

## 14. Exemplos

### 14.1 Minimo Funcional
```cpp
#include <Arduino.h>
#include "SmartSysApp.h"

static iotsmartsys::SmartSysApp app;

void setup() {
  Serial.begin(115200);
  app.configureLED({.GPIO = LED_BUILTIN, .highIsOn = true});
  app.setup();
}

void loop() {
  app.handle();
}
```

### 14.2 Multiplas Capabilities
```cpp
#include <Arduino.h>
#include "SmartSysApp.h"

static iotsmartsys::SmartSysApp app;

void setup() {
  Serial.begin(115200);
  
  app.configureFactoryResetButton({.GPIO = BOOT_PIN});
  app.configureLED({.GPIO = LED_BUILTIN});
  
  app.addLightCapability({.GPIO = 5, .highIsOn = true});
  app.addPirSensorCapability({.GPIO = 15});
  
  app.setup();
}

void loop() {
  app.handle();
}
```

### 14.3 Com Sensor de Temperatura
```cpp
#include <Arduino.h>
#include "SmartSysApp.h"
#include "Core/Sensors/DS18B20Sensor.h"

static iotsmartsys::SmartSysApp app;
static std::unique_ptr<ITemperatureSensor> tempSensor;

void setup() {
  Serial.begin(115200);
  
  tempSensor = std::make_unique<DS18B20Sensor>(4);
  
  app.addTemperatureSensorCapability({
    .sensor = tempSensor.get(),
    .readIntervalMs = 60000
  });
  
  app.setup();
}

void loop() {
  app.handle();
}
```

---

## 15. Troubleshooting

### 15.1 Nao conecta ao Wi-Fi
**Causas:** Credenciais incorretas, AP fora do alcance, 5 GHz apenas

**Solucoes:**
1. Verificar SSID e senha
2. Checar sinal (rss)
3. Reiniciar roteador
4. Reset de fabrica e reconfigurar

### 15.2 Mensagens de reconexao
```
[Warn] [WiFi] Lost connection, reconnecting...
[Info] [WiFi] Connected to <ssid>
```
**Causas:** Sinal fraco (< -80 dBm), interferencia

**Solucoes:**
1. Mover dispositivo mais perto do roteador
2. Trocar canal do Wi-Fi

### 15.3 Capability nao responde
**Verificacoes:**
1. Pino GPIO correto?
2. Configured antes de setup()?
3. Dentro do limite (8)?
4. Pino em uso por outra capability?

### 15.4 LED nao acende
**Verificacoes:**
1. Pino GPIO correto?
2. `highIsOn` estah correto?
3. LED conectado corretamente?

### 15.5 Coleta de Informacoes para Debug
```cpp
void printDebugInfo() {
  Serial.println("=== Debug Info ===");
  Serial.printf("Device ID: %s\n", deviceIdentityProvider_.getDeviceID().c_str());
  Serial.printf("Version: %s\n", IOTSMARTSYSCORE_VERSION);
  Serial.printf("Free Heap: %u\n", ESP.getFreeHeap());
  Serial.printf("WiFi RSSI: %d\n", wifi_.getRssi());
  Serial.printf("WiFi Connected: %s\n", wifi_.isConnected() ? "yes" : "no");
  Serial.printf("MQTT Connected: %s\n", mqtt_.isConnected() ? "yes" : "no");
  Serial.println("==================");
}
```

### 15.6 Reset de Fabrica via Comando
```json
{
  "type": "factory_reset",
  "timestamp": 1699876543210
}
```
Publicar em `device/{device_id}/command`

---

## Apêndice A: Topicos MQTT

| Topico | Direcao | Descricao |
|--------|---------|-----------|
| `smarthome/discovery` | Publish | Announce ao conectar |
| `device/{device_id}/command` | Subscribe | Comandos |
| `device/state` | Publish | Estados |
| `device/metrics` | Publish | Metricas |

## Apêndice B: Pinos Recomendados (ESP32 DevKit)

| Pino | Uso Tipico |
|------|------------|
| GPIO 0 | Boot / Reset de fabrica |
| GPIO 2 | LED built-in |
| GPIO 5 | Rele / LED |
| GPIO 15 | Sensor digital |
| GPIO 33 | Sensor analogico |

---

**Fim do Manual**

Para suporte adicional:
- `README.md` do repositório
- Código-fonte em `src/SmartSysApp.h`
- Exemplos em `examples/`
1. Verificacao periodicda (se configurado)
2. Fetch manifesto da API
3. Comparar versao local vs remote
4. Se nova versao:
   - Validar manifesto (checksum, versao minima)
   - Download firmware (HTTP)
   - Validar checksum SHA256
   - Escrever particao OTA
   - Marcar para boot
5. Reinicio automatico
6. Se boot falhar: rollback automatico

### 10.4 Seguranca
- Checksum SHA256
- Versao minima (previne downgrade)
- Particoes OTA para rollback
- Reinicio seguro

---

## 11. Seguranca e Confiabilidade

### 11.1 Reset de Fabrica
- Pressionar botao 10s+
- LED pisca rapido
- NVS limpo
- Reinicio e provisionamento

### 11.2 Reinicio Seguro
```cpp
systemCommandProcessor_.restartSafely();
```
Fecha conexoes, salva estados, reinicio limpo.

### 11.3 Logs
```cpp
enum class LogLevel {
  Error,  // Erros criticos
  Warn,   // Avisos
  Info,   // Informacoes normais
  Debug   // Debug detalhado
};
```

```cpp
serviceManager_.setLogLevel(core::LogLevel::Info);
logger_.info("Module", "Mensagem: %s", value);
logger_.warn("Module", "Aviso: %s", value);
logger_.error("Module", "Erro: %s", value);
```

### 11.4 Watchdog
- Timeout: ~5s
- Evite delay() > 5s
- Chame app.handle() frequentemente

---

## 12. API Referencia

### 12.1 SmartSysApp

| Metodo | Descricao |
|--------|-----------|
| `void setup()` | Inicializa o runtime |
| `void handle()` | Processa eventos (no loop) |
| `configureLED(config)` | Configura LED status |
| `configureFactoryResetButton(config)` | Configura botao reset |
| `addLightCapability(config)` | Adiciona capability de luz |
| `addPirSensorCapability(config)` | Adiciona sensor PIR |
| `addTemperatureSensorCapability(config)` | Adiciona sensor de temperatura |

### 12.2 ServiceManager
```cpp
ServiceManager::init().settingsManager();
ServiceManager::init().capabilityManager();
ServiceManager::init().transportController();
```

### 12.3 SettingsManager
```cpp
settingsManager_.copyCurrent(settings);
settingsManager_.setUpdatedCallback(cb, ctx);
settingsManager_.applySettings(newSettings);
```

**Sensores Digitais:** PirSensor, DoorSensor, PushButton, TouchButton, ClapSensor

**Sensores Analogicos:** Temperature, Humidity, Luminosity, WaterLevelPercent, GlpSensor

### 6.4 Callbacks
```cpp
auto *light = app.addLightCapability({...});
light->addStateObserver([](bool isOn) {
  Serial.printf("Luz: %s\n", isOn ? "ON" : "OFF");
});
```

---

## 7. Conectividade

### 7.1 Wi-Fi
**Estados:** Disconnected -> Connecting -> Connected -> Lost -> Provisioning

**Reconexao:** Backoff exponencial (max 30s), apos 5 falhas -> AP provisionamento

```cpp
wifi_.getSsid();           // SSID
wifi_.getIpAddress();      // IP
wifi_.getRssi();           // RSSI (-100 a 0)
wifi_.getSignalStrength(); // Signal
```

### 7.2 MQTT
**Cliente:** EspIdfMqttClient, MQTT 3.1.1
**QoS:** 0 (metricas), 1 (comandos)

**Fila Offline:** Limite padrao 10 mensagens (FIFO)

```cpp
auto stats = mqtt_.offlineQueueStats();
```

### 7.3 Callback MQTT
```cpp
transportController_.configureMqtt(
    deviceIdentityProvider_.getDeviceID().c_str(),
    &SmartSysApp::onMqttConnectedThunk,
    this);

void onMqttConnected(const TransportConnectedView &info) {
  // info.clientId, info.broker
  // Publicar announce, assinar comandos
}
```

### 7.4 Transporte Serial
```ini
-DSERIAL_ENABLED=1
-DSERIAL_BAUD_RATE=115200
```
Protocolo JSON sincrono.

---

## 8. Metricas

### 8.1 Payload Padrao
```json
{
  "device_id": "ESP32_ABC123",
  "uptime_ms": 86400000,
  "cpu_cores": 2,
  "cpu_percent": 15.5,
  "memory_percent": 45.2,
  "temperature_c": 42.1,
  "frequency_mhz": 240,
  "network": {
    "rssi": -65,
    "last_disconnected": 1699876543210,
    "connection_count": 5
  }
}
```

### 8.2 Campos Principais
| Campo | Descricao |
|-------|-----------|
| `cpu_percent` | Uso de CPU (%) |
| `memory_percent` | Uso de heap (%) |
| `rssi` | Sinal Wi-Fi (dBm) |
| `uptime_ms` | Tempo ligado (ms) |

### 8.3 Intervalo de Publicacao
```ini
-DIOTSMARTSYS_METRICS_INTERVAL_MS=60000  # 60s padrao
```
Ou via settings: `collect_interval_metrics = 60000`

**Desabilitar:** `collect_interval_metrics = 0`

### 8.4 Topico
`device/metrics` (QoS 0, periodicamente)
Sem Wi-Fi em 30s -> AP provisionamento:
- SSID: `IoTSmartSys-<device_id>`
- IP: `192.168.4.1`
- Portal: `http://192.168.4.1`

---

## 4. Arquitetura

### 4.1 Ciclo de Vida
```cpp
setup() -> app.setup()   // Uma vez
loop() -> app.handle()   // Repetidamente (a cada <100ms)
```

### 4.2 Tasks FreeRTOS
- **networkTask** (prioridade 4): Wi-Fi, reconexao, MQTT init
- **transportTask** (prioridade 4): MQTT pub/sub, serial

### 4.3 handle() Cooperativo
```cpp
void loop() {
  app.handle();
  
  // Evite delay() longos - use millis()
  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();
    // Tarefa nao-bloqueante
  }
}
```

### 4.4 Limite de Capabilities
```ini
-DIOTSMARTSYS_MAX_CAPABILITIES=8  # Padrao
-DIOTSMARTSYS_MAX_CAPABILITIES=12 # Maximo testado
```
-DDS18B20_SENSOR_ENABLED=1

; Pinos
-DLED_PIN=23
-DPIR_SENSOR_PIN=15
```

### 2.4 Comandos
```bash
pio run -e esp32_dev              # Build
pio run -e esp32_dev -t upload    # Upload
pio device monitor -b 115200      # Serial
pio test -e esp32s3_test          # Testes
```
**ESP8266 nao suportado.**
