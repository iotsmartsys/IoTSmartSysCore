# Especificação — FanCapability

**ID:** `IOTSSC-FAN-CAPABILITY`

**Classe da fonte:** Normativa

**Versão:** 0.1

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Não pronta [`Not Ready`]

**Revisão de implementabilidade:** Pendente [`Pending Review`]

**Bloqueio arquitetural:** Nenhum conhecido antes da análise formal

**Relações normativas e de dependência:**

- Nova [`New`] — `FanCapability`, `FanConfig`, API pública e exemplo
  executável;
- Preserva `IOTSSC-PUBLIC-API`, `IOTSSC-RUNTIME` e
  `IOTSSC-HW-EXAMPLES`;
- Submete-se a `IOTSSC-BINARY-COMMAND-STATE@0.6` por derivar de
  `BinaryCommandCapability`.

## 1. Objetivo e contexto

Adicionar ao IoTSmartSysCore uma capability binária específica para ventilador,
com o type público exato `"Fan Actuator"`. Seu comportamento deve ser o mesmo
de `SwitchCapability`, sem reutilizar `SwitchConfig` e sem representar o
ventilador publicamente como um switch genérico.

O primeiro recorte controla somente alimentação binária por um Hardware
Adapter de saída digital. Controle de velocidade, rotação e automação térmica
não integram esta versão.

## 2. Escopo

- classe pública `FanCapability`;
- constante pública `FAN_ACTUATOR_TYPE` com o valor exato
  `"Fan Actuator"`;
- estados `off` e `on` e operações binárias equivalentes às de
  `SwitchCapability`;
- classe pública própria `FanConfig`, sem aceitar `SwitchConfig` como contrato
  de configuração da nova capability;
- registro aditivo no builder e na fachada `SmartSysApp`;
- ownership em arena, falha atômica, identidade e lifecycle vigentes;
- participação automática na persistência de comandos binários;
- exemplo executável `fan` na MCB R1 usando a saída de relé oficial;
- documentação de montagem, uso e segurança do exemplo.

## 3. Fora de escopo

- controle de velocidade ou potência por PWM;
- seleção de velocidade discreta;
- inversão do sentido de rotação;
- leitura de RPM, tacômetro ou detecção de travamento;
- controle térmico automático, termostato ou associação com sensores;
- nova abstração genérica de motor, carga ou atuador;
- alteração de `SwitchCapability`, `SwitchConfig` ou seus consumidores;
- alteração do pinout oficial da MCB R1;
- alteração do limite de oito capabilities ou do lifecycle cooperativo;
- suporte a ESP8266 ou promoção de ESP-IDF nativo a runtime suportado;
- criação, ampliação, reestruturação ou correção de artefatos de teste.

### 3.1 Arquitetura e organização

**Precedente aplicável:** `SwitchCapability`, incluindo a derivação de
`BinaryCommandCapability`, o uso do adapter de saída, o registro pelo builder e
a exposição pela fachada `SmartSysApp`.

**Elementos preservados:** separação Contracts/Core/App/Platform,
`ICommandHardwareAdapter`, arena de ownership da aplicação, configuração antes
de `SmartSysApp::setup()`, processamento cooperativo de `handle()`, limite de
oito capabilities e identidade pública imutável após registro.

**Desvio arquitetural explícito:** nenhum. A especialização adiciona nomes e
tipos públicos próprios sobre mecanismos existentes.

### 3.2 Limite de escopo funcional

**Capacidades arquiteturais pressupostas:** registro e lifecycle vigentes,
Hardware Adapter digital de saída e persistência já contratada para toda classe
derivada de `BinaryCommandCapability`.

**Preparação arquitetural separada:** não aplicável. A funcionalidade não cria
novo lifecycle, ownership, protocolo, persistência ou política transversal.

## 4. Requisitos

### 4.1 Capability e comportamento binário

- **FAN-001:** deve existir uma classe pública `FanCapability` derivada de
  `BinaryCommandCapability`.
- **FAN-002:** o type público da capability deve ser exatamente
  `"Fan Actuator"`, exposto pela constante `FAN_ACTUATOR_TYPE`.
- **FAN-003:** o estado desligado deve ser `"off"` e o estado ligado deve ser
  `"on"`, com estado inicial desligado conforme o precedente de
  `SwitchCapability` e do adapter de saída vigente.
