# EKM Changelog — IoTSmartSysCore

## EKM-CHG-0001 — Fundação EKM para o legado

**Estado:** Closed

**Data:** 22/07/2026

### Objetivo

Instituir a governança mínima EKM, registrar o baseline do projeto e criar fontes normativas iniciais para API pública, runtime e release.

### Baseline

- Branch `main`.
- Commit `0c6d5e63eb09d826beba2e16a3085c1a8f814668`.
- Worktree inicial limpo.

### Fontes criadas

- `AGENTS.md`;
- `docs/rfc/EKM-GUIDELINES.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`;
- `docs/specs/PUBLIC-API-COMPATIBILITY.md`;
- `docs/specs/CORE-RUNTIME-LIFECYCLE.md`;
- `docs/specs/RELEASE-AND-DISTRIBUTION.md`.

### Restrições

Nenhum código, build, workflow, teste ou processo de release deve ser alterado nesta transação.

### Validações requeridas

- referências internas e metadados conferidos;
- estados reconciliados com o mapa de conhecimento;
- contratos de API e limite de oito capabilities confrontados com `SmartSysApp.h`;
- divergência do header de versão confirmada no `Makefile` e no GitHub Actions;
- `git diff --check`: aprovado;
- nenhuma alteração fora dos sete ativos EKM aprovados.

### Resultado

A fundação EKM foi instituída sem modificar código, build, workflow, testes ou comportamento. As lacunas descobertas permanecem `Open` no mapa e não impedem o encerramento desta transação documental.

## EKM-CHG-0002 — Especificação dos exemplos executáveis

**Estado:** Open

**Data:** 22/07/2026

### Objetivo

Especificar um catálogo de exemplos reais das capabilities e funcionalidades públicas, selecionáveis por environment PlatformIO e utilizáveis em validação de hardware.

### Baseline

- Branch `main`.
- Worktree inicial limpo.
- `src/main.cpp` é a aplicação padrão e executável atual de hardware.
- Existem exemplos em `examples/`, mas sem seleção uniforme por environment.

### Escopo da fase documental

- criar `EXECUTABLE-HARDWARE-EXAMPLES.md`, submetê-la à decisão humana e promovê-la para `Active` após aprovação;
- registrar o domínio e a lacuna de decisões no mapa;
- não alterar código, PlatformIO, exemplos existentes, testes ou automações.

### Decisões aprovadas

- placa canônica: `iotsmartsys_mcb_r1`;
- exemplos iniciais: `basic_light` e `environment_dht`;
- serviços externos: infraestrutura real somente por configuração privada existente;
- CI: build sem upload dos environments `example_basic_light_mcb_r1` e `example_environment_dht_mcb_r1`.

### Estado documental

- `EXECUTABLE-HARDWARE-EXAMPLES.md`: `Active` / `Implemented`;
- `EKM-GAP-0006`: `Closed`;
- primeiro recorte implementado com runner único, perfil MCB R1, exemplos `basic_light` e `environment_dht`, environments estáveis e matriz de CI;
- o build padrão foi preservado e sua flag de LED foi reconciliada com o `src/main.cpp` legado.

### Requisitos implementados

- `HWEX-001` a `HWEX-017`: atendidos no recorte aplicável por aplicações Arduino completas, seleção em build time, configuração explícita, documentação de hardware, CI e preservação da API pública;
- `HWEX-DEC-001` a `HWEX-DEC-004`: materializados sem incorporar credenciais ou portas locais.

### Evidências da implementação

- resolução dos environments por `pio project config --json-output`: aprovada;
- `pio run -e esp32_dev`: aprovado;
- `pio run -e example_basic_light_mcb_r1`: aprovado;
- `pio run -e example_environment_dht_mcb_r1`: aprovado;
- inspeção de símbolos dos dois firmwares: exatamente um `setup()` e um `loop()` em cada um;
- busca por indicadores de segredos no novo catálogo e configuração: nenhum resultado;
- `git diff --check`: aprovado.

### Validações pendentes

- upload e validação manual em MCB R1 de pelo menos um exemplo;
- confirmação em hardware do identificador/configuração no boot e dos estímulos documentados;
- execução da matriz no GitHub Actions após integração.

### Critério de encerramento

Implementação e validação dos requisitos da especificação, reconciliação das evidências e atualização do estado da implementação. A implementação automatizável foi concluída, mas a transação permanece `Open` até existir a evidência física exigida para promoção a `Validated`.

### Reabertura do escopo normativo — 23/07/2026

A especificação foi promovida para a versão 1.1 após confirmação de que `iotsmartsys_mcb_r1` possui pinout oficial importado automaticamente pela board. O estado da implementação retornou de `Implemented` para `In Progress` e a Technical Readiness Review passou a `Pending Review`.

