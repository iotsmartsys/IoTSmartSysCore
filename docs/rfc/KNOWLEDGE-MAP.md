# Knowledge Map — IoTSmartSysCore

**Status:** Active

**Última atualização:** 30/07/2026

## 1. Governança

| Área | Fonte | Tipo | Estado |
|---|---|---|---|
| Instruções e roteamento para agentes | `AGENTS.md` | Normativo | Active — EKM 1.17 |
| Adaptador para Claude Code | `CLAUDE.md` | Operacional | Active |
| Diretrizes locais | `docs/rfc/EKM-GUIDELINES.md` | Normativo | Active — EKM 1.17 |
| Mapa de conhecimento | `docs/rfc/KNOWLEDGE-MAP.md` | Normativo | Active |
| Histórico e transações | `docs/rfc/EKM-CHANGELOG.md` | Operacional | Active |

## 2. Fontes normativas

| Domínio | Fonte | Estado normativo | Implementação |
|---|---|---|---|
| Governança EKM 1.17 | `docs/rfc/EKM-GUIDELINES.md` | Active | Implemented |
| API pública e compatibilidade | `docs/specs/PUBLIC-API-COMPATIBILITY.md` | Active | Implemented |
| Ciclo de vida do runtime | `docs/specs/CORE-RUNTIME-LIFECYCLE.md` | Active | Implemented |
| Release e distribuição | `docs/specs/RELEASE-AND-DISTRIBUTION.md` | Active | In Progress |
| Exemplos executáveis e hardware | `docs/specs/EXECUTABLE-HARDWARE-EXAMPLES.md` | Active | Implemented |
| Estado do controle de garagem | `docs/specs/GARAGE-CONTROL-STATE.md` | Active | Validated |
| Persistência de comandos binários | `docs/specs/BINARY-COMMAND-STATE-PERSISTENCE.md` | Proposed | Implemented |

`docs/REPO_DOSSIER.md` é material informativo legado e não prevalece sobre as fontes acima.

## 3. Cobertura de adoção

| Domínio | Cobertura | Entradas principais | Observação |
|---|---|---|---|
| API pública | Specified | `src/SmartSysApp.*`, builders, interfaces, configs | Compatibilidade exige validação dedicada |
| Runtime principal | Specified | `src/main.cpp`, `src/SmartSysApp.cpp` | Arduino sobre ESP32 |
| Capabilities | Specified | builders, adapters e contracts | Controle de garagem ativo; persistência de comandos binários implementada (`BCS-DEC-001` pendente, não bloqueante; validação em hardware pendente); limite intencional de 8 |
| Settings e API HTTP/HTTPS | Mapped | settings, API e storage | Histórico de regressões; falta especificação profunda |
| Wi-Fi e MQTT | Mapped | connectivity e transport | MQTT é transporte principal |
| UART | Inventoried | serial transport | Transporte auxiliar |
| Provisioning e factory reset | Mapped | bootstrap e platform services | Requer especificação própria quando tocado |
| OTA | Inventoried | serviços OTA | Sem especificação própria |
| Plataformas | Mapped | `src/Platform/Arduino`, `src/Platform/Espressif`, legado ESP8266 | ESP-IDF é preparação futura; ESP8266 não é suportado |
| Build e release | Specified | `platformio.ini`, `Makefile`, `.github/workflows/` | Existem desvios abertos |
| Testes | Inventoried | `test/` | Cobertura concentrada em builders/settings |
| Exemplos executáveis | Specified | `src/ExecutableExampleRunner.cpp`, `examples/executable/`, `configs/executable_examples.ini` | Technical Readiness `Implementable`; correção de pinout implementada e validada estaticamente; validação física pendente |

## 4. Lacunas

### EKM-GAP-0001 — Evidência de compatibilidade pública

**Estado:** Open

Criar matriz de compatibilidade e validação representativa para `SmartSysApp`, `SensorFactory`, interfaces e configs.

### EKM-GAP-0002 — Release divergente

**Estado:** Open

