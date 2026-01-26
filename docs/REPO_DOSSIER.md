# Dossie Tecnico do Repositorio IoTSmartSysCore

## TOC
- [1. Inventario e Mapa do Repo](#1-inventario-e-mapa-do-repo)
  - [1.1 Arvore de pastas (ate 3 niveis)](#11-arvore-de-pastas-ate-3-niveis)
  - [1.2 Modulos principais e papeis](#12-modulos-principais-e-papeis)
  - [1.3 Entry points e consumo](#13-entry-points-e-consumo)
- [2. Build/Targets (PlatformIO e afins)](#2-buildtargets-platformio-e-afins)
  - [2.1 platformio.ini e configs/*.ini](#21-platformioini-e-configsini)
  - [2.2 Envs relevantes (por alvo)](#22-envs-relevantes-por-alvo)
  - [2.3 Boards custom](#23-boards-custom)
  - [2.4 Matriz Target x Feature](#24-matriz-target-x-feature)
- [3. Arquitetura (Visao de Camadas)](#3-arquitetura-visao-de-camadas)
  - [3.1 Diagrama textual](#31-diagrama-textual)
  - [3.2 Contratos e padroes](#32-contratos-e-padroes)
  - [3.3 Hot paths](#33-hot-paths)
  - [3.4 Logging](#34-logging)
- [4. Mapa de Dependencias (Includes e Acoplamentos)](#4-mapa-de-dependencias-includes-e-acoplamentos)
  - [4.1 Mapa de includes (top 15)](#41-mapa-de-includes-top-15)
  - [4.2 Central headers](#42-central-headers)
  - [4.3 Dependencias invertidas (alto nivel)](#43-dependencias-invertidas-alto-nivel)
  - [4.4 Violations](#44-violations)
- [5. Concorrencia / RTOS / Tempo](#5-concorrencia--rtos--tempo)
  - [5.1 Inventario FreeRTOS/tempo](#51-inventario-freertos-tempo)
  - [5.2 Modelo de concorrencia atual](#52-modelo-de-concorrencia-atual)
  - [5.3 Assuncoes ESP32 codificadas](#53-assuncoes-esp32-codificadas)
- [6. Radar de Compilacao Condicional](#6-radar-de-compilacao-condicional)
  - [6.1 Padroes de #if/#ifdef](#61-padroes-de-ififdef)
  - [6.2 Tabela de macros](#62-tabela-de-macros)
  - [6.3 Macros do projeto e onde sao definidas](#63-macros-do-projeto-e-onde-sao-definidas)
- [7. Dependencias Externas e Integracoes](#7-dependencias-externas-e-integracoes)
  - [7.1 lib_deps e libs por include](#71-lib_deps-e-libs-por-include)
  - [7.2 Integracoes presentes](#72-integracoes-presentes)
  - [7.3 Detalhes por integracao](#73-detalhes-por-integracao)
- [8. Padroes de Codigo e Convencoes](#8-padroes-de-codigo-e-convencoes)
  - [8.1 Convencoes de nomes](#81-convencoes-de-nomes)
  - [8.2 Padrao de configuracao](#82-padrao-de-configuracao)
  - [8.3 Regras editoriais/filosofia](#83-regras-editoriaisfilosofia)
- [9. Como Estender](#9-como-estender)
  - [9.1 Nova Capability](#91-nova-capability)
  - [9.2 Novo Provider/Service](#92-novo-providerservice)
  - [9.3 Novo firmware/app consumindo a lib](#93-novo-firmwareapp-consumindo-a-lib)
- [10. Apendice: Acoplamentos Perigosos (ESP32/FreeRTOS)](#10-apendice-acoplamentos-perigosos-esp32freertos)
  - [10.1 Headers de Core que incluem freertos/ ou esp-idf](#101-headers-de-core-que-incluem-freertos-ou-esp-idf)
  - [10.2 Tipos FreeRTOS expostos em interfaces publicas](#102-tipos-freertos-expostos-em-interfaces-publicas)
  - [10.3 Casos em que o Core exige threading/RTOS](#103-casos-em-que-o-core-exige-threadingrtos)
  - [10.4 Callback sob lock / secao critica](#104-callback-sob-lock--secao-critica)
- [11. Riscos e Plano de Migracao](#11-riscos-e-plano-de-migracao)
  - [11.1 Top 10 riscos tecnicos](#111-top-10-riscos-tecnicos)
  - [11.2 Plano de migracao para ESP8266 (sem implementar)](#112-plano-de-migracao-para-esp8266-sem-implementar)

## 1. Inventario e Mapa do Repo

### 1.1 Arvore de pastas (ate 3 niveis)

```
.
├── boards/
│   └── iotsmartsys_mcb_r1.json
├── configs/
│   ├── base_esp.ini
│   └── esp32s3-test.ini
├── docs/
│   └── REPO_DOSSIER.md
├── examples/
│   ├── basic_usage/
│   ├── composite/
│   └── configuration_portal/
├── include/
│   └── boards/
│       └── iotsmartsys_mcb_r1/
├── lib/
├── partitions/
├── src/
│   ├── App/
│   ├── Contracts/
│   ├── Core/
│   ├── Infra/
│   ├── Platform/
│   ├── Settings/
│   └── Version/
├── test/
│   ├── mocks/
│   ├── test_builder/
│   ├── test_glp_meter/
│   ├── test_glp_sensor/
│   ├── test_heightwaterlevel/
│   ├── test_humidity/
│   ├── test_mqtt_settings/
│   ├── test_operational_color_sensor/
│   ├── test_settings_provider/
│   ├── test_temperature/
│   ├── test_waterflow/
│   ├── test_waterlevelpercent/
│   └── test_waterlevelliters/
├── tools/
│   └── remove_comments.py
├── include/README
├── library.json
├── Makefile
├── platformio.ini
└── private.ini
```

Observacao: `private.ini` pode conter credenciais; nao inspecionado nem exposto (so referenciado).

### 1.2 Modulos principais e papeis

- **Contracts**: Interfaces e tipos de dominio (capabilities, settings, logging, transports). Ex.: `src/Contracts/Capabilities/ICapability.h`, `src/Contracts/Settings/Settings.h`, `src/Contracts/Logging/ILogger.h`.
- **Core**: Logica de negocio e runtime (capabilities, services, settings manager, provisioning, commands, transports). Ex.: `src/Core/Capabilities/`, `src/Core/Settings/SettingsManager.cpp`, `src/Core/Transports/TransportHub.h`.
- **App**: Orquestracao do app e builders de capabilities. Ex.: `src/SmartSysApp.h`, `src/App/Builders/Builders/CapabilitiesBuilder.h`, `src/App/Managers/`.
- **Platform/Arduino**: Adaptadores Arduino (logger, hardware adapters, sensores, serial transport, web portal). Ex.: `src/Platform/Arduino/Logging/ArduinoSerialLogger.h`, `src/Platform/Arduino/Transports/ArduinoSerialTransportChannel.h`.
- **Platform/Espressif**: Integracoes Espressif/ESP-IDF (MQTT, NVS, BLE provisioning, event bridges, pinouts). Ex.: `src/Platform/Espressif/Mqtt/EspIdfMqttClient.h`, `src/Platform/Espressif/Settings/Providers/EspIdfNvsSettingsProvider.h`.
- **Infra**: OTA, factories de sensores, display. Ex.: `src/Infra/OTA/OTAManager.h`, `src/Infra/Factories/SensorFactory.h`, `src/Infra/display/`.
- **Settings**: Pasta reservada (pouco usada no recorte atual). Ver `src/Settings/`.
- **Version**: Metadados de build/versao. Ex.: `src/Version/VersionInfo.h`.
- **tests**: Suite de testes (Unity). Ex.: `test/test_*` e `configs/esp32s3-test.ini`.

### 1.3 Entry points e consumo

- **Firmware principal (Arduino/PlatformIO)**: `src/main.cpp` cria `iotsmartsys::SmartSysApp` e chama `setup()/handle()`.
- **Exemplos**: `examples/basic_usage/main.cpp.ino`, `examples/composite/main.cpp`, `examples/configuration_portal/main.cpp.ino`.
- **Lib consumida por terceiros**: via `library.json` e includes de `src/SmartSysApp.h` (uso descrito em `Readme.md`).

## 2. Build/Targets (PlatformIO e afins)

### 2.1 platformio.ini e configs/*.ini

- `platformio.ini` define envs e referencia configs em `configs/*.ini` e `private.ini` via `extra_configs`.
- `configs/base_esp.ini` define base comum (Arduino + espressif32) e flags padrao.
- `configs/esp32s3-test.ini` define env de teste com Unity e flags de teste (ver observacao sobre `env:base32`).
- `private.ini` existe e pode conter dados sensiveis; somente referenciado, nao lido.

### 2.2 Envs relevantes (por alvo)

- **env:ESP32_MCB01**
  - board: `iotsmartsys_mcb_r1` (custom)
  - platform: herdado de `env:base_esp` (`espressif32`)
  - framework: `arduino`
  - build_flags: herdado de `configs/base_esp.ini`
  - observacoes: define macros de board no JSON (`IOTSMARTSYS_MCB01`, `IOTSMARTSYS_BOARD_REV`).

- **env:esp32_dev**
  - board: `esp32dev`
  - platform/framework: herdado de `env:base_esp`
  - build_flags: herdado de `configs/base_esp.ini`

- **env:esp8266_dev**
  - board: `esp12e`
  - platform: `espressif8266` (override)
  - framework: `arduino`
  - build_flags: herdado de `configs/base_esp.ini`
  - observacoes: varios componentes centrais sao ESP32/ESP-IDF (ver secoes 5, 10 e 11).

- **env:esp32s3**
  - board: `esp32-s3-devkitc-1`
  - platform/framework: herdado de `env:base_esp`
  - build_flags adicionais: `-DARDUINO_USB_CDC_ON_BOOT=1`, `-DARDUINO_USB_MODE=1`

- **env:esp32s3_test (configs/esp32s3-test.ini)**
  - extends: `env:base32` (nao encontrado no repo; possivel typo ou dependencia externa)
  - test_framework: `unity`, `test_build_src = yes`
  - build_flags: inclui `-D UNIT_TEST_MAIN` e pinos de teste

### 2.3 Boards custom

- `boards/iotsmartsys_mcb_r1.json` define board IoTSmartSys MCB R1, com `extra_flags` para macros de board.
- Pinouts customizados em `src/Platform/Espressif/Pinouts/SmartSys_MCB01_Pinouts.h` e selecionados em `src/pins.h`.
- `include/boards/iotsmartsys_mcb_r1/pins.h` esta vazio (possivel placeholder).

### 2.4 Matriz Target x Feature

| Target | Wi-Fi | MQTT | OTA | HTTP Update | BLE Prov | ESPNOW | Zigbee | Storage (NVS/FS) | RTOS usage | Observacoes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| ESP32_MCB01 | Sim | Sim (ESP-IDF) | Opcional | Sim | Opcional | Nao | Nao | NVS (ESP-IDF) | Sim | Board custom, macros de pinout. |
| esp32_dev | Sim | Sim (ESP-IDF) | Opcional | Sim | Opcional | Nao | Nao | NVS (ESP-IDF) | Sim | Similar ao ESP32_MCB01, sem pinout custom. |
| esp32s3 | Sim | Sim (ESP-IDF) | Opcional | Sim | Opcional | Nao | Nao | NVS (ESP-IDF) | Sim | USB CDC flags habilitados. |
| esp8266_dev | Sim | Parcial/NA | Opcional | Sim | Nao (BLE ESP32) | Nao | Nao | Nao claro (NVS ESP-IDF) | Parcial | Varios headers e providers sao ESP32-only (risco de build). |

## 3. Arquitetura (Visao de Camadas)

### 3.1 Diagrama textual

```
App (SmartSysApp, Managers, Builders)
  -> Core (Capabilities, Settings, Transports, Commands, Provisioning)
    -> Contracts (Interfaces e tipos comuns)
    -> Platform/Adapters (Arduino/Espressif)
  -> Infra (OTA, SensorFactory, Display)
```

### 3.2 Contratos e padroes

- **Contracts (interfaces)**: `src/Contracts/**` define contratos de capabilities, settings, logging e transports. Ex.: `src/Contracts/Capabilities/ICapability.h`, `src/Contracts/Settings/IReadOnlySettingsProvider.h`, `src/Contracts/Logging/ILogger.h`.
- **ServiceProvider/ServiceManager (DI simples)**:
  - Registro central de servicos em `src/Core/Providers/ServiceManager.cpp`.
  - `ServiceProvider` exposto em `src/Contracts/Providers/ServiceProvider.h` e instanciado em `src/Core/Providers/ServiceProvider.cpp`.
  - Padrao: singleton + setters de ponteiros (`setLogger`, `setSettings`, etc.).
- **Gates (readiness)**:
  - `ConnectivityGate` usa latch de bits para Wi-Fi/IP/MQTT: `src/Contracts/Connectivity/ConnectivityGate.h` e `src/Core/Connectivity/ConnectivityGate.cpp`.
  - `SettingsGate` controla nivel de prontidao de settings: contrato em `src/Contracts/Settings/SettingsGate.h`, implementacao em `src/Core/Settings/SettingsGateImpl.h`.
  - Esses gates sao usados por `src/App/Wifi/WiFiManager.cpp`, `src/Core/Services/MqttService.cpp` e `src/Infra/OTA/OTAManager.cpp`.

### 3.3 Hot paths

- **Boot**: `src/SmartSysApp.cpp` -> `ServiceManager::init()` -> `SettingsManager::init()` -> `ConnectivityBootstrap::run()`.
- **Conectividade**: `WiFiManager::begin/handle()` (`src/App/Wifi/WiFiManager.cpp`) -> `ConnectivityGate` bits -> `MqttService::handle()` (`src/Core/Services/MqttService.cpp`).
- **Settings**: `SettingsManager::handle()` dispara sync API via `EspIdfSettingsFetcher` (`src/Platform/Espressif/Settings/EspIdfSettingsFetcher.cpp`) -> `SettingsGate` -> callback `SmartSysApp::onSettingsUpdated()`.
- **MQTT**: `TransportController::configureMqtt()` (`src/App/Managers/TransportController.cpp`) -> `MqttService` -> `EspIdfMqttClient` (`src/Platform/Espressif/Mqtt/EspIdfMqttClient.h`).
- **OTA**: `OTAManager::handle()` (`src/Infra/OTA/OTAManager.cpp`) -> `FirmwareUpdater` (`src/Infra/OTA/FirmwareUpdater.cpp`) ou `OTA` (`src/Infra/OTA/OTA.cpp`), conforme flags.
- **Sensors/Capabilities**: `CapabilityController::handle()` -> `CapabilityManager` (`src/Core/Capabilities/CapabilityManager.cpp`) -> capabilities individuais.

### 3.4 Logging

- **Interface**: `src/Contracts/Logging/ILogger.h` (logf + helpers) e `src/Contracts/Logging/Log.h` (logger global).
- **Implementacao default**: `src/Platform/Arduino/Logging/ArduinoSerialLogger.h` com controle de nivel em `setMinLevel()`.
- **Inicializacao**: `ServiceManager::registerServices()` em `src/Core/Providers/ServiceManager.cpp` configura `Log::setLogger()` e `Time::setProvider()`.
- **Tags**: geralmente string literal (ex.: "MQTT", "SettingsManager") em `src/Core/Services/MqttService.cpp`, `src/Core/Settings/SettingsManager.cpp`.

## 4. Mapa de Dependencias (Includes e Acoplamentos)

### 4.1 Mapa de includes (top 15)

Contagem aproximada via busca de `#include`.

| Include | Ocorrencias (aprox.) |
| --- | --- |
| `string` | 34 |
| `Contracts/Logging/ILogger.h` | 28 |
| `Arduino.h` | 28 |
| `cstdint` | 20 |
| `Contracts/Logging/Log.h` | 14 |
| `cstring` | 13 |
| `Core/Capabilities/CapabilityHelpers.h` | 12 |
| `vector` | 11 |
| `ICapability.h` | 10 |
| `Contracts/Connectivity/ConnectivityGate.h` | 10 |
| `Contracts/Adapters/IHardwareAdapter.h` | 10 |
| `cstdio` | 9 |
| `Contracts/Connections/WiFiManager.h` | 9 |
| `Contracts/Common/StateResult.h` | 9 |
| `Contracts/Settings/Settings.h` | 8 |

### 4.2 Central headers

- **`src/Contracts/Logging/ILogger.h`**
  - Define interface de logging e niveis (`LogLevel`).
  - Dependentes: Core/App/Platform (uso espalhado em `src/Core/**`, `src/App/**`, `src/Platform/**`).
  - Risco: baixo, mas o logger e global por `Log::get()`.

- **`src/Contracts/Logging/Log.h`**
  - Accessor global de logger; default no-op em `src/Contracts/Logging/Log.cpp`.
  - Dependentes: Core, Platform (ex.: `src/Infra/OTA/OTA.cpp`).
  - Risco: uso global dificulta testes (injeccao limitada).

- **`src/Core/Capabilities/CapabilityHelpers.h`**
  - Helpers para debouncing/latching/polling (base para muitas capabilities).
  - Dependentes: varias capabilities em `src/Core/Capabilities/**`.
  - Risco: acoplamento de comportamento comum; mudancas afetam varias capabilities.

- **`src/Contracts/Connectivity/ConnectivityGate.h`**
  - Contrato de bits de conectividade (Wi-Fi, IP, MQTT).
  - Dependentes: `src/App/Wifi/WiFiManager.cpp`, `src/Core/Services/MqttService.cpp`, `src/Infra/OTA/OTAManager.cpp`.
  - Risco: sem gate, MQTT/OTA ficam bloqueados.

- **`src/Contracts/Connections/WiFiManager.h`**
  - Abstracao de Wi-Fi com includes de `WiFi.h`/`ESP8266WiFi.h`.
  - Dependentes: App e Platform (provisioning e settings). Ex.: `src/App/Managers/ProvisioningController.cpp`.
  - Risco: header de Contracts puxa Arduino WiFi (peso e acoplamento de plataforma).

- **`src/Contracts/Settings/Settings.h`**
  - Estrutura principal de settings (mqtt/wifi/api/firmware).
  - Dependentes: SettingsManager, OTA, MqttSink, Provisioning. Ex.: `src/Core/Settings/SettingsManager.cpp`, `src/Infra/OTA/OTAManager.cpp`.
  - Risco: qualquer mudanca pode repercutir no fluxo de provisioning e OTA.

- **`src/Contracts/Common/StateResult.h`**
  - Enum de erros utilizado em settings e providers.
  - Dependentes: `src/Core/Settings/SettingsManager.cpp`, `src/Platform/Espressif/Settings/EspIdfSettingsFetcher.cpp`.
  - Risco: baixo, mas usado em contracts publicos.

- **`src/Contracts/Providers/Time.h`**
  - Provider global de tempo (`Time::get`).
  - Dependentes: capabilities, WiFiManager, MqttService. Ex.: `src/Core/Capabilities/CapabilityHelpers.h`.
  - Risco: global state; cuidado ao mockar para testes.

- **`src/Contracts/Adapters/IHardwareAdapter.h`**
  - Base para adaptadores de hardware (input/output).
  - Dependentes: capabilities e factories em `src/Platform/Arduino/Factories/`.
  - Risco: baixo, mas central para todas as capabilities.

- **`src/Contracts/Capabilities/ICapability.h`**
  - Base de capability com estado, tipo e event sink.
  - Dependentes: praticamente todas as capabilities e builders.
  - Risco: mudancas afetam todo o ecossistema de capabilities.

### 4.3 Dependencias invertidas (alto nivel)

- **Core depende de**: Contracts (forte), Platform (direto em `src/Core/Providers/ServiceManager.h` e ESP-IDF em `src/Core/Services/IMqttClient.h`), Arduino/ESP-IDF em alguns .cpp.
- **App depende de**: Core, Contracts, Platform (ex.: `src/SmartSysApp.h` inclui `Platform/Espressif/**`), Infra (OTA, SensorFactory).
- **Platform depende de**: Contracts (interfaces), Core (ex.: `ConnectivityGate`, `SettingsManager`).
- **Infra depende de**: Contracts, Core e Arduino/Platform (ex.: `src/Infra/OTA/FirmwareUpdater.cpp` inclui `WiFi.h`/`HTTPClient.h`).

### 4.4 Violations

- **Core inclui Platform**: `src/Core/Providers/ServiceManager.h` inclui `src/Platform/Arduino/**` e `src/Platform/Espressif/**` (acoplamento direto Core->Platform).
- **Core inclui ESP-IDF**: `src/Core/Services/IMqttClient.h` inclui `mqtt_client.h` e expõe tipos `esp_err_t`/`esp_mqtt_event_handle_t`.
- **Core inclui FreeRTOS em header**: `src/Core/Settings/SettingsGateImpl.h` inclui `freertos/FreeRTOS.h` e `freertos/semphr.h`.
- **Contracts inclui WiFi.h/ESP8266WiFi.h**: `src/Contracts/Connections/WiFiManager.h` depende diretamente de headers Arduino.
- **Header pesado em App**: `src/SmartSysApp.h` agrega muitos includes de Platform/Infra, aumentando tempo de build e acoplamento.

## 5. Concorrencia / RTOS / Tempo

### 5.1 Inventario FreeRTOS/tempo

| Arquivo | Tipo | Impacto | Como migrar (ideia) |
| --- | --- | --- | --- |
| `src/Platform/Espressif/Arduino/Connectivity/ArduinoEventLatch.h` | event group | Alto | Criar latch baseado em bitset/atomics (sem FreeRTOS). |
| `src/Platform/Espressif/Connectivity/FreeRtosEventLatch.h` | event group | Alto | Substituir por implementacao leve no Platform (ESP8266). |
| `src/Platform/Espressif/Runtime/FreeRtosGatedRoutineRunner.h` | task + delay | Alto | Criar scheduler cooperativo (loop) ou Task abstraction. |
| `src/Platform/Espressif/Settings/EspIdfSettingsFetcher.cpp` | task + semaphore + delay | Alto | Reescrever fetcher com polling no loop ou task wrapper. |
| `src/Platform/Espressif/Settings/EspIdfSettingsFetcher.h` | SemaphoreHandle_t/TaskHandle_t | Alto | Esconder tipos via PIMPL ou typedefs por plataforma. |
| `src/Core/Settings/SettingsManager.cpp` | semaphore | Medio | Trocar por mutex/lock abstraction em Core. |
| `src/Core/Settings/SettingsGateImpl.h` | SemaphoreHandle_t | Medio | Mover para Platform e manter interface em Contracts. |

### 5.2 Modelo de concorrencia atual

- **Loop principal**: `SmartSysApp::handle()` chama managers e servicos em loop (`wifi_.handle()`, `otaManager_.handle()`, `settingsManager_.handle()`, `transportController_.handle()`).
- **Task dedicada**: `EspIdfSettingsFetcher` cria task para fetch HTTP (`xTaskCreate`), com callbacks de retorno em outra thread.
- **Callbacks/eventos**: Wi-Fi usa eventos (`WiFi.onEvent`) em `src/App/Wifi/WiFiManager.cpp`; MQTT usa callbacks do ESP-IDF em `src/Platform/Espressif/Mqtt/EspIdfMqttClient.cpp`.
- **Riscos principais**:
  - Callback de settings (`SettingsManager::onFetchCompleted`) roda fora do loop, podendo disparar `SmartSysApp::onSettingsUpdated()` e `SystemCommandProcessor::restartSafely()` fora do contexto principal.
  - Uso de `ConnectivityGate` como latch global pode esconder transicoes rapidas (state flapping).
  - Conflito potencial entre loop e task em estruturas compartilhadas (mitigado com semaforo em settings).

### 5.3 Assuncoes ESP32 codificadas

- Uso de ESP-IDF direto em multiplos pontos: `src/Core/Services/IMqttClient.h` (`mqtt_client.h`), `src/Platform/Espressif/Settings/EspIdfSettingsFetcher.cpp` (`esp_http_client`), `src/Platform/Espressif/Providers/DeviceIdentityProvider.cpp` (`esp_efuse_mac_get_default`), `src/Core/Commands/SystemCommandProcessor.cpp` (`esp_restart`, `esp_wifi_stop`).
- `ConnectivityBootstrap` usa `esp_ota_ops.h` em `src/App/Managers/ConnectivityBootstrap.cpp`.
- Pinouts dependem de `sdkconfig.h` e `CONFIG_IDF_TARGET_*` em `src/Platform/Espressif/Pinouts/*.h`.

## 6. Radar de Compilacao Condicional

### 6.1 Padroes de #if/#ifdef

- Targets: `ESP32`, `ESP8266`, `ARDUINO_ARCH_ESP32`, `CONFIG_IDF_TARGET_*`.
- Features: provisioning (`BLE_PROVISIONING_CHANNEL_ENABLE`, `WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE`, `WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE`), OTA (`OTA_DISABLED`), sensores (`DHT_SENSOR_ENABLED`, `DS18B20_SENSOR_ENABLED`, `BH1750_ENABLED`, `BMP180_SENSOR_ENABLED`, `HX711_ENABLED`, `IR_REMOTE_ESP8266`), display (`ST7789_170x320_ENABLED`).

### 6.2 Tabela de macros

| Arquivo | Macro | Proposito | Risco | Sugestao |
| --- | --- | --- | --- | --- |
| `src/Core/Services/IMqttClient.h` | `ESP32` | Compilar MQTT ESP-IDF | Alto para ESP8266 | Criar interface neutra e impl por plataforma. |
| `src/Contracts/Connections/WiFiManager.h` | `ESP32` | Selecionar WiFi.h vs ESP8266WiFi.h | Medio | Isolar WiFi includes em Platform. |
| `src/SmartSysApp.h` | `OTA_DISABLED` | Compilar sem OTA | Baixo | Manter flag; documentar default. |
| `src/App/Managers/ProvisioningController.h` | `BLE_PROVISIONING_CHANNEL_ENABLE` | Habilitar canal BLE | Medio | Garantir fallback quando desabilitado. |
| `src/Platform/Arduino/Provisioning/WebPortalProvisioningChannel.cpp` | `WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE` | Captive portal | Baixo | Documentar dependencia de DNS. |
| `src/Contracts/Sensors/SensorModel.h` | `DHT_SENSOR_ENABLED`, `DS18B20_SENSOR_ENABLED`, `BMP180_SENSOR_ENABLED` | Controle de sensores | Baixo | Centralizar flags em um header. |
| `src/Platform/Espressif/Pinouts/*.h` | `CONFIG_IDF_TARGET_*` | Seleciona pinouts por target | Medio | Abstrair por board/variant. |
| `src/Infra/OTA/FirmwareUpdater.cpp` | `ESP32`/`ESP8266` | Escolhe HTTP libs | Medio | Usar facade de HTTP Update por plataforma. |
| `src/Platform/Espressif/Arduino/Connectivity/ArduinoEventLatch.h` | `ARDUINO_ARCH_ESP32` | EventGroup no Arduino ESP32 | Alto | Implementar latch alternativo para ESP8266. |
| `src/Infra/display/Display_ST7789_170_320.*` | `ST7789_170x320_ENABLED` | Display opcional | Baixo | Documentar flag no README. |

### 6.3 Macros do projeto e onde sao definidas

- **Flags de provisioning**: `WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE`, `WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE`, `BLE_PROVISIONING_CHANNEL_ENABLE` em `configs/base_esp.ini`.
- **Flags de sensores**: `DHT_SENSOR_ENABLED`, `DS18B20_SENSOR_ENABLED`, `BH1750_ENABLED`, `BMP180_SENSOR_ENABLED`, `HX711_ENABLED`, `IR_REMOTE_ESP8266` comentadas em `configs/base_esp.ini` e usadas em `src/Platform/Arduino/Sensors/**`.
- **OTA**: `OTA_DISABLED` (comentado em `configs/base_esp.ini`).
- **Board**: `IOTSMARTSYS_MCB01`, `IOTSMARTSYS_BOARD_REV` em `boards/iotsmartsys_mcb_r1.json`.
- **USB CDC (S3)**: `ARDUINO_USB_CDC_ON_BOOT`, `ARDUINO_USB_MODE` em `platformio.ini` (env:esp32s3).
- **Testes**: `UNIT_TEST_MAIN`, `PIN_TEST`, `BAUD_RATE` em `configs/esp32s3-test.ini` (com possivel typo `-D BAUD_RATE`).
- **Privadas**: `private.ini` pode injetar macros/credenciais (nao inspecionado).

## 7. Dependencias Externas e Integracoes

### 7.1 lib_deps e libs por include

- `library.json` inclui: Adafruit ST7735/ST7789, Adafruit GFX, BH1750, Wire, Adafruit Unified Sensor, DHT, DallasTemperature, SparkFun APDS9960, Adafruit NeoPixel.
- `configs/base_esp.ini` tem `lib_deps` comentado (provavel duplicacao/override pelo `library.json`).

### 7.2 Integracoes presentes

- **MQTT**: cliente ESP-IDF (`EspIdfMqttClient`) e roteamento via `TransportHub`.
- **OTA**: ArduinoOTA (`src/Infra/OTA/OTA.cpp`) e HTTP Update (`src/Infra/OTA/FirmwareUpdater.cpp`).
- **Provisioning**: BLE GATT (ESP-IDF) e Web Portal (Arduino WebServer).
- **Settings**: NVS (ESP-IDF) + fetch HTTP de API (ESP-IDF).
- **Serial Transport**: UART via `ArduinoSerialTransportChannel`.
- **Sensores**: DHT, DS18B20, BH1750, HX711, IR, etc. (condicional por macro).
- **Display**: ST7789 (condicional).

### 7.3 Detalhes por integracao

- **MQTT**
  - Inicio: `src/Platform/Espressif/Mqtt/EspIdfMqttClient.h`, `src/Core/Services/MqttService.h`.
  - Configura: `TransportController::configureMqtt()` em `src/App/Managers/TransportController.cpp` e settings em `src/Contracts/Settings/MqttSettings.h`.
  - Logs: `src/Core/Services/MqttService.cpp`.
  - Falhas: bloqueia conexao sem `ConnectivityGate` e `SettingsGate`; retries em backoff.

- **OTA (ArduinoOTA)**
  - Inicio: `src/Infra/OTA/OTA.h` e `src/Infra/OTA/OTA.cpp`.
  - Configura: `OTAManager` em `src/Infra/OTA/OTAManager.h` usa settings e gate.
  - Logs: `src/Infra/OTA/OTA.cpp` e `src/Infra/OTA/OTAManager.cpp`.
  - Falhas: se Wi-Fi ou settings nao prontos, OTA nao inicia; erros logados por callbacks.

- **HTTP Update (FirmwareUpdater)**
  - Inicio: `src/Infra/OTA/FirmwareUpdater.cpp`.
  - Configura: via `FirmwareConfig` em `src/Contracts/Settings/FirmwareConfig.h` e `OTAManager`.
  - Logs: `FW-OTA` em `src/Infra/OTA/FirmwareUpdater.cpp`.
  - Falhas: retries com limite; exige Wi-Fi conectado; valida SHA opcional.

- **Provisioning BLE**
  - Inicio: `src/Platform/Espressif/Provisioning/BleProvisioningChannel.h`.
  - Configura: habilitado por `BLE_PROVISIONING_CHANNEL_ENABLE` em `src/App/Managers/ProvisioningController.cpp`.
  - Logs: tags em `src/Platform/Espressif/Provisioning/BleProvisioningChannel.cpp`.
  - Falhas: depende de stack BLE do ESP32; sem BLE no ESP8266.

- **Provisioning Web Portal**
  - Inicio: `src/Platform/Arduino/Provisioning/WebPortalProvisioningChannel.h`.
  - Configura: `WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE` e `WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE`.
  - Logs: `src/Platform/Arduino/Provisioning/WebPortalProvisioningChannel.cpp`.
  - Falhas: depende de WebServer/UDP; captive portal opcional.

- **Settings (NVS + API)**
  - Inicio: `src/Platform/Espressif/Settings/Providers/EspIdfNvsSettingsProvider.h` e `src/Platform/Espressif/Settings/EspIdfSettingsFetcher.h`.
  - Configura: `SettingsManager::init()` e `SettingsManager::handle()` em `src/Core/Settings/SettingsManager.cpp`.
  - Logs: `SettingsManager`, `EspIdfNvsSettingsProvider`, `EspIdfSettingsFetcher`.
  - Falhas: cache ausente leva a provisioning; API invalida mantem NVS. Logs incluem dados sensiveis (ssid/password/key) em `src/App/Managers/ConnectivityBootstrap.cpp`.

- **Serial Transport**
  - Inicio: `src/Platform/Arduino/Transports/ArduinoSerialTransportChannel.h`.
  - Configura: `SmartSysApp::configureSerialTransport()` em `src/SmartSysApp.cpp`.
  - Logs: `TransportHub` e `ArduinoSerialTransportChannel`.
  - Falhas: depende de UART configurado e callback de dispatcher.

- **Sensores e Display**
  - Inicio: `src/Infra/Factories/SensorFactory.h`, `src/Platform/Arduino/Sensors/**`, `src/Infra/display/Display_ST7789_170_320.*`.
  - Configura: flags de build em `configs/base_esp.ini` e configs em `src/App/Builders/Configs/CapabilityConfig.h`.
  - Logs: variam por sensor (ex.: IR sensor loga via `src/Platform/Arduino/Sensors/ArduinoIRCommandSensor.cpp`).
  - Falhas: dependem de libs externas e flags; stubs existem para alguns sensores quando desabilitados.

## 8. Padroes de Codigo e Convencoes

### 8.1 Convencoes de nomes

- **Interfaces**: prefixo `I` (`src/Contracts/Capabilities/ICapability.h`, `src/Contracts/Providers/ITimeProvider.h`).
- **Implementacoes**: sufixos `Impl`, `Manager`, `Controller`, `Provider` (`src/Core/Settings/SettingsGateImpl.h`, `src/App/Managers/CapabilityController.h`).
- **Capabilities**: sufixo `Capability` (`src/Core/Capabilities/*Capability.cpp`, contratos em `src/Contracts/Capabilities/*Capability.h`).
- **Namespaces**: `iotsmartsys::core`, `iotsmartsys::app`, `iotsmartsys::platform`.

### 8.2 Padrao de configuracao

- **Settings central**: `SettingsManager` controla cache/API e sinaliza readiness via `SettingsGate`.
- **Persistencia**: NVS em `src/Platform/Espressif/Settings/Providers/EspIdfNvsSettingsProvider.cpp`.
- **Defaults**: definidos em structs de settings (ex.: `src/Contracts/Settings/MqttSettings.h`, `src/Contracts/Settings/WifiConfig.h`).
- **Aplicacao de settings**: `SmartSysApp::onSettingsUpdated()` em `src/SmartSysApp.cpp` reinicia quando ha mudancas.

### 8.3 Regras editoriais/filosofia

- Nao ha guia formal de estilo no repo.
- Padrao de evitar heap em capabilities: `CapabilitiesBuilder` aloca em arena (`src/App/Builders/Builders/CapabilitiesBuilder.h` e `src/SmartSysApp.h`).
- Uso de logger global (`Log::get()`) para evitar null durante init (`src/Contracts/Logging/Log.cpp`).

## 9. Como Estender

### 9.1 Nova Capability

1. Defina contrato (se publico) em `src/Contracts/Capabilities/`.
2. Implemente em `src/Core/Capabilities/` (pode usar helpers em `src/Core/Capabilities/CapabilityHelpers.h`).
3. Adicione config em `src/App/Builders/Configs/CapabilityConfig.h` (se precisar de parametros).
4. Registre no builder em `src/App/Builders/Builders/CapabilitiesBuilder.h`/`.cpp`.
5. Exponha no facade em `src/SmartSysApp.h`/`src/SmartSysApp.cpp`.

### 9.2 Novo Provider/Service

1. Crie interface em `src/Contracts/Providers/`.
2. Implemente em `src/Platform/**` ou `src/Core/**`.
3. Registre no `ServiceManager` em `src/Core/Providers/ServiceManager.cpp` e exponha no `ServiceProvider` se necessario (`src/Contracts/Providers/ServiceProvider.h`).

### 9.3 Novo firmware/app consumindo a lib

1. Crie um `main.cpp` no app com `SmartSysApp` (exemplo em `src/main.cpp`).
2. Configure capabilities via `add*Capability` de `SmartSysApp`.
3. Opcional: configurar transportes extras via `configureSerialTransport()`.
4. Use `platformio.ini` com envs apropriados (ex.: `env:esp32_dev`).

## 10. Apendice: Acoplamentos Perigosos (ESP32/FreeRTOS)

### 10.1 Headers de Core que incluem freertos/ ou esp-idf

- `src/Core/Settings/SettingsGateImpl.h` -> `freertos/FreeRTOS.h`, `freertos/semphr.h`.
- `src/Core/Services/IMqttClient.h` -> `mqtt_client.h` (ESP-IDF) e tipos `esp_err_t`.
- `src/Core/Providers/ServiceManager.h` -> providers ESP-IDF (NVS/Settings fetcher/parser) em `src/Platform/Espressif/Settings/**`.

### 10.2 Tipos FreeRTOS expostos em interfaces publicas

- `SemaphoreHandle_t` em `src/Core/Settings/SettingsGateImpl.h` (Core header).
- `EventGroupHandle_t` em `src/Platform/Espressif/Arduino/Connectivity/ArduinoEventLatch.h` e `src/Platform/Espressif/Connectivity/FreeRtosEventLatch.h`.
- `TaskHandle_t`/`SemaphoreHandle_t` em `src/Platform/Espressif/Settings/EspIdfSettingsFetcher.h`.

### 10.3 Casos em que o Core exige threading/RTOS

- `SettingsGateImpl` usa mutex FreeRTOS em `src/Core/Settings/SettingsGateImpl.cpp`.
- `SettingsManager` usa semaforo FreeRTOS em `src/Core/Settings/SettingsManager.cpp`.
- `IMqttClient` e `ServiceManager` acoplam ESP-IDF (MQTT/NVS) no Core.

### 10.4 Callback sob lock / secao critica

- Nao foram encontrados callbacks executados dentro de lock. `SettingsGateImpl` e `SettingsManager` liberam lock antes de chamar callbacks (`src/Core/Settings/SettingsGateImpl.cpp`, `src/Core/Settings/SettingsManager.cpp`).

## 11. Riscos e Plano de Migracao

### 11.1 Top 10 riscos tecnicos

1. **Core acoplado a ESP-IDF/FreeRTOS**: `src/Core/Services/IMqttClient.h` e `src/Core/Settings/SettingsGateImpl.h` impedem build limpo em ESP8266.
2. **DeviceIdentityProvider ESP32-only**: `src/Platform/Espressif/Providers/DeviceIdentityProvider.cpp` usa `esp_efuse`; impede OTA hostname no ESP8266.
3. **ServiceManager inclui Platform**: `src/Core/Providers/ServiceManager.h` puxa implementacoes de plataforma direto no Core.
4. **Contracts com includes de WiFi**: `src/Contracts/Connections/WiFiManager.h` inclui `WiFi.h`/`ESP8266WiFi.h`.
5. **MQTT apenas ESP32**: `EspIdfMqttClient` e `IMqttClient` so existem quando `ESP32` definido.
6. **SettingsFetcher usa task FreeRTOS**: `src/Platform/Espressif/Settings/EspIdfSettingsFetcher.cpp`.
7. **Logs de credenciais**: `src/App/Managers/ConnectivityBootstrap.cpp` loga SSID/password/API key (exposicao de dados).
8. **Headers pesados no facade**: `src/SmartSysApp.h` inclui Platform/Infra, aumentando acoplamento e tempo de build.
9. **Provisioning BLE hard-coded**: `src/Platform/Espressif/Provisioning/BleProvisioningChannel.cpp` usa ESP-IDF BLE; sem fallback.
10. **Ambiente de testes possivel inconsistente**: `configs/esp32s3-test.ini` referencia `env:base32` nao existente.

### 11.2 Plano de migracao para ESP8266 (sem implementar)

- **Fase 0: build passa (headers)**
  - Criar stubs/abstractions para ESP32-only (`src/Core/Services/IMqttClient.h`, `src/Platform/Espressif/Providers/DeviceIdentityProvider.*`).
  - Mover includes ESP-IDF para .cpp ou PIMPL.
  - Introduzir interfaces neutras em `src/Contracts/**`.

- **Fase 1: OS abstraction minima (mutex/timer/scheduler)**
  - Criar camada `Core/Runtime` com `IMutex`, `IEventLatch`, `ITimer`.
  - Trocar `SettingsGateImpl` e `SettingsManager` para usar essa abstracao (sem FreeRTOS direto).
  - Implementar para ESP32 (FreeRTOS) e ESP8266 (loop/Arduino).

- **Fase 2: remover FreeRTOS do Core**
  - Remover `freertos/*` de `src/Core/**`.
  - Mover `SettingsGateImpl` para Platform e expor apenas interface em Contracts.
  - Ajustar `ServiceManager` para injetar implementacoes por plataforma.

- **Fase 3: feature flags e matriz por target**
  - Definir flags explicitas para MQTT/OTA/Provisioning por target em `platformio.ini`.
  - Criar um MQTT client para ESP8266 (ex.: PubSubClient) implementando `ITransportChannel`.
  - Separar `DeviceIdentityProvider` por plataforma.

- **Fase 4: testes e validacao**
  - Testes unitarios (Unity) para SettingsGate/SettingsManager com mocks.
  - Testes de integracao: Wi-Fi + MQTT connect, OTA HTTP, provisioning web.
  - Matriz CI por target (ESP32, ESP32S3, ESP8266).