- **FAN-004:** `FanCapability` deve disponibilizar as operações públicas
  `turnOn()`, `turnOff()`, `toggle()`, `power()` e `isOn()` com a mesma
  semântica observável de `SwitchCapability`.
- **FAN-005:** comandos aceitos devem alcançar o Hardware Adapter, ser
  confirmados por read-back e atualizar ou publicar somente o estado confirmado,
  conforme `BinaryCommandCapability`.
- **FAN-006:** `setup()` e `handle()` devem preservar a inicialização e a
  sincronização cooperativa vigentes, sem espera ativa, task ou temporizador
  próprio da nova classe.
- **FAN-007:** por derivar de `BinaryCommandCapability`, toda instância de
  `FanCapability` deve participar automaticamente da restauração e persistência
  de estado definida por `IOTSSC-BINARY-COMMAND-STATE@0.6`, sem opt-in próprio.

### 4.2 Configuração própria

- **FAN-008:** deve existir uma classe pública `iotsmartsys::app::FanConfig`,
  distinta de `SwitchConfig` e derivada de `HardwareConfig`.
- **FAN-009:** `FanConfig` deve herdar os construtores públicos de
  `HardwareConfig` e expor somente o contrato vigente de saída binária:

```text
GPIO
highIsOn
capability_name
```

- **FAN-010:** os defaults devem permanecer `highIsOn = true` e
  `capability_name = nullptr`; `GPIO` não possui default operacional válido.
- **FAN-011:** nenhuma API de criação de `FanCapability` pode exigir nem aceitar
  `SwitchConfig` no lugar de `FanConfig`.

### 4.3 API, registro e ownership

- **FAN-012:** a API pública da fachada deve ser:

```cpp
FanCapability *
SmartSysApp::addFanCapability(FanConfig config);
```

- **FAN-013:** o builder deve expor registro equivalente que receba
  `const FanConfig&`, resolva a identidade com `FAN_ACTUATOR_TYPE`, crie o
  adapter de saída com `GPIO` e `highIsOn` e registre a `FanCapability`.
- **FAN-014:** a configuração e o registro devem ocorrer antes de
  `SmartSysApp::setup()` e preservar o limite de oito capabilities.
- **FAN-015:** o ponteiro retornado deve ser não proprietário e estável durante
  a vida da aplicação; capability e adapter permanecem sob ownership da
  aplicação.
- **FAN-016:** identidade ausente deve seguir a geração automática vigente.
  Identidade duplicada, vazia depois de resolvida ou acima dos limites
  públicos, falta de slot, falha de adapter ou falta de arena deve retornar
  `nullptr` sem consumir slot, registrar identidade ou deixar objeto parcial.
- **FAN-017:** o registro aditivo não pode alterar assinaturas, defaults ou
  comportamento de capabilities preexistentes.

### 4.4 Exemplo executável

- **FAN-018:** o catálogo deve receber o exemplo `fan`, composto por aplicação
  Arduino única, README, seletor exclusivo no runner e environment estável
  `example_fan_mcb_r1`.
- **FAN-019:** o exemplo deve consumir exclusivamente a API pública
  `SmartSysApp::addFanCapability()` com `FanConfig`, antes de
  `SmartSysApp::setup()`.
- **FAN-020:** a saída deve usar diretamente o símbolo oficial
  `ITS_MCB01_RELAY_PIN`; a ausência do símbolo deve causar erro de build
  compreensível. GPIO literal ou macro paralela de pino é proibido.
- **FAN-021:** o exemplo deve configurar explicitamente nível ativo alto,
  identificar a capability como `fan` e iniciar com o estado seguro desligado.
- **FAN-022:** o boot deve informar o identificador do exemplo, a placa, o GPIO
  resolvido, o nível ativo e o nome da capability, sem expor segredo.
- **FAN-023:** o `loop()` deve chamar `SmartSysApp::handle()` continuamente. O
  exemplo não pode duplicar aplicação de comando, sincronização, persistência
  ou lógica interna da capability.
- **FAN-024:** o README deve documentar API, placa, pino, nível ativo, ligação
  do ventilador por driver/relé adequado, comandos de build/upload/monitor,
  procedimento manual, resultado esperado e riscos elétricos e mecânicos.