O release não impede execução fora de `main` e há divergência entre os caminhos do header de versão usados pelo `Makefile` e pelo workflow.

### EKM-GAP-0003 — Domínios críticos ainda não especificados

**Estado:** Open

Settings, API HTTP/HTTPS, Wi-Fi e MQTT possuem histórico de regressão e precisam de especificações incrementais antes de mudanças relevantes.

### EKM-GAP-0004 — Fronteiras de plataforma

**Estado:** Open

Classificar formalmente o código preparatório para ESP-IDF e o código legado ESP8266 antes de remoções ou expansão de suporte.

### EKM-GAP-0005 — Dossiê histórico

**Estado:** Open

Revisar `docs/REPO_DOSSIER.md`, corrigir referências obsoletas e decidir se partes devem migrar para especificações.

### EKM-GAP-0006 — Catálogo inicial de exemplos

**Estado:** Closed

Foram definidos `iotsmartsys_mcb_r1`, os exemplos `basic_light` e `environment_dht`, o uso privado da infraestrutura real e a compilação dos dois environments em CI. A especificação foi promovida para `Active`.

### EKM-GAP-0007 — Conformidade dos exemplos com o pinout MCB R1

**Estado:** Closed

Uma nova revisão integral válida declarou a especificação `Implementable` antes da correção. `basic_light` e `environment_dht` passaram a consumir diretamente `ITS_MCB01_RELAY_PIN` e `ITS_MCB01_TEMPERATURE_SENSOR_PIN`; macros locais e GPIOs literais foram removidos dos environments. Builds e verificações estáticas aprovaram a conformidade de pinout. A validação física geral da especificação permanece pendente em `EKM-CHG-0002`, mas não constitui lacuna do contrato de pinout.

### EKM-GAP-0008 — EKM Gate e garantias automatizadas

**Estado:** Superseded

O modelo 1.9 não incorpora `EKM Gate`, orquestração ou garantias automatizadas ao
fluxo vigente. Qualquer adoção futura dependerá de nova decisão do Arquiteto e
de evidência proporcional ao problema; nenhuma garantia automática está
implantada atualmente.

### EKM-GAP-0009 — Validação da máquina de estados da garagem

**Estado:** Closed

A máquina de estados conforme `IOTSSC-GARAGE-CONTROL` foi implementada e
validada em ambiente de hardware conforme declaração do Arquiteto. A limitação
preexistente do environment automatizado permanece registrada em
`EKM-CHG-0007`, mas não mantém aberta a lacuna de validação física.

## 5. Baseline inicial

- Branch: `main`.
- Commit: `0c6d5e63eb09d826beba2e16a3085c1a8f814668`.
- Worktree inicial da adoção: limpo.
- Runtime suportado: Arduino sobre ESP32.
- ESP8266: não suportado.
- Release: tags na branch `main`, com publicação pelo GitHub Actions no PlatformIO.

## 6. Evolução da governança

- `EKM-CHG-0003`: introduziu Technical Readiness Review binária e atomicidade da especificação antes da implementação.
- `EKM-CHG-0004`: introduziu imutabilidade normativa em produção, estado de entrega e previsão do futuro `EKM Gate`.
- `EKM-CHG-0005`: tornou a Technical Readiness Review cumulativa, exigiu matriz completa, separou revisão e implementação em execuções distintas e reservou ao arquiteto a autorização para implementar.
- `EKM-CHG-0006`: adotou o modelo EKM 1.9, removeu controles universais sem
  evidência de ganho e transferiu a linhagem técnica documental para o Git.
- `EKM-CHG-0008`: adotou o modelo EKM 1.17, o fluxo sequencial por atores,
  preservação arquitetural explícita, gate de encerramento das execuções,
  Consultor de Arquitetura subordinado e adaptador `CLAUDE.md`.
- `EKM-CHG-0009`: especifica persistência NVS e restauração no boot para
  capabilities derivadas de `BinaryCommandCapability`; a revisão técnica
  declarou o recorte `Implementable`.