Novos requisitos:

- `HWEX-018` a `HWEX-023`;
- `HWEX-DEC-005`;
- uso obrigatório de `ITS_MCB01_RELAY_PIN` em `basic_light`;
- uso obrigatório de `ITS_MCB01_TEMPERATURE_SENSOR_PIN` em `environment_dht`;
- proibição de literais e redefinições dos símbolos oficiais de pinout.

Nenhuma conclusão foi antecipada sobre a conformidade da implementação existente. `EKM-GAP-0007` permanece `Open` até a análise técnica, eventual correção e validação.

### Technical Readiness Review de HWEX-018 a HWEX-023 — 23/07/2026

Análise integral executada sem alterar código, build ou configuração de implementação.

**Resultado:** `Implementable`.

**Evidências:**

- `HWEX-018`: conforme — cadeia `boards/iotsmartsys_mcb_r1.json` (`IOTSMARTSYS_MCB01`, `IOTSMARTSYS_BOARD_REV`) → `src/pins.h` → `src/SmartSysApp.h` → exemplos importa o pinout automaticamente, sem cópia paralela;
- `HWEX-019`, `HWEX-020`: não conforme — `configs/executable_examples.ini` define `-DEXAMPLE_LIGHT_PIN=26` (literal) e `examples/executable/basic_light/example.hpp` consome `EXAMPLE_LIGHT_PIN`, sem referenciar `ITS_MCB01_RELAY_PIN`;
- `HWEX-021`: não conforme — `configs/executable_examples.ini` define `-DEXAMPLE_DHT_PIN=23` (literal) e `examples/executable/environment_dht/example.hpp` consome `EXAMPLE_DHT_PIN`, sem referenciar `ITS_MCB01_TEMPERATURE_SENSOR_PIN`;
- `HWEX-022`: conforme — nenhum environment ou `build_flags` da MCB R1 redefine símbolos oficiais do pinout; `-DLED_BUILTIN=23` permanece isolado no environment genérico `esp32_dev`, exceção prevista pelo próprio requisito;
- `HWEX-023`: não acionado — a board importa o pinout esperado e existe símbolo inequívoco para cada função demonstrada; não há lacuna decisória;
- `HWEX-DEC-005`: não conforme na implementação atual pelo mesmo motivo de `HWEX-019` a `HWEX-021`, mas sem redefinição do pinout oficial.

**Conclusão:** os desvios encontrados não exigem decisão ausente — o símbolo oficial já existe e seu valor coincide com o literal hoje usado (26 e 23). A correção é mecânica (substituir literais/macros próprias dos exemplos pelos símbolos oficiais), não altera comportamento observável, API pública ou critério de aceite, e está autorizada pelos requisitos já aprovados. Nenhum requisito resultou em `Needs Clarification`.

**Estado após a revisão:**

- `docs/specs/EXECUTABLE-HARDWARE-EXAMPLES.md`: seção 18 atualizada para `Technical readiness: Implementable`;
- estado da implementação da especificação permanece `In Progress` e o estado da entrega permanece `Not Ready` até a correção mecânica e a validação serem executadas;
- `EKM-GAP-0007` permanece `Open`: a análise técnica foi concluída com resultado `Implementable`, mas a correção de `HWEX-019`, `HWEX-020`, `HWEX-021` e `HWEX-DEC-005` ainda não foi aplicada nem validada;
- nenhum código, build, teste ou configuração de implementação foi alterado nesta etapa, conforme escopo solicitado.

### Invalidação da Technical Readiness anterior — 23/07/2026

A auditoria comparativa com diferentes agentes revelou que a especificação permitia ao próximo executor implementar “sem nova Technical Readiness Review”, em contradição com `AGENTS.md` e `EKM-GUIDELINES.md`.

Decisão:

- remover a dispensa de nova revisão;
- retornar `Technical readiness` para `Pending Review`;
- preservar a análise anterior apenas como evidência histórica, sem autoridade para iniciar implementação;
- exigir nova revisão integral contra o baseline vigente antes de qualquer correção;
- manter `EKM-CHG-0002` e `EKM-GAP-0007` abertos.

Nenhum artefato de implementação foi alterado nesta correção normativa.

### Nova Technical Readiness Review integral — 23/07/2026

**Baseline:** branch `implement_ekm`, commit `b90fe872ed70a6769bd278d3fa76b18f9d9b968a`, worktree inicial limpo.

**Resultado:** `Implementable`.

A especificação completa, suas relações normativas e a implementação vigente foram novamente analisadas antes de qualquer alteração de implementação. A cadeia de importação do pinout oficial foi confirmada e os símbolos `ITS_MCB01_RELAY_PIN` e `ITS_MCB01_TEMPERATURE_SENSOR_PIN` existem de forma inequívoca para as duas funções demonstradas.

