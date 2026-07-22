# Especificação — Ciclo de Vida do Runtime

**ID:** IOTSSC-RUNTIME

**Estado normativo:** Active

**Estado de implementação:** Implemented

**Última atualização:** 22/07/2026

## 1. Objetivo

Definir o ciclo de vida esperado do `SmartSysApp` e preservar os comportamentos que sustentam os firmwares atuais.

## 2. Requisitos

- **RUN-001:** hardware, transports opcionais e capabilities devem ser configurados antes de `app.setup()`.
- **RUN-002:** `setup()` deve inicializar os serviços básicos, identidade, settings e conectividade necessários ao bootstrap.
- **RUN-003:** o bootstrap deve selecionar explicitamente o fluxo de provisioning ou o fluxo operacional conforme o estado persistido.
- **RUN-004:** no fluxo operacional, capabilities, dispatcher, MQTT e tarefas devem ser inicializados em ordem segura.
- **RUN-005:** MQTT é o transporte principal; UART permanece auxiliar e não deve alterar esse papel.
- **RUN-006:** `handle()` deve manter o processamento cooperativo necessário a conectividade, provisioning, factory reset, transports, settings, registration, capabilities e OTA.
- **RUN-007:** enquanto o dispositivo estiver em provisioning, o fluxo operacional incompatível não deve avançar.
- **RUN-008:** tarefas de rede e transporte podem ser executadas por FreeRTOS; se sua criação falhar, o processamento deve continuar pelo fallback existente no loop principal.
- **RUN-009:** conexão MQTT bem-sucedida deve executar o anúncio vigente do dispositivo.
- **RUN-010:** atualização válida de settings deve interromper MQTT quando necessário e provocar reinicialização controlada.
- **RUN-011:** falhas transitórias de conectividade não devem corromper settings, capabilities ou identidade.
- **RUN-012:** o limite de oito capabilities e a configuração completa antes de `setup()` são invariantes do runtime.

## 3. Fluxo conceitual

```text
construção
→ configuração de hardware/transports/capabilities
→ setup
→ bootstrap de provisioning ou operacional
→ criação de tarefas, quando possível
→ handle contínuo
→ reboot controlado quando requerido por settings
```

## 4. Fora de escopo

- mudar políticas de provisioning, OTA ou factory reset;
- especificar detalhadamente MQTT, Wi-Fi ou HTTP/HTTPS;
- tornar ESP-IDF nativo uma plataforma suportada;
- permitir configuração tardia de capabilities.

## 5. Critérios de aceite

- build do exemplo oficial Arduino/ESP32;
- testes dos ramos de bootstrap e fallback de tasks;
- testes da atualização de settings com reboot;
- validação de conectividade MQTT, anúncio e fluxo cooperativo;
- teste em hardware para mudanças no ciclo de vida.

## 6. Estado conhecido

O comportamento foi mapeado na implementação atual. A cobertura automatizada ainda não comprova todos os ramos críticos; portanto o estado permanece `Implemented`.

## 7. Relações

- `PUBLIC-API-COMPATIBILITY.md`;
- futuras especificações de settings, Wi-Fi/MQTT, provisioning e OTA;
- `EKM-GAP-0003`.
