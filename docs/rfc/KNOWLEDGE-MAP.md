# Knowledge Map — IoTSmartSysCore

**Status:** Active

**Última atualização:** 01/08/2026 (revisão de implementabilidade da persistência binária 0.6)

## 1. Governança

| Área | Fonte | Tipo | Estado |
|---|---|---|---|
| Instruções e roteamento para agentes | `AGENTS.md` | Normativo | Active — EKM 1.19 |
| Adaptador para Claude Code | `CLAUDE.md` | Operacional | Active |
| Diretrizes locais | `docs/rfc/EKM-GUIDELINES.md` | Normativo | Active — EKM 1.19 |
| Mapa de conhecimento | `docs/rfc/KNOWLEDGE-MAP.md` | Normativo | Active |
| Histórico e transações | `docs/rfc/EKM-CHANGELOG.md` | Operacional | Active |

## 2. Fontes normativas

| Domínio | Fonte | Estado normativo | Implementação |
|---|---|---|---|
| Governança EKM 1.19 | `docs/rfc/EKM-GUIDELINES.md` | Active | Implemented |
| API pública e compatibilidade | `docs/specs/PUBLIC-API-COMPATIBILITY.md` | Active | Implemented |
| Ciclo de vida do runtime | `docs/specs/CORE-RUNTIME-LIFECYCLE.md` | Active | Implemented |
| Release e distribuição | `docs/specs/RELEASE-AND-DISTRIBUTION.md` | Active | In Progress |
| Exemplos executáveis e hardware | `docs/specs/EXECUTABLE-HARDWARE-EXAMPLES.md` | Active | Implemented |
| Estado do controle de garagem | `docs/specs/GARAGE-CONTROL-STATE.md` | Active | Validated |
| Persistência de comandos binários | `docs/specs/BINARY-COMMAND-STATE-PERSISTENCE.md` | Proposed | Not Started (versão 0.6) — revisão `Needs Clarification` em `EKM-CHG-0023`; decisão parcial `EKM-CHG-0024` torna `rename()`/`applyRenamedName()` obsoletos, mas `BCS-DEC-006`/`EKM-GAP-0011` continuam bloqueando chamadas legadas e campos públicos mutáveis; limites 63/31 permanecem definidos; baseline `esp32_dev` continua dependência separada conforme `BCS-DEC-003` |

`docs/REPO_DOSSIER.md` é material informativo legado e não prevalece sobre as fontes acima.

## 3. Cobertura de adoção

| Domínio | Cobertura | Entradas principais | Observação |
|---|---|---|---|
| API pública | Specified | `src/SmartSysApp.*`, builders, interfaces, configs | Compatibilidade exige validação dedicada |
| Runtime principal | Specified | `src/main.cpp`, `src/SmartSysApp.cpp` | Arduino sobre ESP32 |
| Capabilities | Specified | builders, adapters e contracts | Controle de garagem ativo; persistência binária 0.6 `Proposed`/`Not Started`/`Needs Clarification` (`EKM-CHG-0023`); limites 63/31 estão definidos, mas `BCS-DEC-006` deve decidir imutabilidade ou semântica da mutação pública após registro; os demais contratos corrigem NVS global, validade estrutural/semântica, valve, provisioning e cooperatividade do writer |
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

### EKM-GAP-0010 — Limite público da identidade de capability

**Estado:** Closed

O Arquiteto confirmou em `BCS-DEC-005` os limites públicos de 63 bytes para o
`capability_name` definitivo e 31 bytes para `type`, excluídos os terminadores,
com rejeição observável antes do registro e sem efeito parcial. A versão 0.6
incorpora limites, compatibilidade, adequação de consumidores excedentes e
critérios assertáveis; a lacuna está encerrada.

### EKM-GAP-0011 — Mutabilidade da identidade após registro

**Estado:** Open

Capabilities já registradas continuam expondo `capability_name` e `type` como
campos públicos mutáveis, além de métodos de renomeação sem retorno de erro. O
Arquiteto deve decidir se a identidade será imutável após registro e como
migrar a API, ou definir validação, identidade prevalente, erro e preservação de
efeitos para mutações suportadas. A ausência bloqueia BCS-002, BCS-022,
BCS-AC-002 e BCS-AC-021. O Arquiteto já decidiu em `EKM-CHG-0024` marcar
`rename()` e `applyRenamedName()` como obsoletos; comportamento legado e
atribuição direta aos campos continuam pendentes.

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
- `EKM-CHG-0011`: adota a EKM 1.18 e exige critérios de aceite assertáveis,
  sem confundir compilação com execução.
- `EKM-CHG-0012`: adota a EKM 1.19 e torna operacional no papel do Autor a
  elaboração rastreável, falsificável e independente dos critérios de aceite.
- `EKM-CHG-0013`: corrige a especificação de persistência binária como versão
  0.2, com critérios assertáveis, e a devolve para análise independente.
- `EKM-CHG-0014`: revisão independente de implementabilidade da versão 0.2,
  declarada `Implementable` sem decisão normativa ausente além de
  `BCS-DEC-001` (não bloqueante).
- `EKM-CHG-0015`: implementação parcial da versão 0.2 preservada como
  `In Progress`, sem satisfazer o gate de testes.
- `EKM-CHG-0016`: validação consultiva confronta os 22 critérios e encontra 1
  aprovado, 7 reprovados e 14 não verificados.
- `EKM-CHG-0017`: revisão de implementabilidade da versão 0.3, historicamente
  `Implementable` e depois contestada por `EKM-CHG-0018`.
- `EKM-CHG-0018`: avaliação consultiva registra riscos de hardware e
  inconsistências da revisão 0.3; solicita correções de autoria.
- `EKM-CHG-0019`: autoria da versão 0.4 da persistência binária, incorporando
  `EKM-CHG-0018`; revisão de implementabilidade reinstaurada como
  `Pending Review`.
- `EKM-CHG-0020`: decisões arquiteturais promovem documentalmente a
  persistência binária para a versão 0.5, encerram os bloqueios de `blink`, gate
  e contexto NVS e preservam a revisão como `Pending Review`.
- `EKM-CHG-0021`: revisão integral da versão 0.5 resulta em `Needs
  Clarification`; `EKM-GAP-0010` registra a decisão ausente sobre o limite
  público de `capability_name`, e o baseline `esp32_dev` falho permanece
  dependência separada conforme `BCS-DEC-003`.
- `EKM-CHG-0022`: autoria da versão 0.6 incorpora `BCS-DEC-005`, publica
  limites de identidade de 63/31 bytes com rejeição observável pré-registro,
  encerra `EKM-GAP-0010` e restaura a revisão como `Pending Review`.
- `EKM-CHG-0023`: revisão integral da versão 0.6 resulta em `Needs
  Clarification`; `EKM-GAP-0011` registra a decisão ausente sobre mutação da
  identidade pública depois do registro.
- `EKM-CHG-0024`: decisão parcial do Arquiteto marca `rename()` e
  `applyRenamedName()` como obsoletos, sem encerrar `EKM-GAP-0011` nem promover
  a revisão da versão 0.6.