Os desvios de `HWEX-019`, `HWEX-020`, `HWEX-021` e `HWEX-DEC-005` admitem somente a correção já determinada pela versão 1.1: remover as macros locais com GPIOs literais e consumir diretamente os símbolos oficiais. Nenhuma inferência relevante, alteração de API pública, mudança de comportamento ou decisão adicional é necessária.

**Escopo autorizado:** correção dos dois exemplos e de seus environments, reconciliação da documentação e execução das validações automatizáveis previstas. `EKM-CHG-0002` e `EKM-GAP-0007` permanecem `Open` durante a implementação e validação.

### Resultado da correção de pinout — 23/07/2026

- `basic_light` usa `ITS_MCB01_RELAY_PIN`;
- `environment_dht` usa `ITS_MCB01_TEMPERATURE_SENSOR_PIN`;
- `EXAMPLE_LIGHT_PIN`, `EXAMPLE_DHT_PIN` e seus literais foram removidos dos environments;
- documentação dos exemplos reconciliada com a autoridade do pinout oficial;
- nenhuma API pública, comportamento, pinout, credencial ou configuração privada foi alterada.

**Validações aprovadas:** resolução do PlatformIO; builds `esp32_dev`, `example_basic_light_mcb_r1` e `example_environment_dht_mcb_r1`; um único `setup()`/`loop()` em cada ELF de exemplo; ausência de macros locais/redefinições de pinout; busca por indicadores de segredos; `git diff --check`.

Os builds mantêm warnings preexistentes do framework Arduino e de flags C/C++, sem warning novo atribuído ao recorte. `EKM-GAP-0007` foi encerrada porque a não conformidade de pinout foi corrigida e validada estaticamente.

`EXECUTABLE-HARDWARE-EXAMPLES.md` está `Implemented` / `Not Ready`. Upload e validação física em MCB R1 permanecem obrigatórios antes de `Validated`; por isso `EKM-CHG-0002` continua `Open`.

## EKM-CHG-0003 — Technical Readiness e atomicidade

**Estado:** Closed

**Data:** 22/07/2026

### Problema observado

Na implementação de `EXECUTABLE-HARDWARE-EXAMPLES.md`, o executor definiu `LED_BUILTIN=23` para preservar o build padrão. A decisão foi tecnicamente coerente, mas a especificação não autorizava explicitamente escolher entre essa solução e outras alternativas possíveis. A descoberta ocorreu durante a implementação, quando o fluxo já havia começado.

### Decisão

- toda especificação deve passar por Technical Readiness Review integral antes de qualquer alteração de implementação;
- o resultado é `Implementable` ou `Needs Clarification`;
- uma lacuna relevante bloqueia atomicamente todos os itens do recorte;
- a decisão ausente deve ser resolvida na especificação e a análise integral repetida;
- o executor não pode converter inferência relevante em implementação.

### Ativos alterados

- `AGENTS.md`;
- `docs/rfc/EKM-GUIDELINES.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

### Validação

- consistência entre instrução de entrada, diretriz, mapa e histórico;
- nenhuma especificação funcional, código, build, teste ou automação alterados;
- `git diff --check` aprovado.

### Encerramento

A governança local foi promovida para o modelo EKM 1.4. Especificações existentes permanecem válidas, mas qualquer nova implementação ou retomada deve cumprir a análise de implementabilidade antes de alterar o repositório.

## EKM-CHG-0004 — Produção imutável, Done e garantias futuras

**Estado:** Open

**Data:** 23/07/2026

### Problema e decisões

- especificações ainda não integradas podem precisar retornar a revisão ou progresso;
- uma versão já integrada à `main` deve preservar a intenção histórica e não pode ser reescrita;
- `Implemented`, `Validated` e entrega em produção representam fatos diferentes;
- garantias EKM ainda dependem de disciplina e futuramente devem receber apoio automatizado.

Foi decidido que:

- `main` é a referência inicial de produção deste projeto;
- versões em produção são imutáveis e mudanças posteriores usam novas especificações relacionadas por `Amends`, `Supersedes`, `Corrects` ou `Retires`;
- o estado de entrega usa `Not Ready`, `Ready for Integration` e `Done`;
- o `EKM Gate` e Automação e Garantias são previstos, porém permanecem `Planned / Not Defined`;
- `EKM-GAP-0008` preserva o trabalho futuro sem afirmar que uma solução já existe.

### Ativos alterados

- `AGENTS.md`;
- `docs/rfc/EKM-GUIDELINES.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

### Estado da entrega

`Ready for Integration` após revisão documental e `git diff --check`. A transação permanece `Open` até integração à `main`, exercitando a Definition of Done do modelo 1.5.
