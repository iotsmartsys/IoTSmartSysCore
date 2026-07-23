# Knowledge Map — IoTSmartSysCore

**Status:** Active

**Última atualização:** 22/07/2026

## 1. Fontes normativas

| Domínio | Fonte | Estado normativo | Implementação |
|---|---|---|---|
| Governança EKM | `docs/rfc/EKM-GUIDELINES.md` | Active | Validated |
| API pública e compatibilidade | `docs/specs/PUBLIC-API-COMPATIBILITY.md` | Active | Implemented |
| Ciclo de vida do runtime | `docs/specs/CORE-RUNTIME-LIFECYCLE.md` | Active | Implemented |
| Release e distribuição | `docs/specs/RELEASE-AND-DISTRIBUTION.md` | Active | In Progress |
| Exemplos executáveis e hardware | `docs/specs/EXECUTABLE-HARDWARE-EXAMPLES.md` | Active | Not Started |

`docs/REPO_DOSSIER.md` é material informativo legado e não prevalece sobre as fontes acima.

## 2. Cobertura de adoção

| Domínio | Cobertura | Entradas principais | Observação |
|---|---|---|---|
| API pública | Specified | `src/SmartSysApp.*`, builders, interfaces, configs | Compatibilidade exige validação dedicada |
| Runtime principal | Specified | `src/main.cpp`, `src/SmartSysApp.cpp` | Arduino sobre ESP32 |
| Capabilities | Mapped | builders, adapters e contracts | Limite intencional de 8 |
| Settings e API HTTP/HTTPS | Mapped | settings, API e storage | Histórico de regressões; falta especificação profunda |
| Wi-Fi e MQTT | Mapped | connectivity e transport | MQTT é transporte principal |
| UART | Inventoried | serial transport | Transporte auxiliar |
| Provisioning e factory reset | Mapped | bootstrap e platform services | Requer especificação própria quando tocado |
| OTA | Inventoried | serviços OTA | Sem especificação própria |
| Plataformas | Mapped | `src/Platform/Arduino`, `src/Platform/Espressif`, legado ESP8266 | ESP-IDF é preparação futura; ESP8266 não é suportado |
| Build e release | Specified | `platformio.ini`, `Makefile`, `.github/workflows/` | Existem desvios abertos |
| Testes | Inventoried | `test/` | Cobertura concentrada em builders/settings |
| Exemplos executáveis | Specified | `src/main.cpp`, `examples/`, `platformio.ini` | Infraestrutura especificada; implementação ainda não iniciada |

## 3. Lacunas

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

## 4. Baseline inicial

- Branch: `main`.
- Commit: `0c6d5e63eb09d826beba2e16a3085c1a8f814668`.
- Worktree inicial da adoção: limpo.
- Runtime suportado: Arduino sobre ESP32.
- ESP8266: não suportado.
- Release: tags na branch `main`, com publicação pelo GitHub Actions no PlatformIO.