- **FAN-025:** o environment do exemplo não pode alterar o build padrão,
  selecionar mais de uma aplicação ou redefinir símbolos do pinout oficial.

## 5. Fluxo e estados

```text
FanConfig antes de setup
→ validação atômica de identidade, slot, adapter e arena
→ construção de FanCapability com type "Fan Actuator"
→ setup do adapter no estado off
→ restauração binária válida, quando existente
→ handle cooperativo
→ comando on/off/toggle/power
→ aplicação no adapter e confirmação por read-back
→ atualização e publicação do estado confirmado
→ solicitação assíncrona de persistência quando aplicável
```

Exemplo normativo de estado ligado:

```json
{
  "capability_name": "fan",
  "type": "Fan Actuator",
  "value": "on"
}
```

## 6. Falhas e condições de borda

- comando rejeitado ou não confirmado não pode ser promovido a estado lógico;
- mudança externa observada no adapter deve sincronizar a capability pelo
  caminho comum de `BinaryCommandCapability`;
- repetição do estado corrente não deve publicar nem persistir nova transição;
- ausência ou falha do storage binário preserva o comportamento definido por
  `IOTSSC-BINARY-COMMAND-STATE@0.6` e não impede o controle em memória;
- polaridade ativa baixa deve inverter apenas o nível elétrico do adapter, sem
  inverter os estados lógicos `off` e `on`;
- falha de registro deve ser atômica e observável por `nullptr` e logging
  vigente;
- rollover do tempo global não cria requisito próprio porque a nova classe não
  introduz relógio ou cadência adicional;
- o ventilador não pode ser energizado implicitamente pela mera construção do
  exemplo.

## 7. Critérios de aceite e validações

### FAN-AC-001 — Identidade e configuração próprias

**Cobre:** FAN-001, FAN-002 e FAN-008 a FAN-013.

- **Dado que** uma aplicação cria `FanConfig` com nome `fan`, GPIO válido e
  nível ativo configurado;
- **Quando** chama `SmartSysApp::addFanCapability()` antes de `setup()`;
- **Então** recebe uma `FanCapability` com nome `fan`, type exato
  `Fan Actuator` e adapter configurado pelos valores de `FanConfig`, sem usar
  `SwitchConfig`;
- **Evidência:** inspeção de API e execução instrumentada do builder, sem criar
  artefato de teste automatizado.

### FAN-AC-002 — Equivalência comportamental com SwitchCapability

**Cobre:** FAN-003 a FAN-006.

- **Dado que** a capability começa em `off` e o adapter permite confirmar ambos
  os níveis;
- **Quando** são exercidos `turnOn`, `turnOff`, `toggle`, `power` e uma mudança
  externa do adapter;
- **Então** aplicação, read-back, estado lógico, `isOn()` e publicação seguem a
  mesma sequência e resultados de `SwitchCapability`, usando somente `off` e
  `on`;
- **Evidência:** execução instrumentada com adapter e sink observáveis, sem
  registrar suíte automatizada.

### FAN-AC-003 — Persistência herdada

**Cobre:** FAN-007.

- **Dado que** existe snapshot válido com estado oposto ao default para a
  identidade `fan`/`Fan Actuator`;
- **Quando** a capability executa `setup()`, confirma a restauração, sofre uma
  transição estável posterior e o escritor alcança quiescência;
- **Então** restaura e persiste pelo protocolo comum sem configuração opt-in,
  leitura NVS em `handle()` ou caminho próprio da classe concreta;
- **Evidência:** inspeção da derivação e execução instrumentada pelos seams
  vigentes; nenhum artefato de teste integra esta versão.

### FAN-AC-004 — Registro atômico e lifecycle

**Cobre:** FAN-014 a FAN-017.

- **Dado que** são tentados separadamente registro válido, identidade duplicada,
  identidade excedente, falta de slot e falta de arena;
- **Quando** as solicitações ocorrem antes de `setup()` e o ciclo cooperativo é
  executado;
- **Então** somente o registro válido ocupa capability, adapter, arena e
  identidade; cada falha retorna `nullptr` sem efeito parcial e o `handle()`
  continua cooperativo;
- **Evidência:** inspeção e execução instrumentada do builder.

### FAN-AC-005 — Exemplo público

**Cobre:** FAN-018 a FAN-025.

- **Dado que** o environment `example_fan_mcb_r1` seleciona a MCB R1;
- **Quando** o firmware é construído e executado com uma carga segura por
  driver ou relé adequado;
- **Então** existe um único `setup()`/`loop()`, o GPIO vem de
  `ITS_MCB01_RELAY_PIN`, o boot identifica a configuração, a saída inicia
  desligada e acompanha comandos válidos de ligar e desligar;
- **Evidência:** build do exemplo, inspeção do ELF e validação manual em
  hardware conforme o README.

### 7.1 Evidências planejadas

- **Artefatos de teste no recorte:** nenhum, por decisão explícita do
  Arquiteto;
- inspeções e execuções instrumentadas podem usar seams existentes sem criar,
  ampliar, reestruturar ou corrigir testes;
- build canônico `pio run -e esp32_dev` durante a Implementação;
- build `pio run -e example_fan_mcb_r1` durante a Implementação;
- inspeção do firmware do exemplo para exatamente um `setup()` e um `loop()`;
- upload, monitor e validação manual em hardware dependem de ordem operacional
  explícita e permanecem evidência posterior;
- ausência de execução instrumentada ou física deve ser registrada como
  `Not Executed`, nunca convertida em aprovação.

## 8. Conhecimento afetado

- adicionar esta fonte ao índice de autoridade e à cobertura de capabilities
  no `docs/rfc/KNOWLEDGE-MAP.md`;
- registrar autoria, análise, implementação e revisão como transações EKOM
  separadas;
- atualizar runner, configuração e catálogo de exemplos somente na
  Implementação;
- preservar relatórios, especificações e transações históricas.

## 9. Relações, decisões, lacunas e débitos

**Fatos observados:** `SwitchCapability` deriva de
`BinaryCommandCapability`, usa `STATE_OFF`, `STATE_ON` e `SWITCH_TYPE`, e é
registrada por `CapabilitiesBuilder` a partir de `SwitchConfig`. O runtime já
possui `ICapabilityType::FanActuator`, mas não possui `FanCapability`,
`FAN_ACTUATOR_TYPE`, `FanConfig` ou API de registro correspondente.
`src/main.cpp` usa atualmente um `SwitchConfig` nomeado `fan-1`, o que demonstra
o uso conceitual que esta extensão torna explícito. O catálogo executável e o
pinout da MCB R1 já oferecem o precedente e o símbolo
`ITS_MCB01_RELAY_PIN`.

**Intenção e decisões confirmadas:** a nova capability deve comportar-se como
`SwitchCapability`, possuir configuração própria, usar o type exato
`Fan Actuator`, incluir exemplo executável no padrão de
`VoltageSensorCapability` e não contratar criação ou alteração de testes.

**Solução proposta:** especialização aditiva de `BinaryCommandCapability`,
configuração própria derivada de `HardwareConfig`, registro pelo builder e
fachada existentes e exemplo binário na saída oficial de relé da MCB R1.

**Decisões pendentes:** nenhuma conhecida no contrato registrado.

**Relações:** `IOTSSC-BINARY-COMMAND-STATE@0.6`, `IOTSSC-PUBLIC-API`,
`IOTSSC-RUNTIME`, `IOTSSC-HW-EXAMPLES` e `EKOM-CHG-0008`.

**ADRs relacionadas:** nenhuma; não foi identificada mudança arquitetural
durável separável da funcionalidade.

**Autoridades confrontadas:** `AGENTS.md`,
`docs/rfc/EKOM-GUIDELINES.md`, `docs/rfc/KNOWLEDGE-MAP.md`,
`PUBLIC-API-COMPATIBILITY.md`, `CORE-RUNTIME-LIFECYCLE.md`,
`EXECUTABLE-HARDWARE-EXAMPLES.md` e
`BINARY-COMMAND-STATE-PERSISTENCE.md`.

**Relatórios esperados:** análise de implementabilidade, implementação e
revisão; validação física posterior conforme risco e autorização.

## 10. Estado da especificação

A versão 0.1 está em Rascunho [`Draft`], com implementação Não iniciada
[`Not Started`], entrega Não pronta [`Not Ready`] e revisão de
implementabilidade Pendente [`Pending Review`]. A passagem à Implementação
depende de análise formal `Ready` da versão corrente e de ordem explícita do
Arquiteto.
