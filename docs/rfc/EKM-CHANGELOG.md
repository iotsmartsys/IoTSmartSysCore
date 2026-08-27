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

**Estado:** Closed

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

**Estado:** Superseded

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

Esta transação foi substituída por `EKM-CHG-0006` na adoção do modelo 1.9. A
referência de produção, a preservação de versões concluídas e os estados de
entrega permanecem vigentes; a previsão de um `EKM Gate` deixou de integrar o
fluxo atual.

## EKM-CHG-0005 — Revisão cumulativa e autorização humana para implementação

**Estado:** Superseded

**Data:** 23/07/2026

### Problema observado

A Technical Readiness Review de `AIR-CONDITIONER-PHILCO-EXAMPLE` encontrou corretamente a ausência da API pública presumida e bloqueou a implementação, mas encerrou a investigação após o primeiro impedimento. Permaneceram sem registro outras dimensões relevantes do mesmo recorte, como o estado normativo `Draft` e a implementação do ciclo de vida da `AirConditionerCapability`.

O experimento também demonstrou que `Implementable` não deve funcionar como autorização produzida e consumida pelo próprio executor.

### Decisão humana

Foi aprovado um controle manual adequado ao estágio atual do projeto:

- a revisão deve continuar após o primeiro bloqueio e classificar todos os requisitos e dimensões obrigatórias;
- toda revisão deve possuir matriz de evidências com resultado individual;
- Technical Readiness Review e implementação devem ocorrer em execuções separadas;
- a execução da revisão deve encerrar sem alterar implementação, mesmo com resultado `Implementable`;
- `Implementable` significa apto para aprovação humana;
- somente aprovação explícita do arquiteto para a revisão e seu baseline autoriza a execução de implementação;
- o executor deve reconfirmar especificação e baseline antes da primeira alteração;
- mudança material invalida a autorização e exige nova revisão integral;
- `Needs Clarification` deve ser reportado como bloqueio, nunca como implementação concluída.

### Limites desta evolução

- não foram introduzidos múltiplos agentes obrigatórios;
- não foi criado pipeline CI/CD;
- não foi definido nem implantado `EKM Gate`;
- aprovação, reconfirmação e conferência das evidências permanecem manuais;
- automação futura continua preservada em `EKM-GAP-0008`.

### Ativos alterados

- `AGENTS.md`;
- `docs/rfc/EKM-GUIDELINES.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

### Resultado

A governança local foi promovida para o modelo EKM 1.6. A transação permanece `Open` até integração à `main`, conforme as regras de produção e entrega vigentes.

Esta transação foi substituída por `EKM-CHG-0006`. O modelo 1.9 preserva a
autoridade humana, a análise de implementabilidade e a separação lógica das
etapas, mas remove matriz universal, baseline documental e registro duplicado
de aprovação.

## EKM-CHG-0006 — Adoção do modelo EKM 1.9

**Estado:** Closed

**Especificação relacionada:** Não aplicável

**Objetivo:** Atualizar a governança local para o modelo EKM 1.9 publicado no
repositório de referência `EKM-guidelines`.

### Decisões

- a ordem do Arquiteto por prompt ou pipeline define etapa e recorte;
- implementação continua exigindo especificação `Implementable`;
- análise de implementabilidade deixa de exigir matriz universal e baseline
  documental;
- Git volta a ser a fonte de commits, branches, diferenças e linhagem;
- toda tarefa passa a exigir árvore limpa, resultado material, commit, push e
  árvore limpa;
- revisões adicionais deixam de ser etapa universal e ocorrem quando
  solicitadas;
- regras específicas confirmadas do IoTSmartSysCore permanecem nas diretrizes
  locais;
- `EKM Gate`, orquestração, locks e filas não integram o modelo vigente.

### Lacunas

- `EKM-GAP-0008` foi substituída: eventual automação futura requer nova decisão
  baseada em evidência;
- a adoção começou sobre alterações locais preexistentes da governança 1.6; o
  contrato de árvore limpa do modelo 1.9 só poderá ser satisfeito após entrega
  ou resolução explícita desse worktree.

### Evidências materiais

- `README.md`, `docs/EKM-METHOD.md`, `docs/GOVERNANCE.md`,
  `docs/LEGACY-ADOPTION.md` e templates do repositório de referência foram
  confrontados com `AGENTS.md` e os ativos EKM locais;
- a governança local foi reduzida ao template 1.9 com regras específicas do
  projeto preservadas;
- decisões históricas das versões anteriores foram preservadas como
  substituídas, sem reescrita silenciosa.

### Resultado

A estrutura documental local adota o modelo EKM 1.9 e foi integrada à referência
de produção. O worktree está limpo e o contrato Git passou a reger as tarefas
seguintes.

## EKM-CHG-0007 — Correção do estado do controle de garagem

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-GARAGE-CONTROL@0.1`

**Objetivo:** Impedir que a `GarageControlCapability` permaneça em `opening` ou
`closing` contra um fim de curso físico estável e eliminar transições causadas
por bounce.

### Decisões

- estado terminal estável representa a evidência física e prevalece sobre a
  intenção anterior do comando;
- o movimento somente começa quando o sensor do extremo de origem é liberado de
  forma estável;
- sensores usam debounce separado, configurável e com default de 50 ms;
- nomes públicos de comandos e estados são preservados;
- autoria, análise de implementabilidade e implementação permanecem etapas
  distintas.
- a implementação será testada com uma instrução autocontida que reúne as
  regras EKM e técnicas pertinentes ao Implementador; o `AGENTS.md` permanece
  restrito às invariantes permanentes do repositório.
- leituras brutas e estados estáveis permanecem separados inclusive durante a
  inicialização; nenhuma combinação inicial confirma extremo antes do debounce;
- o estado inicial `unknown` não é republicado sem mudança lógica.

### Lacunas

- `EKM-GAP-0009` foi encerrada após o Arquiteto declarar a implementação
  testada e validada em ambiente de hardware.

### Evidências materiais

- relato operacional e histórico de estados demonstram `closed → opening`
  seguido de permanência incorreta em `opening`;
- a implementação atual transforma o comando em estado de movimento e impede
  `closed` enquanto `currentState` é `opening`;
- `IOTSSC-GARAGE-CONTROL@0.1` registra requisitos, condições de borda e
  critérios de aceite;
- a análise confirmou suporte de config, builder, adapters, provider de tempo,
  event sink e Unity para implementar e testar o recorte sem quebra pública;
- `docs/experiments/GARAGE-CONTROL-STATE-IMPLEMENTER-PROMPT.md` contém a ordem,
  o contrato funcional, as restrições e as validações da próxima etapa sem
  exigir releitura da metodologia EKM completa.
- a capability implementa debounce independente por sensor, prioridade da
  evidência terminal estável, direção comandada ou inferida e suporte a sensores
  ausentes ou parciais;
- o builder propaga `sensorDebounceTimeMs`, cujo default público é 50 ms, sem
  alterar o pulso configurado por `debounceTimeMs`;
- o build `esp32_dev` foi aprovado;
- o binário da suíte `test_garage_control_state` foi compilado para ESP32-S3
  com configuração temporária equivalente;
- `pio test -e esp32s3_test` não iniciou a compilação porque o environment
  preexistente estende `env:base32` sem plataforma; a configuração não foi
  alterada por estar fora do recorte autorizado;
- os testes automatizados e a validação física não foram declarados aprovados.
- em etapa posterior, o Arquiteto declarou a implementação testada e validada
  em ambiente de hardware, fornecendo a decisão humana necessária para a
  promoção a `Validated`.

### Resultado

A especificação passa a `Active`, a implementação a `Validated` e a entrega a
`Ready for Integration`, com revisão `Implementable`. A limitação anteriormente
observada no environment automatizado permanece registrada como evidência
histórica, sem invalidar a posterior validação em hardware declarada pelo
Arquiteto. `EKM-CHG-0007` e `EKM-GAP-0009` são encerradas. A integração em
`main` permanece uma etapa separada.

## EKM-CHG-0008 — Adoção do modelo EKM 1.17 e roteamento por atores

**Estado:** Closed

**Especificação relacionada:** Não aplicável

### Objetivo

Adequar a governança local ao modelo EKM 1.17 e garantir que agentes gerais e
Claude Code carreguem as mesmas regras e exatamente o perfil recebido na ordem
do Arquiteto.

### Decisões confirmadas

- a atuação ocorre como Consultor de Arquitetura, subordinado ao Arquiteto;
- o recorte é exclusivamente documental e não altera código funcional,
  especificações funcionais nem seus estados;
- `AGENTS.md` passa a rotear regras comuns e exatamente um perfil oficial da
  EKM;
- `CLAUDE.md` funciona somente como adaptador para o roteamento obrigatório de
  `AGENTS.md`;
- a governança local é reconciliada com o modelo EKM 1.17 na mesma mudança;
- as invariantes técnicas e validações canônicas do IoTSmartSysCore são
  preservadas.

### Evidências materiais

- método, governança, decisões de desenho, regras comuns, perfil do Consultor e
  templates vigentes do repositório `EKM-guidelines` foram confrontados com as
  fontes locais;
- o roteamento inclui os quatro atores do fluxo e o Consultor de Arquitetura;
- a diretriz local incorpora branch derivada da `main`, preservação
  arquitetural, encerramento terminal das execuções e confirmação final do
  Consultor;
- o mapa localiza `AGENTS.md`, `CLAUDE.md`, diretrizes, histórico e a versão
  vigente do modelo.

### Resultado

O roteamento EKM 1.17, o adaptador Claude Code e as fontes locais de governança
foram reconciliados. Nenhuma lacuna funcional foi criada ou encerrada e nenhuma
alegação de validação técnica independente, integração à `main`, release ou
deploy foi produzida.

### Registro da atuação do Consultor

**Estado da confirmação final:** Confirmada pelo Arquiteto.

- **Papel exercido:** Consultor de Arquitetura.
- **Ordem autorizada:** adequar o repositório à EKM 1.17 e adicionar
  `CLAUDE.md`.
- **Repositório e recorte:** IoTSmartSysCore; `AGENTS.md`, `CLAUDE.md` e fontes
  locais de governança afetadas, sem código ou estado funcional.
- **Operações autorizadas:** investigar as fontes vigentes, editar a
  documentação, validar consistência e, após confirmação final, criar commit e
  realizar push.
- **Decisões confirmadas:** adotar o roteamento por atores da EKM 1.17, usar
  `CLAUDE.md` somente como adaptador e reconciliar diretriz, mapa e changelog na
  mesma mudança.
- **Resultado material:** cinco fontes de governança coerentes com a EKM 1.17,
  preservando invariantes e comandos canônicos do projeto.
- **Validações e limitações:** integridade textual, referências e whitespace
  verificados; builds não executados por não serem aplicáveis ao recorte
  documental. O Consultor participou da autoria e não constitui revisão
  independente desta mudança.
- **Significado da confirmação:** o Arquiteto confirmou que este registro
  representa a autorização e as decisões recebidas e autorizou commit e push.
  A confirmação não declara validação técnica independente, integração à
  `main`, release ou deploy.

## EKM-CHG-0009 — Persistência de estados de comandos binários

**Estado:** Open

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.1`

### Objetivo

Restaurar no boot o último estado registrado de cada capability derivada de
`BinaryCommandCapability` e registrar em NVS toda mudança lógica confirmada.

### Intenção confirmada

- todas as capabilities abrangidas devem aplicar no boot o último estado
  registrado;
- o registro deve usar NVS;
- a leitura deve ser leve;
- cada mudança de estado deve ser registrada.

### Solução proposta

- contrato de storage no Core e provedor NVS em `Platform/Espressif`;
- snapshot compacto, versionado e isolado do blob de settings;
- uma leitura de dados por boot e consultas posteriores em cache;
- identidade por `capability_name` definitivo e `type`;
- persistência do estado semântico binário, convertido para o vocabulário de
  cada capability;
- commit a cada transição confirmada, sem gravação para repetição do mesmo
  valor;
- restauração somente após aplicação aceita e leitura de confirmação do
  adapter;
- falhas de storage não bloqueiam nem revertem o runtime.

### Decisão pendente

`BCS-DEC-001` registra que a ordem não definiu se factory reset deve apagar o
snapshot. A recomendação, ainda não confirmada, é apagar o namespace da
funcionalidade junto com os demais dados persistentes do dispositivo.

### Resultado da autoria

`IOTSSC-BINARY-COMMAND-STATE@0.1` foi criada como `Proposed` / `Not Started` /
`Not Ready` / `Pending Review`. Nenhum código de implementação, teste funcional,
build, upload, release ou deploy integra esta etapa.

### Resultado da análise de implementabilidade

A revisão foi promovida para `Implementable`, preservando a especificação como
`Proposed`, a implementação como `Not Started` e a entrega como `Not Ready`.

As fontes técnicas confirmam ponto comum em `BinaryCommandCapability`, retorno
de aceitação e read-back no contrato do adapter, identidade definitiva antes de
`CapabilityManager::setup()`, composição por `ServiceProvider` e registrar de
plataforma, precedente NVS versionado e testes Unity com NVS real. O recorte
pode ser implementado sem alterar APIs públicas ou criar estrutura fora da
fronteira arquitetural já autorizada.

`BCS-DEC-001` permanece pendente, mas foi classificada como não bloqueante:
factory reset está fora do escopo desta especificação e não deve ser alterado
pela implementação. Nenhum código, teste, build funcional, upload, release ou
deploy foi executado nesta etapa. Uma ordem posterior do Arquiteto continua
obrigatória para implementar.

## EKM-CHG-0010 — Implementação da persistência de estados de comandos binários

**Estado:** Open

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.1`

### Objetivo

Implementar o recorte completo autorizado por `EKM-CHG-0009`: contrato de
storage no Core, provedor Espressif em NVS, integração em
`BinaryCommandCapability` e testes PlatformIO/Unity.

### Ordem recebida

O Arquiteto confirmou explicitamente o início da implementação, com recorte
completo e resultado-alvo `Implemented` sustentado por build e testes
automatizáveis.

### Resultado

`IOTSSC-BINARY-COMMAND-STATE@0.1` foi promovida para `Implemented` (estado
normativo `Proposed` preservado; fora da responsabilidade deste papel). Ver a
seção 14 da especificação para o detalhamento completo do código alterado,
testes criados e evidência material.

### Impedimentos pré-existentes resolvidos mediante autorização do Arquiteto

- `configs/esp32s3-test.ini` referenciava um ambiente `env:base32` inexistente,
  bloqueando `pio test -e esp32s3_test` para todo o repositório, não apenas
  para este recorte. Corrigido para `env:base_esp`.
- `src/main.cpp` (arquivo local, listado em `*main.cpp` no `.gitignore` e não
  versionado) não guardava `setup()`/`loop()` contra `UNIT_TEST_MAIN`,
  colidindo com qualquer teste Unity. Guarda adicionada localmente; não gera
  diff versionado.

### Validações

- `pio run -e esp32_dev`: aprovado.
- `pio test -e esp32s3_test --without-uploading --without-testing`: testes
  desta especificação e os testes pré-existentes compatíveis compilaram e
  passaram na etapa de build; testes pré-existentes já desalinhados de outras
  APIs do projeto (anteriores a esta transação) continuam falhando por
  motivos não relacionados.
- `pio test -e esp32s3_test` com upload real: não observável nesta sessão por
  ausência de hardware ESP32-S3 conectado — limitação registrada, não
  alegada como evidência.
- `git diff --check`: aprovado.

### Limitações preservadas

- `BCS-DEC-001` (factory reset) permanece pendente e não bloqueante.
- Validação em hardware permanece pendente e é responsabilidade do Engenheiro
  Revisor para a promoção a `Validated`.
- Este registro não promove `Validated` nem `Done`, e não autoriza upload,
  release ou deploy.

### Resultado da revisão do Tech Lead

**Resultado:** alterações necessárias. A implementação foi reconciliada para
`In Progress`; a especificação permanece `Proposed`, a entrega permanece
`Not Ready` e esta transação continua `Open`.

Foram identificados achados materiais na restauração de válvula sem o
interpreter, no protocolo ausente de sincronização/persistência do LED, na
aceitação de snapshot sem validação de integridade, no tratamento potencialmente
fatal ou indistinto de falhas NVS e no truncamento silencioso da identidade.
A cobertura também não atende todos os tipos, blink, instrumentação de leituras
ou injeção separada de falhas de open, write e commit.

O build `esp32_dev` e a compilação direcionada dos dois novos testes foram
aprovados. A compilação reportou zero casos executados. A tentativa direcionada
de executar os testes terminou em erro de upload, também com zero casos
executados, por ausência de ESP32-S3 conectado. Não houve validação física.

A seção 15 da especificação registra classificação, requisitos afetados,
evidências e recomendação. Nenhum código foi corrigido nesta revisão e nenhuma
aprovação, integração, release ou deploy foi declarada.

## EKM-CHG-0011 — Adoção de critérios de aceite assertáveis

**Estado:** Closed

**Especificação relacionada:** Não aplicável

### Objetivo

Adotar localmente a EKM 1.18, tornando explícito que critérios obrigatórios
precisam permitir asserção objetiva de cenário, resultado observável e
evidência terminal.

### Evidência e decisão

A implementação experimental de persistência binária compilou e criou testes,
mas a revisão encontrou mocks semanticamente incompatíveis, classes concretas
fora do caminho supostamente comum, corrupção sem oráculo suficiente e zero
casos executados usados para sustentar `Implemented`.

O Arquiteto decidiu restaurar posteriormente o recorte funcional ao estado
anterior à implementação, tornar seus critérios simples e assertáveis e repetir
o experimento. A EKM oficial foi preparada como 1.18 para generalizar somente a
regra demonstrada: critérios distinguem aprovação, reprovação e ausência de
execução; doubles preservam semântica material; compilação não comprova
comportamento executado.

### Resultado preparado

`AGENTS.md`, diretrizes locais e mapa foram reconciliados com a EKM 1.18.
Nenhum código funcional, teste ou estado da especificação foi alterado nesta
atuação de governança. A reversão e a nova autoria pertencem à atuação
sequencial posterior.

### Registro da atuação do Consultor

**Estado da confirmação final:** Confirmada pelo Arquiteto.

- **Papel exercido:** Consultor de Arquitetura e par do Arquiteto.
- **Ordem e operações:** evoluir a EKM oficial, reconciliar a adoção local,
  validar consistência e, após confirmação final, criar commits e push.
- **Resultado:** regra assertável preparada na EKM 1.18 e adoção local
  reconciliada.
- **Limitações:** eficácia ainda não demonstrada; o Consultor participou da
  solução e não constitui revisão independente.
- **Significado solicitado:** confirmar este registro e autorizar commit e push
  nos dois repositórios, sem declarar eficácia, validação funcional, integração,
  release ou deploy.

## EKM-CHG-0012 — Adoção local do procedimento de autoria assertável

**Estado:** Closed

**Especificação relacionada:** Não aplicável

### Objetivo

Adotar localmente a EKM 1.19 e tornar operacional a responsabilidade do Autor
de elaborar critérios de aceite rastreáveis, falsificáveis e independentes de
novas decisões durante a implementação.

### Resultado

`AGENTS.md`, as diretrizes locais e o mapa de conhecimento foram reconciliados
com a EKM 1.19. A diretriz local determina que cada requisito obrigatório seja
relacionado a condição inicial, ação, resultado observável e evidência
terminal; um executor independente deve conseguir converter o resultado em
asserção, e a evidência deve conseguir reprovar uma implementação incompatível
plausível.

O perfil oficial continua sendo a fonte integral do procedimento. A diretriz
local preserva apenas a regra necessária para roteamento e aplicação no
projeto, sem duplicar o perfil.

Nenhuma especificação, implementação funcional, teste, configuração de runtime,
release ou deploy foi alterado nesta transação.

### Registro da atuação do Consultor

**Estado da confirmação final:** Confirmada pelo Arquiteto.

- **Papel exercido:** Consultor de Arquitetura.
- **Ordem e operações:** atualizar o IoTSmartSysCore para a última versão da
  EKM, reconciliar somente a governança local, validar consistência e, após
  confirmação final, criar commit e realizar push.
- **Resultado:** roteamento, diretrizes e mapa reconciliados com a EKM 1.19.
- **Limitações:** a eficácia da diretriz será avaliada na repetição do
  experimento. O Consultor participou da formulação e da adoção e não constitui
  revisão independente desse mecanismo.
- **Significado da confirmação:** autorizar este registro, commit e push, sem
  aprovar a especificação funcional, implementação, integração, release ou
  deploy.

## EKM-CHG-0013 — Correção assertável da persistência de comandos binários

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.2`

### Objetivo

Corrigir a especificação após o experimento da versão 0.1, tornando cada
requisito obrigatório verificável por cenário, ação, resultado observável e
evidência terminal conforme a EKM 1.19.

### Intenção e decisões confirmadas

A intenção funcional permanece inalterada: restaurar no boot o último estado
binário validamente registrado para cada capability abrangida e persistir toda
transição confirmada. O Arquiteto determinou que os critérios sejam claros,
simples e suficientes para que o agente executor consiga afirmar ou reprovar a
própria implementação.

`BCS-DEC-001`, sobre factory reset, permanece pendente, fora do escopo e não
bloqueante.

### Resultado da autoria

A versão 0.2:

- relaciona BCS-001 a BCS-023 a critérios BCS-AC-001 a BCS-AC-022;
- exige evidência terminal capaz de distinguir aprovação, reprovação e ausência
  de execução;
- torna reprováveis valve sem interpreter, LED fora do protocolo comum,
  corrupção validada apenas por tamanho/versão, falhas NVS confundidas com
  ausência, identidade truncada e testes compilados com zero casos executados;
- define fidelidade material mínima para doubles de adapter, interpreter, NVS,
  storage e relógio;
- separa o gate automatizável de `Implemented` da validação física posterior.

A especificação foi deixada como `Proposed` / `Not Started` / `Not Ready` /
`Pending Review`. O Autor não executou análise de implementabilidade, alteração
de código, testes funcionais ou build.

### Limitação material

Os artefatos da tentativa experimental 0.1 ainda existem nesta branch e não
constituem implementação da versão 0.2. Sua restauração pertence a uma operação
separada, fora do papel do Autor, e deve ocorrer antes de uma nova implementação
controlada para não contaminar o experimento.

## EKM-CHG-0014 — Revisão de implementabilidade da persistência binária 0.2

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.2`

### Objetivo

Determinar, como Engenheiro Analista, se `IOTSSC-BINARY-COMMAND-STATE@0.2`
pode ser implementada sem decisão normativa, de produto ou arquitetura
ausente, sem reutilizar a conclusão de implementabilidade da versão 0.1.

### Resultado da análise

A revisão foi promovida para `Implementable`, preservando a especificação como
`Proposed`, a implementação como `Not Started` e a entrega como `Not Ready`.

Confrontação com o estado atual do repositório confirmou que os pontos de
composição já exigidos existem (`BinaryCommandCapability` como ponto comum,
`ICommandCapability`/adapter com aceitação e leitura, identidade definitiva
antes de `CapabilityManager::setup()`, `ServiceProvider` +
`EspressifPlatformServiceRegistrar` como precedente de composição,
`common::StateResult` com granularidade suficiente). Os artefatos
experimentais da versão 0.1 ainda presentes na branch (`BinaryCommandCapability::restoreFromStorage()`,
`LEDCapability::handle()`, `EspNvsBinaryCapabilityStateProvider`) reproduzem,
ponto a ponto, os desvios que os fatos observados da versão 0.2 registraram
(valve sem interpreter no restore, LED fora do protocolo comum, integridade
validada apenas por tamanho/versão, truncamento silencioso de identidade).
Cada desvio tem correção alcançável dentro da arquitetura vigente e já descrita
pelos requisitos BCS-002, BCS-004, BCS-006, BCS-012 e BCS-016; nenhum exige
novo contrato, nova camada ou decisão do Arquiteto.

`pio run -e esp32_dev` foi executado nesta sessão apenas para verificar fato e
terminou `SUCCESS` (Flash 89.8%, RAM 23.8%) com o código experimental 0.1 ainda
presente. `pio test -e esp32s3_test` foi executado e terminou `ERRORED` na
etapa de upload para todas as suítes, por exigir um ESP32-S3 conectado
(`upload_port` fixo), incluindo os testes já existentes da própria
funcionalidade — condição ambiental preexistente do projeto, não uma lacuna
desta especificação.

`BCS-DEC-001` permanece pendente e classificada como não bloqueante, sem
mudança em relação à revisão da versão 0.1.

### Limitação material

Nenhum código, teste, configuração, build funcional, upload, release ou deploy
foi alterado por esta análise. Os artefatos experimentais 0.1 permanecem como
material de partida e não constituem evidência aceita para a versão 0.2; uma
nova implementação controlada continua pendente de ordem do Arquiteto.

## EKM-CHG-0015 — Implementação controlada da persistência binária 0.2

**Estado:** Open

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.2`

### Objetivo

Como Engenheiro Implementador, sob ordem do Arquiteto, corrigir os desvios
ponto a ponto identificados em `EKM-CHG-0014` (revisão de implementabilidade
0.2) nos artefatos experimentais 0.1 ainda presentes na branch, sem criar novo
contrato, nova camada ou abstração transversal.

### Alterações materiais

- `src/Core/Capabilities/CapabilityHelpers.h`: `restoreFromStorage()` passou a
  percorrer `command_interpreter` (quando presente) tanto para o comando de
  restauração quanto para a leitura de confirmação, em vez de chamar
  `command_hardware_adapter.applyCommand()`/`getStateValue()` diretamente.
  Corrige BCS-004/BCS-009/BCS-010 para `ValveCapability` (5.3).
- `src/Core/Capabilities/LEDCapability.cpp`: `handle()` passou a chamar
  `syncFromHardware()` incondicionalmente (além do avanço de blink quando
  ativo), preservando publicação/persistência também fora do blink. Corrige o
  desvio "LED fora do protocolo comum" (BCS-016).
- `src/Platform/Espressif/Capabilities/Providers/EspNvsBinaryCapabilityStateProvider.*`:
  adicionado campo `checksum` (FNV-1a sobre versão + todos os registros,
  ativos ou não) validado em `loadSnapshot()`; `copyField()` passou a rejeitar
  (não truncar) identidade que não cabe no buffer interno, e `save()` valida o
  comprimento antes de gravar. Corrige BCS-002/BCS-006/BCS-012.
- Testes e doubles ajustados para a fidelidade exigida em 8.2: `MockBinaryHardwareAdapter`
  passou a rejeitar vocabulário fora de `on`/`off`/`toggle` (como
  `OutputHardwareAdapter`) e a implementar `getState()` de fato;
  `test_binary_command_capability_state.cpp` passou a configurar
  `ValveHardwareCommandInterpreter` real nos casos de valve e ganhou casos
  para o protocolo de `LEDCapability::handle()` fora e dentro de blink (com
  `MockTimeProvider` controlável); `test_binary_capability_state_storage.cpp`
  ganhou casos de identidade sobredimensionada rejeitada e de corrupção de
  byte único no cabeçalho e em registro ativo.

### Validações executadas

- `pio run -e esp32_dev`: `SUCCESS` (Flash 89.8%, RAM 23.8%).
- `git diff --check`: aprovado, sem erros.
- `pio test -e esp32s3_test --filter test_binary_command_capability_state --filter test_binary_capability_state_storage`:
  compilação e upload tentados; ambas as suítes terminaram `ERRORED` na etapa
  de upload por exigir um ESP32-S3 físico conectado (`upload_port` fixo em
  `configs/esp32s3-test.ini`), indisponível nesta sessão — mesma pré-condição
  ambiental já registrada em `EKM-CHG-0014`. A compilação dos testes (etapa
  anterior ao upload) não reportou erro.

### Limitação material

O gate de `Implemented` (spec 8.4) exige `pio test -e esp32s3_test` com estado
terminal aprovado e casos desta especificação efetivamente executados; essa
evidência depende de hardware ESP32-S3 físico não disponível nesta sessão. A
implementação desta transação permanece `In Progress` até essa evidência
existir; nenhum critério da matriz BCS-AC foi promovido a aprovado sem
execução real. `BCS-DEC-001` permanece pendente e não bloqueante, sem
alteração do fluxo de factory reset.

## EKM-CHG-0016 — Validação consultiva da implementação binária 0.2

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.2`

### Objetivo e limite

Confrontar, como Consultor de Arquitetura, a implementação produzida em
`EKM-CHG-0015` com BCS-AC-001 a BCS-AC-022, sem corrigir código e sem promover
estado pertencente ao Engenheiro Revisor.

O Consultor participou da formulação dos critérios e da autoria da versão 0.2;
esta validação é tecnicamente confrontativa, mas não constitui revisão
independente.

### Resultado

O gate de `Implemented` não foi atendido. A implementação permanece corretamente
`In Progress`.

| Classificação | Critérios |
|---|---|
| Aprovado | BCS-AC-022 |
| Reprovado | BCS-AC-002, BCS-AC-006, BCS-AC-007, BCS-AC-011, BCS-AC-012, BCS-AC-016, BCS-AC-020 |
| Não verificado | BCS-AC-001, BCS-AC-003, BCS-AC-004, BCS-AC-005, BCS-AC-008, BCS-AC-009, BCS-AC-010, BCS-AC-013, BCS-AC-014, BCS-AC-015, BCS-AC-017, BCS-AC-018, BCS-AC-019, BCS-AC-021 |

### Achados materiais

1. **Alto — identidade longa continua incompatível com o contrato.**
   A especificação exige preservar integralmente todo nome e tipo aceito pela
   configuração pública e BCS-AC-002 reprova rejeição por limite interno menor
   que o público. O provedor rejeita nomes a partir de 48 bytes e o teste novo
   afirma que essa rejeição é o resultado esperado. A API pública usa
   `std::string`/`const char *` e não declara esse limite. O agente inverteu o
   oráculo explícito em vez de implementar o resultado exigido.
2. **Alto — recuperação NVS pode apagar settings e abortar o runtime.**
   `ensureNvsInit()` ainda executa `ESP_ERROR_CHECK(nvs_flash_erase())`.
   A operação apaga a partição NVS inteira, não apenas o namespace da
   funcionalidade, e a macro pode abortar. Isso viola isolamento de settings,
   continuidade do runtime e tratamento não fatal.
3. **Alto — falha de storage ainda é confundida com ausência.**
   `loadSnapshot()` converte qualquer erro de `nvs_open()` e da consulta inicial
   de `nvs_get_blob()` em `Ok`/ausência. Somente `ESP_ERR_NVS_NOT_FOUND` poderia
   representar ausência; os demais erros precisam permanecer distinguíveis.
   Falhas de write e commit também caem no fallback `StorageReadFail`, embora os
   logs citem operações diferentes.
4. **Alto — snapshot estruturalmente inválido ainda pode ser aceito.**
   O checksum detecta mutação não acompanhada de recomputação, mas após validá-lo
   o provedor não verifica `used`, `isOn` nem terminação das identidades.
   Snapshot com checksum coerente e campos semanticamente inválidos é aceito;
   `strcmp()` pode alcançar campos sem terminador. BCS-AC-007 permanece
   funcionalmente reprovado.
5. **Alto — ausência ou falha de restore da valve não preserva seu vocabulário.**
   O caminho de sucesso passou a usar o interpreter, mas o fallback final ainda
   chama `getStateValue()` diretamente. Para valve, o adapter devolve `off`/`on`
   e o estado lógico esperado é `closed`/`open`. No primeiro `handle()`, a
   conversão posterior pode ainda criar uma transição e persistência que
   BCS-AC-011 proíbe no primeiro boot.
6. **Alto — cobertura obrigatória permanece incompleta.**
   Não há casos dedicados para Switch Plug e Light, ordem completa de restore,
   namespace sentinela, validade estrutural, contadores NVS, todas as origens de
   comando, injeção por operação NVS, reboot após write/commit falho, mudança de
   identidade, logs e preservação completa da API/limite.

As correções de valve no caminho de sucesso e de LED dentro/fora de blink são
coerentes com os respectivos oráculos por inspeção. Sem execução dos testes,
BCS-AC-004 e BCS-AC-015 permanecem não verificados, não aprovados.

### Evidências terminais

- `pio run -e esp32_dev`: `SUCCESS`; RAM 23,8%, Flash 89,8%.
- `pio test -e esp32s3_test`: estado terminal de erro; 15 suítes coletadas,
  zero aprovadas. As duas suítes desta funcionalidade compilaram e falharam no
  upload por ausência de hardware; outras suítes também possuem erros de
  compilação preexistentes. Zero casos comportamentais desta especificação
  foram executados.
- `git diff --check`: aprovado.
- inspeção estática confrontou o delta da implementação, os doubles, os testes,
  o adapter Arduino real, o interpreter da valve, o contrato de identidade e
  as operações NVS.

### Interpretação do experimento

A EKM 1.19 melhorou o resultado de governança: o Implementador registrou
explicitamente critérios parciais ou não verificados e não promoveu falso
`Implemented`.

Ela não garantiu completude da implementação. O agente concentrou-se nos
achados conhecidos da versão 0.1, deixou critérios obrigatórios para “próximos
passos”, associou BCS-AC-007 ao contador de leituras que pertence a BCS-AC-009
e contrariou diretamente BCS-AC-002 ao testar rejeição de identidade longa como
sucesso.

Além disso, os artefatos 0.1 não foram restaurados antes da análise e da
implementação 0.2. O agente trabalhou sobre a solução anterior e seus achados;
portanto, esta execução não isola o efeito da nova especificação sobre uma
implementação iniciada do zero.

### Registro da atuação do Consultor

**Estado da confirmação final:** Confirmada pelo Arquiteto.

- **Papel exercido:** Consultor de Arquitetura.
- **Ordem e operações:** validar a implementação produzida pelo Claude Sonnet 5
  contra a especificação 0.2, executar validações pertinentes e registrar o
  resultado, sem corrigir código nem promover estados do Revisor.
- **Resultado:** gate não atendido; 1 critério aprovado, 7 reprovados e 14 não
  verificados; implementação corretamente preservada como `In Progress`.
- **Validações:** build canônico aprovado, suíte canônica terminal com zero
  suítes aprovadas e integridade textual aprovada.
- **Limitações e independência:** ausência de hardware impediu execução das
  suítes da funcionalidade; outras suítes possuem erros preexistentes. O
  Consultor participou da formulação e autoria da versão 0.2 e não constitui
  revisão independente.
- **Significado da confirmação:** autorizar o fechamento deste registro, commit
  e push somente da documentação, sem aprovar implementação, promover estado,
  aceitar risco ou autorizar correção funcional, integração, release ou deploy.

## EKM-CHG-0017 — Revisão de implementabilidade da persistência binária 0.3

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.3`

### Objetivo

Determinar, como Engenheiro Analista, se `IOTSSC-BINARY-COMMAND-STATE@0.3`
pode ser implementada sem decisão normativa, de produto ou arquitetura
ausente, sem reutilizar a conclusão de implementabilidade da versão 0.2
(`EKM-CHG-0014`).

### Resultado da análise

A revisão foi promovida para `Implementable`, preservando a especificação como
`Proposed`, a implementação como `Not Started` e a entrega como `Not Ready`.

A versão 0.3 corrige a 0.2 para tornar obrigatórias BCS-024 (identidade única
do grafo de `ServiceManager`/`ServiceProvider`) e BCS-025 (preservação do
provisionamento BLE). BCS-001 a BCS-023 mantêm a implementabilidade já
sustentada pela análise da seção 13 do documento e pelas correções já aplicadas
em `EKM-CHG-0015` (commit `5ac3921`); esta transação concentrou a verificação
independente no acréscimo BCS-024/BCS-025.

`src/Core/Providers/ServiceManager.cpp` confirma, no código atual, a causa raiz
exatamente como descrita na seção 2.1 da especificação: `init()` e
`instance()` declaram cada um sua própria variável `static ServiceManager`
local, formando duas instâncias. `ProvisioningController.cpp` (linha 93) chama
`ServiceManager::instance()` de forma síncrona a partir do callback de
conclusão do provisionamento BLE, reproduzindo o segundo trecho da cadeia
causal do abort em `BTC_TASK`. O componente irmão
`src/Core/Providers/ServiceProvider.cpp`, no mesmo diretório, já implementa o
padrão exigido (um único `static` em `instance()`, com `init()` delegando para
ele) — precedente equivalente mais próximo que demonstra a correção como
alteração local e mecânica, sem novo contrato, camada ou decisão do Arquiteto.
`SettingsManager::save()` já é síncrono e já antecede o restart controlado no
fluxo vigente de `ProvisioningController::setupProvisioning()`, confirmando que
a ordem exigida por BCS-025 já existe e que o defeito de 2.1 está na duplicação
do grafo, não na ordem de gravação.

### Achado material não bloqueante

`pio run -e esp32_dev`, executado nesta sessão apenas para verificação de fato,
terminou `FAILED` em `src/main.cpp` (`ESP32_LED_GREEN`/`ESP32_LED_BLUE` não
declarados para `board = esp32dev`; esses símbolos só existem no pinout
ESP32-S3). `git diff main -- src/main.cpp
src/Platform/Espressif/Pinouts/ platformio.ini` não retorna diferença: a falha
já existe em `main`, é anterior e alheia a toda a cadeia `EKM-CHG-0009` a
`EKM-CHG-0016`, e `src/main.cpp`/`src/Platform/Espressif/Pinouts/` não integram
o conhecimento afetado desta especificação. Registrado para o Arquiteto: sem
uma ordem separada autorizando essa correção, o Implementador pode não
conseguir produzir a evidência `pio run -e esp32_dev` `SUCCESS` exigida pelo
gate 8.4, mesmo com BCS-001 a BCS-025 corretamente implementados.

`BCS-DEC-001` permanece pendente e classificada como não bloqueante, sem
mudança em relação às revisões anteriores.

### Limitação material

Nenhum código, teste, configuração, build funcional, upload, release ou deploy
foi alterado por esta análise. Uma nova ordem do Arquiteto é necessária para
iniciar a implementação do acréscimo BCS-024/BCS-025.

## EKM-CHG-0018 — Avaliação consultiva da persistência binária 0.3

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.3`

### Objetivo e limite

Registrar, como Consultor de Arquitetura, os riscos de comportamento em
hardware, as inconsistências materiais da revisão de implementabilidade 0.3 e
as correções recomendadas para uma futura autoria. Esta avaliação não altera
requisitos nem promove ou reverte estados pertencentes ao Autor, Analista,
Implementador ou Revisor.

O estado formal `Implementable` produzido em `EKM-CHG-0017` permanece
registrado, mas sua fundamentação está contestada pelos achados abaixo. Não se
recomenda iniciar nova implementação até o Autor reconciliar o contrato e uma
nova atuação independente do Engenheiro Analista confrontar a versão
resultante.

### Inconsistências da revisão 0.3

1. **Garantia de concorrência inexistente.** A seção 15 afirma que a
   inicialização de estática local é thread-safe por garantia do C++11. O
   build base e o environment `esp32s3_ia` usam explicitamente
   `-fno-threadsafe-statics`. O precedente de `ServiceProvider` continua útil,
   mas a correção precisa ser sustentada pela ordem observável de inicialização
   antes das tasks, não por uma garantia desativada da toolchain.
2. **Reuso contraditório da revisão 0.2.** A revisão declara não reutilizar a
   conclusão anterior e, simultaneamente, preserva BCS-001 a BCS-023 com base na
   seção histórica e na implementação 0.2. Uma revisão incremental pode ser
   proposta, mas não pode ser apresentada como revisão integral independente
   sem confrontar novamente todo o contrato vigente.
3. **Gate obrigatório sem caminho de aprovação.** O gate 8.4 exige
   `pio run -e esp32_dev` com `SUCCESS`; a própria análise obteve `FAILED` por
   causa preexistente e declarou sua correção fora do recorte. Enquanto o
   Arquiteto não autorizar a correção do baseline ou outro oráculo equivalente,
   o Implementador não consegue satisfazer integralmente a especificação.
4. **Conclusão causal excessiva.** A duplicação de `ServiceManager` explica o
   panic observado, mas não comprova que seja a única correção necessária para
   BCS-025. `SettingsManager::save()` retorna falha, porém o callback ignora o
   retorno, agenda restart e registra sucesso incondicionalmente.
5. **Metadado Git copiado sem necessidade material.** A especificação e o
   changelog passaram a citar commit da implementação 0.2, embora o Git já
   preserve essa linhagem e nenhum desvio dependa desse identificador.

### Riscos reais de comportamento em hardware

- **Provisionamento indisponível:** no firmware atual, o caminho BLE reproduzido
  aborta em `BTC_TASK` antes de persistir settings e retorna ao provisioning no
  boot seguinte.
- **Perda global de configuração:** `ensureNvsInit()` usa
  `ESP_ERROR_CHECK(nvs_flash_erase())`; o erase alcança toda a partição NVS, não
  apenas `iotbcs`, podendo remover Wi-Fi, API, MQTT e demais settings. Falha do
  erase pode abortar o runtime.
- **Estado físico divergente após reboot:** identidades acima do limite interno,
  falhas de storage confundidas com ausência e registros rejeitados fazem o
  hardware retornar ao default em vez do último estado confirmado.
- **Snapshot semanticamente inválido:** checksum, tamanho e versão não validam
  `used`, `isOn` nem terminação das identidades. `strcmp()` sobre campo sem
  terminador pode ler além do registro, causar correspondência imprevisível ou
  abort.
- **Valve inconsistente:** o fallback de restore usa `getStateValue()` sem
  interpreter, expondo `off`/`on` onde a capability exige `closed`/`open` e
  podendo gerar publicação e commit artificiais no primeiro ciclo.
- **Desgaste de flash:** persistir e commitar cada alternância de `blink` pode
  produzir dezenas de milhares de commits por dia e acelerar falhas NVS.
- **Latência do ciclo cooperativo:** `nvs_set_blob()` e `nvs_commit()` são
  executados sincronamente em cada transição, podendo introduzir jitter,
  atrasar conectividade e aumentar risco de watchdog sob alta frequência.
- **Ausência de evidência multiplataforma:** o build canônico `esp32_dev`
  permanece falho; portanto não existe evidência atual de preservação do
  runtime ESP32 genérico.

O impacto depende da carga conectada. Em bancada com LED e recuperação local,
os riscos podem ser observados experimentalmente. Para relés, válvulas, bombas,
fechaduras, aquecimento, cargas de potência ou instalação remota, a aceitação
do estado atual não é recomendada.

### Correções solicitadas para futura autoria

O Autor deve produzir uma nova versão relacionada que:

1. torne explícita a ordem de inicialização única de `ServiceManager` antes de
   acessos concorrentes, considerando `-fno-threadsafe-statics`, e defina
   critério que observe identidade, quantidade de construções e ordem;
2. determine que restart e status de sucesso do provisioning somente ocorram
   após `SettingsManager::save()` concluir com sucesso; falha deve permanecer
   observável e não pode descartar a possibilidade de nova tentativa;
3. proíba recuperação do storage binário por erase global da NVS e qualquer
   `ESP_ERROR_CHECK` capaz de abortar o runtime nesse fluxo;
4. exija validação estrutural e semântica de cada registro antes de usar strings
   ou aplicar estado, incluindo domínio de `used`/`isOn` e terminação dos campos;
5. reconcilie a identidade persistente com a API pública sem transformar
   rejeição por limite interno menor em resultado aprovado;
6. obrigue todo fallback da valve a percorrer o interpreter antes de atualizar,
   publicar ou persistir o estado lógico;
7. defina um oráculo de cooperatividade que observe latência e continuidade do
   loop durante write/commit, sem tratar mera compilação como evidência;
8. acrescente testes assertáveis para isolamento do namespace de settings,
   falhas NVS por operação, snapshot semanticamente inválido, limite de
   identidade, fallback da valve, resultado de `SettingsManager::save()` e
   provisioning completo após reboot;
9. remova da fonte normativa metadados Git que não expliquem desvio material e
   preserve as revisões anteriores apenas como evidência histórica contestada.

### Decisões devolvidas ao Arquiteto

- **Política de desgaste por `blink`:** decidir entre persistir cada
  alternância, excluir transições transitórias de blink, consolidar estado ou
  adotar debounce/batching com limite explícito de perda aceitável.
- **Gate de build:** autorizar a correção do baseline `esp32_dev`, substituir o
  environment obrigatório por outro suportado ou definir como a dependência será
  satisfeita sem ampliar silenciosamente o recorte.
- **Contexto da persistência:** confirmar se write/commit NVS pode permanecer
  síncrono no ciclo das capabilities ou se deve ser deslocado para trabalho
  cooperativo/assíncrono com semântica de falha e reboot especificada.

### Ação de segurança externa ao recorte funcional

Uma execução anterior imprimiu o conteúdo de `private.ini` no transcript. Os
valores não são reproduzidos neste registro. Credenciais potencialmente válidas
devem ser rotacionadas e execuções futuras não devem ler nem imprimir arquivos
privados para diagnosticar build ou pinout. Esta ação não deve ser incorporada
como requisito funcional de persistência binária.

### Registro da atuação consultiva

**Estado da confirmação final:** Confirmada pelo Arquiteto.

- **Papel exercido:** Consultor de Arquitetura e par do Arquiteto.
- **Ordem e resultado autorizados:** registrar a avaliação consultiva da versão
  0.3 e indicar correções para futura atuação do Autor.
- **Repositório, recorte e operações:** IoTSmartSysCore; riscos de hardware,
  inconsistências da revisão 0.3 e correções propostas; edição exclusiva de
  `EKM-CHANGELOG.md` e `KNOWLEDGE-MAP.md`, validação textual e, após
  confirmação, commit e push.
- **Decisões confirmadas:** manter a EKOM vigente sem alteração; registrar a
  avaliação sem modificar requisitos, código ou estados formais da
  especificação.
- **Resultado material preparado:** `EKM-CHG-0018` e mapa de conhecimento
  atualizados para localizar a contestação, os riscos, as correções solicitadas
  e as decisões pendentes.
- **Validações, limitações e independência:** confronto estático das fontes e
  integridade textual; nenhum build, teste, upload ou validação funcional foi
  iniciado nesta atuação. O Consultor participou do diagnóstico e da autoria
  da versão 0.3 e não constitui Autor, Analista ou Revisor independente deste
  recorte.
- **Significado da confirmação solicitada:** confirmar que este registro
  representa a avaliação do Arquiteto e autorizar sua marcação como confirmada,
  o fechamento de `EKM-CHG-0018`, commit e push somente da documentação. A
  confirmação não altera a especificação, não invalida formalmente
  `EKM-CHG-0017`, não aprova correção, risco, implementação, integração,
  release ou deploy.

## EKM-CHG-0019 — Autoria da persistência binária 0.4

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.4`

### Objetivo

Produzir a versão 0.4 da especificação de persistência de comandos binários,
relacionada por `Corrects` à versão 0.3, incorporando a avaliação consultiva
`EKM-CHG-0018`.

### Resultado material

- `docs/specs/BINARY-COMMAND-STATE-PERSISTENCE.md` promovida documentalmente
  para a versão 0.4 com estados `Proposed` / `Not Started` / `Not Ready` /
  `Pending Review`;
- fatos, escopo, fora de escopo, solução, requisitos BCS-001 a BCS-029, falhas,
  critérios BCS-AC-001 a BCS-AC-028, fidelidade dos doubles, gates e conhecimento
  afetado reconciliados com `EKM-CHG-0018`;
- decisões pendentes registradas sem decisão do Autor: `BCS-DEC-001` (não
  bloqueante), `BCS-DEC-002` (blink), `BCS-DEC-003` (gate de build) e
  `BCS-DEC-004` (contexto síncrono/assíncrono), com impacto na futura revisão;
- revisões e implementação 0.2/0.3 preservadas apenas como histórico
  contestado; o estado `Implementable` da versão 0.3 não foi reutilizado;
- metadados Git sem necessidade normativa removidos da fonte normativa;
- mapa de conhecimento atualizado para localizar a versão 0.4.

### Correções normativas incorporadas

1. inicialização única de `ServiceManager` antes de acessos concorrentes, sob
   `-fno-threadsafe-statics`;
2. sucesso e restart do provisioning condicionados a
   `SettingsManager::save()`;
3. proibição de erase global da NVS e de abort por `ESP_ERROR_CHECK` no storage
   binário;
4. validação estrutural e semântica completa do snapshot;
5. reconciliação do limite de identidade com a API pública;
6. interpreter obrigatório em todos os fallbacks da valve;
7. oráculo de cooperatividade para write/commit;
8. critérios completados para falhas NVS, isolamento de settings, identidade,
   valve e provisioning após reboot.

### Restrições observadas

Nenhum código, teste, build ou configuração foi alterado. Nenhuma revisão de
implementabilidade foi executada nesta atuação.

### Validações

- `git diff --check`: aprovado na entrega;
- conferência dos estados de saída e da relação `Corrects` 0.3 → 0.4.

### Próximo passo

Nova atuação independente do Engenheiro Analista sobre
`IOTSSC-BINARY-COMMAND-STATE@0.4`, sem reutilizar `EKM-CHG-0017`.

## EKM-CHG-0020 — Decisões arquiteturais da persistência binária 0.5

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.5`

### Objetivo e limite

Incorporar à especificação as decisões do Arquiteto que encerram os bloqueios
`BCS-DEC-002`, `BCS-DEC-003` e `BCS-DEC-004`, preservando os estados formais
dos atores e sem alterar código, testes ou configuração.

### Decisões confirmadas

1. alternâncias produzidas exclusivamente pelo temporizador de `blink` são
   transitórias e não são persistidas; o último estado estável permanece
   válido durante o modo, e o estado estável confirmado ao encerrá-lo é
   solicitado uma vez quando tiver mudado;
2. `pio run -e esp32_dev` permanece o gate canônico obrigatório; eventual
   correção preexistente do baseline exige autorização e entrega separadas e
   não pode ser substituída silenciosamente por outro environment;
3. write e commit do snapshot binário são executados por um único escritor
   assíncrono Espressif, fora de callbacks BLE, caminhos síncronos de comando e
   `handle()` das capabilities;
4. o trabalho pendente é limitado a uma entrada consolidada por identidade,
   até oito, sem alocação ou crescimento por transição; mudança ocorrida
   durante write/commit não pode ser perdida;
5. o worker é ativado uma única vez após `ServiceManager::init()` retornar com
   o grafo completo e antes da primeira solicitação; falha de criação é
   observável e não autoriza fallback síncrono;
6. aceitação da solicitação não significa commit concluído; trabalho pendente,
   operação em curso, sucesso e falha permanecem observáveis, e somente commit
   bem-sucedido altera o snapshot restaurável;
7. a execução instrumentada no target deve preservar margem mínima de 25% da
   pilha configurada para o worker sob a carga definida pelo critério;
8. `BCS-DEC-001` permanece pendente, fora do recorte e não bloqueante.

### Resultado material

- especificação promovida documentalmente para a versão 0.5, relacionada por
  `Corrects` à versão 0.4;
- requisitos BCS-013, BCS-016 a BCS-018 e BCS-029 reconciliados com a política
  estável de `blink` e o escritor assíncrono;
- BCS-AC-001, BCS-AC-010 e BCS-AC-013 a BCS-AC-015 atualizados para distinguir
  solicitação, consolidação, quiescência e commit;
- BCS-AC-022 fixado no gate `esp32_dev`;
- BCS-AC-023 e BCS-AC-028 ampliados para observar ordem de ativação, identidade
  única, contexto executor, limite, concorrência, falha de criação e pilha;
- estados preservados como `Proposed`, `Not Started`, `Not Ready` e `Pending
  Review`.

### Registro da atuação consultiva

- **Papel exercido:** Consultor de Arquitetura e par do Arquiteto.
- **Ordem e resultado autorizados:** corrigir a especificação de persistência
  binária com as decisões arquiteturais necessárias e entregar a documentação
  confirmada.
- **Repositório, recorte e operações:** IoTSmartSysCore; persistência binária,
  `blink`, gate de build, writer NVS, ciclo de serviços, critérios e registros
  EKM; edição documental, validação textual, commit e push.
- **Decisões explicitamente confirmadas:** as oito decisões relacionadas nesta
  transação, sem decisão sobre factory reset.
- **Resultado material produzido:** versão 0.5 da especificação, esta transação
  e mapa de conhecimento correspondente.
- **Validações, limitações e independência:** integridade textual e unicidade
  dos 29 requisitos e 28 critérios verificadas; nenhum build, teste funcional,
  upload ou validação em hardware foi iniciado. O Consultor participou desta
  correção e não constitui Analista ou Revisor independente do mesmo recorte.
- **Significado da confirmação:** o Arquiteto confirmou a entrega documental e
  autorizou seu registro, commit e push. A confirmação não promove estados,
  não aprova implementação, não valida hardware e não autoriza integração,
  release ou deploy.

### Próximo passo

Nova análise independente de implementabilidade sobre
`IOTSSC-BINARY-COMMAND-STATE@0.5`. A correção do baseline `esp32_dev`, se ainda
necessária, depende de ordem separada e deve anteceder a aprovação de
BCS-AC-022.

## EKM-CHG-0021 — Revisão de implementabilidade da persistência binária 0.5

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.5`

### Objetivo

Determinar, como Engenheiro Analista, se a versão integral 0.5 pode ser
implementada sem decisão normativa, de produto ou arquitetura ausente, sem
reutilizar as conclusões históricas contestadas das versões 0.2 e 0.3.

### Resultado da análise

A revisão foi promovida para `Needs Clarification`, preservando a especificação
como `Proposed`, a implementação como `Not Started` e a entrega como `Not
Ready`.

O contrato público aceita `capability_name` por `const char *`, armazena-o em
`std::string` e permite renomeação sem teto documentado. O buffer local de 32
bytes limita apenas nomes automáticos; não limita nomes externos. BCS-002
proíbe que o storage tenha limite interno menor, enquanto BCS-AC-002 e
BCS-AC-021 exigem o maior comprimento público aceito. Portanto um implementador
não possui oráculo finito sem inventar limite, compatibilidade ou representação.
`BCS-DEC-005` e `EKM-GAP-0010` devolvem essa decisão ao Arquiteto.

A análise integral confirmou que `BCS-DEC-001` continua fora do escopo e não
bloqueante e que os demais requisitos possuem fronteiras, precedentes e
critérios suficientes: contrato no Core, provedor Espressif, composição por
serviços, inicialização única antes de concorrência, worker assíncrono limitado,
interpreter da valve, provisioning condicionado a `save()` e seams observáveis.

### Evidência e dependências

`pio run -e esp32_dev` terminou `FAILED` na compilação de `src/main.cpp` porque
`ESP32_LED_GREEN` e `ESP32_LED_BLUE` não estão definidos. A falha confirma a
dependência externa prevista por `BCS-DEC-003`: correção mínima autorizada e
entregue separadamente deve anteceder BCS-AC-022. Como a política e o caminho
responsável já estão definidos, esse baseline falho não acrescenta uma decisão
normativa ausente à versão 0.5.

Nenhum teste funcional, upload ou validação física foi executado. Nenhum código,
teste ou arquivo de configuração de implementação foi alterado.

### Próximo passo

O Arquiteto deve decidir o contrato público de comprimento da identidade e
ordenar nova autoria da especificação. A versão reconciliada deve retornar a
uma revisão independente; implementação permanece não autorizada. A correção do
baseline `esp32_dev` continua sendo uma entrega separada.

## EKM-CHG-0022 — Autoria da persistência binária 0.6

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Objetivo

Corrigir integralmente a versão 0.5 após `EKM-CHG-0021`, incorporando a decisão
do Arquiteto sobre o contrato público finito da identidade persistente sem
alterar código, testes ou configuração de implementação.

### Decisão confirmada

`BCS-DEC-005` limita o `capability_name` definitivo a 63 bytes e `type` a 31
bytes de sua representação UTF-8, excluídos os terminadores nulos. Valor acima
do limite deve falhar observavelmente antes do registro, sem truncamento,
consumo de slot, capability/adapter parcial, alteração do cache ou solicitação
de persistência. Omissão de nome preserva a geração automática vigente e o
nome resultante passa pela mesma validação.

Configurações existentes dentro dos limites permanecem compatíveis. Consumidor
com identidade excedente deve adequá-la antes de adotar a versão; não existe
truncamento, alias ou migração silenciosa de registro persistido.

### Resultado material

- especificação promovida documentalmente para a versão 0.6, relacionada por
  `Corrects` à versão 0.5;
- BCS-002, BCS-022, BCS-AC-002 e BCS-AC-021 reconciliados com os limites,
  rejeição pré-registro, geração automática e ausência de efeito parcial;
- storage obrigado a preservar integralmente 63/31 bytes e seus terminadores,
  sem limite interno menor;
- checklist integral restaurado e `EKM-GAP-0010` encerrada;
- estados definidos como `Proposed`, `Not Started`, `Not Ready` e `Pending
  Review`, sem reutilizar revisões anteriores.

### Validações e limitações

Somente integridade textual, rastreabilidade normativa e coerência documental
pertencem a esta autoria. Nenhum código, teste funcional, build, upload ou
validação física foi iniciado. A falha conhecida do baseline `esp32_dev`
permanece dependência separada conforme `BCS-DEC-003`.

### Próximo passo

Nova atuação independente do Engenheiro Analista sobre
`IOTSSC-BINARY-COMMAND-STATE@0.6`. Implementação permanece não autorizada.

## EKM-CHG-0023 — Revisão de implementabilidade da persistência binária 0.6

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Objetivo

Determinar, como Engenheiro Analista, se a versão integral 0.6 pode ser
implementada sem decisão normativa, de produto ou arquitetura ausente, sem
reutilizar revisões anteriores.

### Resultado da análise

A revisão foi promovida para `Needs Clarification`, preservando a especificação
como `Proposed`, a implementação como `Not Started` e a entrega como `Not
Ready`.

`BCS-DEC-005` resolveu os máximos de 63/31 bytes, a capacidade do storage e a
rejeição no fluxo inicial do builder. A superfície pública, porém, devolve
ponteiros para capabilities já registradas; `ICapability::capability_name` e
`ICapability::type` são campos públicos mutáveis, e `rename()`/
`applyRenamedName()` alteram identidade com retorno `void`. Assim, um consumidor
pode produzir identidade excedente após slot e objetos já existirem, enquanto
BCS-002 exige rejeição pré-registro sem efeito parcial e a solução proíbe
caminho alternativo de renomeação acima do limite.

`BCS-DEC-006` e `EKM-GAP-0011` devolvem ao Arquiteto a escolha entre identidade
imutável após registro, com a compatibilidade pública correspondente, ou
mutação suportada com validação, identidade prevalente, falha observável e
preservação explícita de slot, adapter, cache e registro. BCS-AC-002 e
BCS-AC-021 também precisam exercer o caminho decidido.

A confrontação restante confirmou contratos e precedentes suficientes para a
fronteira Core/plataforma, serviços únicos, worker assíncrono, valve,
provisioning e doubles. `BCS-DEC-001` continua fora do escopo e não bloqueante.
A falha conhecida de `esp32_dev` permanece dependência separada já governada
por `BCS-DEC-003`, não nova decisão ausente nesta versão.

### Validações e limitações

Foram executadas somente inspeções estáticas e validação textual. Nenhum código,
teste ou configuração de implementação foi alterado; nenhum build, teste
funcional, upload ou validação física foi iniciado.

### Próximo passo

O Arquiteto deve decidir `BCS-DEC-006` e ordenar nova autoria integral. A versão
reconciliada deve retornar a uma análise independente; implementação permanece
não autorizada.

## EKM-CHG-0024 — Obsolescência dos métodos públicos de renomeação

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Decisão confirmada

O Arquiteto determinou que `ICapability::rename()` e
`ICapability::applyRenamedName()` sejam marcados como obsoletos na API pública e
não recebam novos usos.

### Alcance e limitação

A decisão é parcial no contexto de `BCS-DEC-006`: não autoriza remoção dos
métodos, não define se chamadas legadas continuam alterando a identidade e não
decide o tratamento dos campos públicos mutáveis `capability_name` e `type`.
Consequentemente, `EKM-GAP-0011` permanece aberta e a revisão da versão 0.6
continua `Needs Clarification`.

Nenhum código, teste ou configuração de implementação foi alterado. Esta
transação registra somente a decisão humana recebida e sua limitação material;
implementação permanece não autorizada.

### Próximo passo

O Arquiteto deve completar `BCS-DEC-006` quanto ao comportamento legado e à
atribuição direta aos campos públicos antes de nova autoria integral.

## EKM-CHG-0025 — Resolução da mutabilidade da identidade e análise complementar

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Objetivo

Registrar as decisões finais do Arquiteto para `BCS-DEC-006`, reconciliar seus
efeitos normativos e determinar, como Engenheiro Analista, se a versão integral
0.6 pode ser implementada sem outra decisão ausente.

### Decisões confirmadas

- `capability_name` e `type` passam a ser imutáveis: o builder deve resolvê-los,
  validá-los e finalizá-los antes do registro; depois disso, a API preserva
  leitura e remove atribuição pública;
- `rename()` e `applyRenamedName()` permanecem públicos, obsoletos e com retorno
  `void`, mas não alteram a identidade de uma capability registrada;
- os métodos `SmartSysApp::add*Capability()` mantêm os ponteiros atualmente
  devolvidos para as capabilities já registradas.

### Resultado da análise

A revisão da versão 0.6 foi promovida para `Implementable`, preservando a
especificação como `Proposed`, a implementação como `Not Started` e a entrega
como `Not Ready`. BCS-002, BCS-020, BCS-022, BCS-AC-002 e BCS-AC-021 passaram a
definir e comprovar finalização pré-registro, leitura sem atribuição, ausência de
mutação pelos métodos obsoletos, manutenção dos retornos `void` e preservação
dos ponteiros públicos. `EKM-GAP-0011` foi encerrada.

A confrontação integral de BCS-001 a BCS-029, BCS-AC-001 a BCS-AC-028,
decisões, falhas, relações, dependências e gates não encontrou outra decisão
normativa, de produto ou arquitetura ausente. `BCS-DEC-001` permanece fora do
escopo e não bloqueante. A falha conhecida do baseline `esp32_dev` continua uma
dependência externa com contrato responsável em `BCS-DEC-003`; ela não impede
implementabilidade, mas continuará reprovando BCS-AC-022 até ser corrigida e o
build canônico terminar com sucesso.

### Validações e limitações

Foram executadas inspeção estática das superfícies públicas e validação de
integridade textual. Nenhum código, teste ou configuração de implementação foi
alterado; nenhum build, teste funcional, upload ou validação física foi
iniciado. `Implementable` não autoriza implementação.

### Próximo passo

Uma ordem posterior do Arquiteto é necessária para iniciar a implementação
integral de `IOTSSC-BINARY-COMMAND-STATE@0.6`. A correção do baseline
`esp32_dev`, se ainda necessária, permanece entrega separada conforme
`BCS-DEC-003`.

## EKM-CHG-0026 — Implementação da persistência de estados binários 0.6

**Estado:** Closed (fechada por `EKM-CHG-0032`, após validação física e
aprovação do Arquiteto)

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Objetivo

Implementar, como Engenheiro Implementador, o contrato integral da versão 0.6:
identidade pública imutável com limites 63/31, restauração interpretada,
persistência assíncrona por escritor único, validação semântica do snapshot,
identidade única do grafo de serviços e provisionamento condicionado ao sucesso
de `SettingsManager::save()`.

### Baseline

- Branch `spec/binary-command-state-persistence`, derivada da `main`.
- Árvore de trabalho limpa no início da atuação.
- Revisão de implementabilidade `Implementable` registrada em `EKM-CHG-0025`.

### Implementação

- `ICapability` publica os limites de identidade e passa a expor
  `capability_name` e `type` com leitura pública e sem atribuição pública;
  `rename()` e `applyRenamedName()` permanecem públicos, `void`, obsoletos e
  inertes. O `CapabilitiesBuilder` resolve o nome definitivo — fornecido ou
  gerado —, valida nome e tipo e finaliza a identidade antes do registro,
  rejeitando de forma observável antes de criar adapter, capability ou slot.
- `BinaryCommandCapability` concentra restauração, read-back, publicação e
  solicitação de persistência; toda leitura de estado confirmado percorre o
  interpreter quando configurado, inclusive nos fallbacks da valve.
- `LEDCapability` marca as alternâncias do temporizador como transitórias e
  confirma o estado estável uma única vez ao encerrar o modo `blink`.
- O provedor Espressif mantém snapshot desejado e confirmado sob mutex, com um
  único worker FreeRTOS serializando write e commit; o caminho solicitante
  retorna sem tocar a NVS. O formato passou à versão 2, com campos de 64/32
  bytes e validação estrutural, semântica e de integridade antes de qualquer
  `strcmp`. Nenhum caminho executa erase global, `ESP_ERROR_CHECK`, abort ou
  restart.
- `ServiceManager::init()` e `ServiceManager::instance()` convergem para uma
  instância única sem depender de estáticas locais thread-safe sob
  `-fno-threadsafe-statics`; `SmartSysApp::setup()` ativa o escritor uma única
  vez após a conclusão do grafo.
- `ProvisioningController::completeProvisioning()` condiciona restart controlado
  e status/log de sucesso ao sucesso de `SettingsManager::save()`.

Foram adicionados os seams exigidos pela seção 8.2 da especificação e três
suítes de teste novas: `test_capability_identity`, `test_service_graph_identity`
e `test_provisioning_save_gate`.

### Validações executadas

- `pio run -e esp32_dev`: `FAILED`, com os mesmos dois erros preexistentes de
  `src/main.cpp` registrados na seção 12.1 da especificação; nenhum erro novo.
- `pio test -e esp32s3_test --without-uploading --without-testing`: compilação
  aprovada para as suítes desta especificação; `test_builder`, `test_waterflow`,
  `test_humidity` e `test_mqtt_settings` falham na compilação.
- `pio test -e esp32s3_test`: não executado, por ausência de alvo ESP32-S3.
- `git diff --check`: aprovado.

### Limitações e impedimentos

1. O baseline `esp32_dev` continua falho; sua correção exige autorização e
   entrega separadas conforme `BCS-DEC-003`. BCS-AC-022 permanece reprovado.
2. Nenhum alvo ESP32-S3 está conectado, portanto nenhum critério comportamental
   foi executado. BCS-AC-024 e a medição de pilha de BCS-AC-028 não puderam ser
   observadas. Compilação não comprova execução.
3. `test_builder`, `test_waterflow`, `test_humidity` e `test_mqtt_settings` não
   compilam na `main` nem nesta branch, por uso de assinatura antiga do builder,
   membros de config inexistentes, `CapabilityManager::count` privado e caminho
   de header inexistente. A verificação foi confirmada com a árvore revertida ao
   baseline. Enquanto persistirem, `pio test -e esp32s3_test` não alcança estado
   terminal aprovado. A correção é alheia ao domínio binário e exige autorização
   e entrega separadas.

Nenhum upload, release, deploy ou validação física foi realizado. `BCS-DEC-001`
permanece fora do escopo.

### Resultado

A implementação da versão 0.6 permanece Em andamento [`In Progress`]. Todos os
critérios BCS-AC continuam **não verificados**, exceto BCS-AC-022, que está
reprovado pelo baseline. A especificação permanece `Proposed` e a entrega
`Not Ready`.

### Próximo passo

O Arquiteto deve decidir sobre a correção autorizada do baseline `esp32_dev` e
das quatro suítes preexistentes quebradas, e disponibilizar um alvo ESP32-S3
para que `pio test -e esp32s3_test` e BCS-AC-024 possam alcançar estado
terminal.

### Fechamento

Os impedimentos acima foram resolvidos por `EKM-CHG-0030` (BCS-REV-001/002 e
baseline) e confirmados por `EKM-CHG-0031` (gate estático). O Arquiteto validou
a implementação e o firmware em hardware e aprovou a promoção; `EKM-CHG-0032`
registra essa evidência e promove os estados. Esta transação é encerrada com
objetivo cumprido.

## EKM-CHG-0027 — Revisão técnica da implementação da persistência binária 0.6

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Objetivo

Revisar, como Engenheiro Revisor, a implementação entregue em `EKM-CHG-0026`
contra o contrato integral da versão 0.6 e emitir recomendação independente de
promoção.

### Achados materiais

1. `BCS-REV-001` — Alta: o provider devolve `Ok` para qualquer falha de abertura
   do namespace ou consulta de tamanho do blob, inclusive
   `ESP_ERR_NVS_NOT_INITIALIZED`, confundindo falha de storage com ausência. Um
   teste exige explicitamente esse comportamento, em desacordo com BCS-017,
   BCS-021, BCS-AC-016 e BCS-AC-020.
2. `BCS-REV-002` — Alta: comando explícito aplicado durante `blink` não encerra
   o modo nem consolida o primeiro estado estável; as alternâncias continuam e
   o caso de substituição exigido por `BCS-DEC-002`/BCS-AC-015 não é testado.
3. `BCS-REV-003` — Alta: o double do writer nunca expõe operação em curso e
   modela writes por identidade, sem barreira no commit real, atualização
   concorrente durante commit, oito identidades pendentes ou medição da margem
   de pilha. A matriz anterior superestima a implementação de BCS-AC-028.

### Validações e limitações

- `pio run -e esp32_dev`: `FAILED` pelos identificadores preexistentes
  `ESP32_LED_GREEN` e `ESP32_LED_BLUE`; BCS-AC-022 permanece reprovado;
- `pio test -e esp32s3_test`: `FAILED`, com 18 suítes coletadas, 0 aprovadas e
  18 em erro; nenhum caso executado. As suítes novas que compilaram não
  ultrapassaram o upload por ausência de hardware, e foram observadas falhas de
  compilação em onze suítes preexistentes, não apenas nas quatro registradas em
  `EKM-CHG-0026`;
- inspeção estática confirmou os três achados;
- `git diff --check` estava aprovado antes do registro documental.

Nenhum código de produção ou teste foi corrigido nesta revisão. Nenhum upload,
release, deploy ou validação física foi realizado.

### Resultado e recomendação

A revisão não aprova promoção. A especificação permanece `Proposed`, a
implementação `In Progress`, a entrega `Not Ready` e a revisão de
implementabilidade `Implementable`. A implementação deve retornar ao
Engenheiro Implementador para corrigir BCS-REV-001 e BCS-REV-002, completar os
seams e testes de BCS-AC-028 e reconciliar a evidência das suítes. Depois disso,
os gates canônicos devem ser repetidos, com alvo ESP32-S3 para os critérios
dependentes de hardware.

## EKM-CHG-0028 — Quarentena das suítes de teste existentes

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Decisão confirmada

O Arquiteto determinou que todas as suítes existentes no repositório até
01/08/2026, inclusive as adicionadas pela implementação 0.6, não devem ser
executadas. No estágio atual elas são antigas ou insuficientemente confiáveis
para atestar comportamento, e sua recuperação imediata teria custo
desproporcional. Os arquivos são preservados para retomada futura.

### Aplicação operacional

As 18 suítes foram enumeradas nominalmente em `test_ignore` no environment
`esp32s3_test`. A enumeração, em vez de curinga, evita que testes futuros sejam
ignorados sem nova decisão. A listagem do PlatformIO deve apresentá-las como
`SKIPPED`, sem build, upload ou execução.

`pio project config --json-output` confirmou as 18 entradas de `test_ignore`;
`pio test -e esp32s3_test --list-tests` apresentou as 18 suítes como `SKIPPED`;
e `pio test -e esp32s3_test` terminou com zero casos, sem iniciar build, upload
ou execução de teste.

`BCS-DEC-007` reconcilia a especificação: `pio test -e esp32s3_test` deixa de
integrar o gate e seus resultados anteriores deixam de constituir evidência
positiva ou negativa. Critérios BCS-AC dependentes dessas suítes permanecem
`Deferred`, não aprovados. O achado BCS-REV-003 passa a dívida futura para a
reativação da estratégia de testes; BCS-REV-001 e BCS-REV-002 continuam defeitos
funcionais abertos.

### Limites e reativação

A quarentena não aprova critérios de aceite, não substitui revisão estática,
build ou validação física e não autoriza remoção dos arquivos. Reativar qualquer
suíte exige decisão explícita do Arquiteto e uma estratégia capaz de produzir
evidência confiável.

### Estado

A especificação permanece `Proposed`, a implementação `In Progress`, a entrega
`Not Ready` e a revisão de implementabilidade `Implementable`.

## EKM-CHG-0029 — Nova revisão técnica após a quarentena de testes

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Objetivo

Repetir a revisão integral da implementação sob a decisão `BCS-DEC-007`, sem
executar ou usar como evidência as 18 suítes em quarentena.

### Resultado da revisão

A revisão não aprova promoção. Não houve mudança de código de produção após a
entrega `0d7f151`; os commits posteriores alteraram somente documentos EKM e a
configuração de quarentena. A inspeção confirmou:

1. `BCS-REV-001` permanece aberto e de impacto alto: falhas diferentes de
   `ESP_ERR_NVS_NOT_FOUND` durante open ou consulta de tamanho do blob ainda são
   registradas como ausência e retornam `Ok`, em desacordo com BCS-017/021;
2. `BCS-REV-002` permanece aberto e de impacto alto: comando explícito ainda não
   encerra `blink` nem consolida o primeiro estado estável exigido por
   BCS-013/016 e `BCS-DEC-002`;
3. `BCS-REV-003` permanece dívida técnica real, mas está `Deferred` e fora do
   gate atual por decisão explícita em `BCS-DEC-007`; não foi convertido em
   aprovação.

### Validações e limitações

- `pio run -e esp32_dev`: `FAILED` pelos identificadores preexistentes
  `ESP32_LED_GREEN` e `ESP32_LED_BLUE`; BCS-AC-022 permanece reprovado;
- `git diff --check`: aprovado antes do registro documental;
- nenhuma suíte foi compilada ou executada; critérios dependentes permanecem
  `Deferred`;
- nenhum upload, validação física, release ou deploy foi realizado.

### Recomendação

Retornar ao Engenheiro Implementador para corrigir BCS-REV-001 e BCS-REV-002.
O build canônico deve alcançar `SUCCESS` por entrega separada conforme
`BCS-DEC-003`. Depois das correções, nova revisão estática terminal deve ser
ordenada, mantendo as suítes em quarentena.

### Estado

A especificação permanece `Proposed`, a implementação `In Progress`, a entrega
`Not Ready` e a revisão de implementabilidade `Implementable`. Não há aprovação
de integração.

## EKM-CHG-0030 — Correção dos achados da persistência binária 0.6

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Objetivo

Corrigir, como Engenheiro Implementador, BCS-REV-001 e BCS-REV-002, entregar
separadamente a correção mínima do baseline `esp32_dev` autorizada por
`BCS-DEC-003` e devolver o resultado a nova revisão estática.

### Implementação

- o provider NVS aceita como ausência somente `ESP_ERR_NVS_NOT_FOUND` em open e
  na consulta de metadados. Demais erros são registrados e retornados como
  falha de storage;
- `LEDCapability::applyCommand()` encerra `blink` para qualquer comando
  explícito, aplica e confirma o valor pelo protocolo comum e consolida no
  máximo uma solicitação estável. Alternâncias internas do timer usam o caminho
  base qualificado e permanecem transitórias;
- em commit separado, `esp32_dev` passou a mapear os dois identificadores
  lógicos de LED usados pelo entrypoint local para `LED_PIN` e
  `ESP32_LED_BUILTIN`. O arquivo local ignorado `src/main.cpp` não foi
  incorporado nem modificado como fonte versionada.

### Validações e limitações

- `pio run -e esp32_dev`: `SUCCESS`, com firmware compilado e linkado, 24,1% de
  RAM e 90,1% de flash;
- a tentativa anterior bloqueada pela permissão do lock global do PlatformIO
  não iniciou compilação e não é contada como evidência;
- `git diff --check`: aprovado antes do registro documental;
- nenhuma suíte foi compilada ou executada, conforme `BCS-DEC-007`;
- nenhum upload, validação física, release ou deploy foi realizado.

### Resultado e próximo passo

As correções foram implementadas, mas a atuação não promove a própria entrega.
A especificação permanece `Proposed`, a implementação `In Progress`, a entrega
`Not Ready` e a revisão de implementabilidade `Implementable`. Solicita-se nova
revisão estática independente para confirmar o encerramento de BCS-REV-001/002
e emitir recomendação de promoção; BCS-REV-003 permanece `Deferred`.

## EKM-CHG-0031 — Revisão estática independente promove a persistência binária 0.6 a Implemented

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Objetivo

Atuação do Engenheiro Revisor solicitada pela seção 12.9: confirmar
estaticamente o encerramento de `BCS-REV-001` e `BCS-REV-002`, confrontar o
gate atualizado da seção 8.4 e emitir recomendação de promoção.

### Revisão

- `git diff 0d7f151..HEAD -- src/` confirma que, desde a entrega de código
  original, apenas `LEDCapability.h`, `LEDCapability.cpp` e
  `EspNvsBinaryCapabilityStateProvider.cpp` foram alterados em produção, mais
  `platformio.ini` (autorizado separadamente por `BCS-DEC-003`). As áreas já
  confirmadas pelas revisões anteriores (identidade, protocolo comum, escritor
  assíncrono, grafo de serviços, provisioning) não foram tocadas;
- `BCS-REV-001` confirmado corrigido: `loadSnapshot()` trata somente
  `ESP_ERR_NVS_NOT_FOUND` como ausência; qualquer outro erro de abertura ou de
  consulta de metadado é registrado e devolvido como falha de storage;
- `BCS-REV-002` confirmado corrigido: `LEDCapability::applyCommand()` encerra
  `blink` para todo comando explícito — inclusive pelo caminho de comando
  remoto, que despacha por `ICommandCapability::applyCommand` virtual —, aplica
  pelo protocolo comum, faz read-back e consolida o estado estável no máximo
  uma vez, com a deduplicação existente prevenindo dupla solicitação;
  alternâncias do próprio temporizador continuam transitórias;
- `BCS-REV-003` permanece `Deferred` por `BCS-DEC-007`, sem alteração de código
  nesta atuação e sem integrar o gate atual;
- nenhum novo achado funcional ou de segurança foi identificado por inspeção
  estática nos arquivos alterados e nas áreas adjacentes que os consomem.

### Validações e limitações

- `pio run -e esp32_dev` (rebuild limpo, executado nesta atuação): `SUCCESS`
  — RAM 24,1%, Flash 90,1%;
- `git diff --check`: aprovado, sem erros;
- `configs/esp32s3-test.ini`: enumeração nominal das 18 suítes em
  `test_ignore` confirmada, sem curinga, conforme `BCS-DEC-007`; nenhuma suíte
  foi compilada ou executada nesta atuação;
- revisão exclusivamente estática e de build; nenhuma validação em hardware
  foi realizada; `BCS-AC-024` e a margem de pilha de `BCS-AC-028` continuam não
  verificados por ausência de alvo ESP32-S3;
- nenhum upload, release ou deploy foi realizado.

### Resultado

Os quatro critérios do gate da seção 8.4 estão satisfeitos. A implementação da
versão 0.6 é promovida para Implementada [`Implemented`]. A especificação
permanece `Proposed`, a entrega `Not Ready` e a revisão de implementabilidade
`Implementable`: esta atuação não constitui aprovação normativa, validação
física (seção 8.5) nem autorização de integração, que continuam condicionadas
a validação suficiente do Tech Lead e decisão explícita do Arquiteto.
Recomenda-se ao Arquiteto autorizar a etapa de validação física em hardware
como próximo passo.

## EKM-CHG-0032 — Validação física do Arquiteto e promoção final da persistência binária 0.6

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Objetivo

Registrar, como Engenheiro Revisor, a validação física e a aprovação
explícita do Arquiteto recebidas em ordem direta, e promover os estados do
ciclo técnico conforme a saída do perfil de Revisor.

### Evidência humana recebida

O Arquiteto declarou ter validado pessoalmente a implementação e o firmware da
versão 0.6 e aprovou explicitamente a promoção ao próximo ator. Esta atuação
não reexecuta nem substitui essa validação: registra-a como recebida, conforme
a regra de que, com validação do Tech Lead e aprovação do Arquiteto já
fornecidas, o Revisor registra a evidência sem repetir a decisão nem criar
aprovação própria.

### Promoções

- Estado normativo: Proposta → Vigente [`Active`];
- Estado da implementação: Implementada → Validada [`Validated`];
- Estado da entrega: Não pronta → Pronta para integração
  [`Ready for Integration`];
- `EKM-CHG-0026` (implementação integral da versão 0.6) é fechada, por
  objetivo cumprido e validado.

### Lacunas preservadas

- `BCS-DEC-001` (factory reset) continua pendente e fora de escopo;
- `BCS-REV-003` continua `Deferred` por `BCS-DEC-007`;
- as 18 suítes preexistentes continuam `SKIPPED` por quarentena vigente;
- `BCS-AC-024` e a margem de pilha de `BCS-AC-028` continuam sem oráculo
  automatizado próprio.

### Resultado

A especificação está `Active` / `Validated` / `Ready for Integration`. Não se
declara Concluída [`Done`]: nenhuma integração à `main` foi confirmada nesta
atuação. A promoção para `Done` depende de confirmação explícita futura do
Arquiteto de que o resultado foi integrado à referência de produção.

### Reconciliação de consistência

Na conferência final da atuação, o Revisor corrigiu duas referências residuais
do mapa de conhecimento que ainda descreviam a implementação como
`Proposed`/`In Progress`, acrescentou `EKM-CHG-0032` à trilha histórica do mapa
e tornou explicitamente histórica a frase de abertura da seção 12 da
especificação. Não houve mudança de requisito, código ou decisão, nem execução
de build, teste, upload ou validação física adicional.

## EKM-CHG-0033 — Retrospectiva EKOM da persistência de comandos binários

**Estado:** Open — aguardando confirmação final do Arquiteto

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

### Objetivo

Revisar o experimento multiagente que produziu e validou a persistência binária
0.6 e classificar, pela métrica experimental EKOM 2.1, as combinações
observáveis de perfil executor e papel.

### Resultado preparado

O relatório
`docs/rfc/EKOM-EXPERIMENT-BINARY-COMMAND-STATE-PERSISTENCE.md` separa resultado
funcional de conformidade EKOM, pontua as execuções com evidência suficiente,
registra descontos, eliminatórios, conflitos de independência e limitações de
comparabilidade e propõe recomendações para novos experimentos.

O resultado funcional do ciclo está validado, mas nenhum perfil é qualificado
como `Accepted`: a amostra pertence essencialmente a uma especificação e um
contexto, houve variação material entre execuções e uma parte das avaliações é
de pair, não independente.

### Limites

- nenhuma especificação funcional é reaberta ou promovida;
- testes, build, upload e validação física não são reexecutados;
- BCS-REV-003 e a quarentena das suítes permanecem inalterados;
- a atribuição `Claude Fable 5` presente nas promoções finais permanece
  separada da relação inicial Sonnet/Opus;
- a classificação aguarda confirmação explícita do Arquiteto antes de commit e
  push.

## EKM-CHG-0034 — Autoria da especificação de leitura de corrente contínua

**Estado:** Superseded — versão 0.1 substituída pelo Draft 0.2

**Especificação relacionada:** `IOTSSC-CURRENT-SENSOR@0.1`

### Objetivo

Registrar como fonte normativa a funcionalidade de medição de corrente
contínua, separada em Hardware Adapter (`ICurrentSensor`, implementado por
`ACS712C30ACurrentSensor`) e Capability (`CurrentSensorCapability`), seguindo o
precedente `IGlpMeter` / `HX711WeightMeter` / `GlpMeterKgCapability`.

### Baseline

- Branch `spec/current-sensing-capability`, derivada de `main`.
- Alterações preexistentes e não relacionadas na árvore de trabalho preservadas
  fora do delta desta transação.

### Fontes criadas ou alteradas

- `docs/specs/CURRENT-SENSING-CAPABILITY.md` (criada);
- `docs/rfc/KNOWLEDGE-MAP.md` (fonte normativa e cobertura);
- `docs/rfc/EKM-CHANGELOG.md` (esta transação).

### Decisões incorporadas

`CUR-DEC-001` a `CUR-DEC-009`: recorte exclusivo de corrente contínua; sem
persistência da calibração de zero; critério de publicação pelo precedente
`GlpMeterKgCapability`; identidade por `resolveIdentity`; fábrica, builder e
`SmartSysApp` no recorte em forma aditiva; nome `ACS712C30ACurrentSensor`;
nenhum artefato de teste no recorte; nenhuma exigência de canal ou faixa
específica de pino analógico; `CURRENT_SENSOR_TYPE` igual a
`"Current Sensor (A)"`.

### Restrições

Transação exclusivamente documental. Nenhum código, build, workflow, teste,
environment ou comportamento de runtime foi alterado. Os cálculos elétricos
estão descritos de forma autossuficiente na seção 6 da especificação, sem
depender de leitura de código.

### Validações requeridas

- `git diff --check`: aprovado;
- delta restrito à especificação criada, ao mapa de conhecimento e a este
  changelog;
- relações normativas confrontadas com `PUBLIC-API-COMPATIBILITY` e
  `CORE-RUNTIME-LIFECYCLE`, ambas preservadas.

### Resultado

A especificação `IOTSSC-CURRENT-SENSOR` foi registrada na versão 0.1 em
`Draft`, com implementação `Not Started` e revisão de implementabilidade
`Pending Review`. Nenhuma implementação foi autorizada ou iniciada.

A análise formal
`docs/reports/2026-08-26T223239Z-0.1-af120342-implementability-analysis.md`
classificou a versão como `Not Ready — Specification Defect` por ausência de
tolerância objetiva em CUR-AC-004. O Arquiteto respondeu ao bloqueador e
autorizou a versão 0.2 em `EKM-CHG-0043`.

## EKM-CHG-0035 — Autoria da especificação do console de tela

**Estado:** Closed — ciclo da versão 0.3 validado em `EKM-CHG-0042`

**Especificação relacionada:** `IOTSSC-SCREEN-CONSOLE@0.1`

### Objetivo

Registrar como fonte normativa a incorporação de um console de tela ao core,
como ferramenta de diagnóstico construída sobre o mesmo padrão do logging:
contrato em `Contracts`, implementação em `Platform`, fachada estática com
implementação nula por default, ativação opt-in por build e custo nulo quando
desativada.

### Baseline

- Branch `spec/screen-console-tooling`, derivada de
  `spec/current-sensing-capability` para preservar a sequência do changelog e do
  mapa de conhecimento.
- Alterações preexistentes e não relacionadas na árvore de trabalho preservadas
  fora do delta desta transação.

### Fontes criadas ou alteradas

- `docs/specs/SCREEN-CONSOLE-TOOLING.md` (criada);
- `docs/rfc/KNOWLEDGE-MAP.md` (fonte normativa e cobertura);
- `docs/rfc/EKM-CHANGELOG.md` (esta transação).

### Decisões incorporadas

`SCR-DEC-001` a `SCR-DEC-010`: primitiva de escrita por cor abstrata com
atalhos por severidade; registro no grafo de serviços com fachada estática
`Screen`; `ScreenMirrorLogger` no recorte, opcional e não instalado por default;
aposentadoria do componente inerte `Display_ST7789_170_320`; suporte restrito a
ST7789 sobre SPI; quebra de texto longo em linhas consecutivas; histórico de
capacidade fixa de 24 linhas; nomes contratados dos componentes; nenhum
artefato de teste no recorte; flag `IOTSMARTSYS_SCREEN_CONSOLE_ENABLED` com
default `0`.

### Restrições

Transação exclusivamente documental. Nenhum código, build, workflow, teste,
environment ou comportamento de runtime foi alterado. A remoção prevista em
`SCR-038` é contrato para a Implementação, não efeito desta transação.

### Validações requeridas

- `git diff --check`: aprovado;
- delta restrito à especificação criada, ao mapa de conhecimento e a este
  changelog;
- relações normativas confrontadas com `PUBLIC-API-COMPATIBILITY`,
  `CORE-RUNTIME-LIFECYCLE` e `RELEASE-AND-DISTRIBUTION`, todas preservadas;
- dependências de display confirmadas como já declaradas em `library.json`.

### Resultado

A especificação `IOTSSC-SCREEN-CONSOLE` foi registrada na versão 0.1 em
`Draft`, com implementação `Not Started` e revisão de implementabilidade
`Pending Review`. Nenhuma implementação foi autorizada ou iniciada.

## EKM-CHG-0036 — Análise de implementabilidade do console de tela 0.1

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-SCREEN-CONSOLE@0.1`

### Objetivo

Determinar, como Engenheiro Analista, se a versão 0.1 do console de tela pode
ser implementada dentro da baseline e do recorte autorizados, sem decisão
normativa ausente, pré-requisito arquitetural ou evidência prévia
indispensável.

### Resultado da análise

Classificação **Pronta** [`Ready`]. Nenhum bloqueador. A revisão de
implementabilidade da especificação passa a `Implementable`; estado normativo
permanece `Draft`, implementação `Not Started` e entrega `Not Applicable`.

O padrão contratado tem precedente equivalente e vigente no logging
(`ILogger`, `Log`/`DefaultLogger`, `ArduinoSerialLogger` e alimentação da
fachada por `ServiceManager::registerServices`), e a extensão é aditiva, fora
do ciclo cooperativo. `IServiceProvider` tem um único implementador no
repositório, o que admite a adição de SCR-034 preservando SCR-037. O componente
de SCR-038 é inerte de fato: guarda `ST7789_170x320_ENABLED`, não definido por
nenhum environment, e referencia identificadores inexistentes.

Confrontados 40 requisitos, 10 critérios de aceite, 10 decisões e as 5 bordas
da seção 7, além de `EKM-GAP-0001` a `EKM-GAP-0011`, nenhum aplicável. Não
existe relatório anterior nesta linhagem a reconciliar.

### Restrições registradas

Cinco restrições materiais não bloqueantes constam do relatório: ausência de
environment que construa o caminho habilitado e de `lib_deps` gráfico em
`base_esp`; ausência de mapa de link para SCR-AC-002; referências textuais ao
componente removido fora do conhecimento afetado declarado; consumo duplo do
`va_list` em `ScreenMirrorLogger`; e assimetria do meio de SCR-AC-001 entre as
seções 8 e 9.

### Fontes criadas ou alteradas

- `docs/reports/2026-08-26T012514Z-0.1-5cc6e5eb-implementability-analysis.md`
  (criado);
- `docs/specs/SCREEN-CONSOLE-TOOLING.md` (revisão de implementabilidade e
  seção 12);
- `docs/rfc/KNOWLEDGE-MAP.md` (destino de relatórios e estado da fonte);
- `docs/rfc/EKM-CHANGELOG.md` (esta transação).

### Validações e limitações

`git diff --check` aprovado. Somente inspeção estática: nenhum código, teste,
configuração ou environment foi alterado; nenhum build, teste, upload ou
validação física foi iniciado. Destino `docs/reports/` criado por autorização
explícita do Arquiteto nesta atuação. Branch de trabalho derivada de
`spec/current-sensing-capability`, conforme baseline de `EKM-CHG-0035`.

### Próximo passo

A implementação da versão 0.1 depende de ordem explícita do Arquiteto; o
Analista não a autoriza nem a inicia.

## EKM-CHG-0037 — Implementação do console de tela 0.1

**Estado:** Closed — ciclo da versão 0.3 validado em `EKM-CHG-0042`

**Especificação relacionada:** `IOTSSC-SCREEN-CONSOLE@0.1`

### Objetivo

Implementar e validar o console de tela como ferramenta de diagnóstico conforme
a versão 0.1 autorizada, preservando os contratos públicos, o ciclo cooperativo
e o comportamento existente quando a funcionalidade estiver desativada.

### Entrada

- ordem explícita do Arquiteto para implementar a versão 0.1;
- análise de implementabilidade `Ready` registrada em `EKM-CHG-0036`;
- branch `spec/screen-console-tooling`, derivada de `main`, sincronizada com o
  upstream e com árvore limpa no início da atuação.

### Estado inicial

A implementação passa mecanicamente de Não iniciada [`Not Started`] para Em
andamento [`In Progress`]. Nenhum código ou validação havia sido executado no
momento desta transição.

### Implementação e decisões locais

- contrato `IScreenConsole`, paleta `ScreenColor`, fachada `Screen` e
  `NoOpScreenConsole` adicionados em `Contracts`;
- `ST7789ScreenConsole` implementado sob a flag default 0, com configuração
  integral, histórico circular fixo de 24 linhas, quebra de texto e redesenho de
  faixas ocupadas;
- `ScreenMirrorLogger` implementado como decorador opcional, com cópias
  independentes do `va_list` e mapeamento de cores por nível;
- grafo de serviços estendido aditivamente e fachada alimentada pelo registro;
- componente inerte `Display_ST7789_170_320.{h,cpp}` removido.

### Evidências

- `pio run -e esp32_dev`: `SUCCESS`, estado terminal, código 0, Arduino/ESP32,
  target `esp32_dev`;
- inspeção do ELF e objetos: nenhuma implementação ST7789 ou dependência gráfica
  linkada com a flag 0; decorador não instalado eliminado pelo linker;
- busca textual: nenhum consumidor dos símbolos removidos permanece em `src/`;
- `git diff --check`: aprovado.

### Limitações e estado

SCR-AC-003 a SCR-AC-008 e a execução instrumentada de SCR-AC-001 permanecem
`Not Executed`, pois exigem hardware e ordem operacional explícita. O caminho
habilitado não foi construído porque nenhum environment vigente ativa a flag e
as dependências gráficas não constam do `lib_deps` de `base_esp`. Nenhum
teste foi criado, alterado ou executado. A implementação permanece `In Progress`.

### Relatório

`docs/reports/2026-08-26T141607Z-0.1-de05f6a6-implementation-report.md`.

## EKM-CHG-0038 — Autoria do exemplo executável do console de tela 0.2

**Estado:** Closed — ciclo da versão 0.3 validado em `EKM-CHG-0042`

**Especificação relacionada:** `IOTSSC-SCREEN-CONSOLE@0.2`

### Objetivo

Complementar o contrato do console de tela com um exemplo executável de uso,
selecionado pelo runner versionado, construível por environment próprio e capaz
de demonstrar o `ScreenMirrorLogger` na placa Ideaspark ESP32 1.9 inch TFT LCD.

### Decisões incorporadas

- o exemplo integra `examples/executable/screen_console/` e é referenciado por
  `src/ExecutableExampleRunner.cpp`; o `src/main.cpp` permanece a aplicação
  padrão e não é alterado;
- o environment `example_screen_console_esp32_dev` herda `env:esp32_dev`,
  habilita o console e declara as dependências gráficas;
- a configuração do exemplo usa ST7789 170 × 320, `CS=GPIO15`, `DC=GPIO2`,
  `RST=GPIO4`, `SCLK=GPIO18`, `MOSI=GPIO23` e backlight `GPIO32`;
- a demonstração constrói, registra e inicializa o console e instala
  explicitamente `ScreenMirrorLogger` sobre o logger corrente;
- nenhum artefato ou execução de teste passa a integrar o recorte; o exemplo,
  sua documentação, seu environment e seu build são evidências próprias.

### Relações e estado

`IOTSSC-HW-EXAMPLES@1.1` é preservada: o novo exemplo usa o runner e a seleção
por environment vigentes. A versão normativa passa de 0.1 para 0.2 e permanece
`Draft`; a revisão de implementabilidade retorna de `Implementable` para
`Pending Review`, pois o relatório `Ready` existente governa somente 0.1. A
implementação permanece `In Progress` e ainda não representa o contrato 0.2.

### Fontes alteradas

- `docs/specs/SCREEN-CONSOLE-TOOLING.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

### Limites

Atuação exclusivamente normativa e documental. Nenhum código, exemplo,
environment, dependência, teste, build, upload ou validação física foi criado,
alterado ou executado. O pinout do anexo fornecido pelo Arquiteto foi tratado
somente como evidência técnica, sem instrução documental paralela.

## EKM-CHG-0039 — Análise de implementabilidade do console de tela 0.2

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-SCREEN-CONSOLE@0.2`

### Objetivo

Determinar se a versão 0.2 pode acrescentar o exemplo executável Ideaspark ao
runner e ao environment `esp32_dev` dentro da baseline e do recorte vigentes.

### Resultado

Classificação **Pronta** [`Ready`]. Nenhum bloqueador normativo, arquitetural,
de impacto ou de evidência prévia. O runner, o catálogo e o padrão de
environments existentes admitem a extensão aditiva; o pinout explícito é
permitido para a board genérica sem pinout normativo; e a ordem de instalação
do decorador pode preservar o reset de `Log` executado por `SmartSysApp::setup()`.

Foram confrontados 48 requisitos, 14 critérios de aceite, 13 decisões, 6
bordas, `EKM-GAP-0001` a `EKM-GAP-0011` e as autoridades relacionadas. O
challenge limitado não encontrou contradição, critério insatisfazível,
remediação externa ou bloqueador anterior sem disposição.

### Fontes e limitações

- relatório criado:
  `docs/reports/2026-08-26T153635Z-0.2-e827ebd6-implementability-analysis.md`;
- especificação, mapa e esta transação reconciliados para `Implementable`;
- somente inspeção estática: nenhum código, exemplo, environment, dependência,
  build, teste, upload ou hardware foi alterado ou executado;
- build habilitado e validações físicas permanecem evidências posteriores da
  Implementação/Revisão, não condições prévias desta classificação.

## EKM-CHG-0040 — Implementação do exemplo executável do console de tela 0.2

**Estado:** Closed — ciclo da versão 0.3 validado em `EKM-CHG-0042`

**Especificação relacionada:** `IOTSSC-SCREEN-CONSOLE@0.2`

### Objetivo

Implementar o exemplo executável `screen_console`, sua seleção pelo runner
versionado e o environment `example_screen_console_esp32_dev`, demonstrando o
`ScreenMirrorLogger` com a configuração Ideaspark contratada pela versão 0.2.

### Condições de entrada

- ordem explícita do Arquiteto recebida;
- revisão de implementabilidade `Ready` registrada em `EKM-CHG-0039`;
- branch `spec/screen-console-tooling` descendente de `main`, sincronizada com
  seu upstream e com árvore limpa;
- implementação permanece `In Progress`; upload e validação física não estão
  autorizados neste recorte.

### Implementação

- exemplo `examples/executable/screen_console/` criado com configuração
  Ideaspark completa, documentação e `SmartSysApp::handle()` cooperativo;
- runner ampliado com a seleção exclusiva
  `IOTSMARTSYS_EXAMPLE_SCREEN_CONSOLE`;
- environment `example_screen_console_esp32_dev` criado a partir de
  `env:esp32_dev`, sem `src/main.cpp`, com flag habilitada e dependências
  Adafruit GFX/ST7789;
- `ScreenMirrorLogger` instalado depois de `SmartSysApp::setup()` e usado para
  a mensagem diagnóstica de boot.

### Evidências e estado

`pio run -e esp32_dev` e
`pio run -e example_screen_console_esp32_dev` alcançaram `SUCCESS`. O ELF do
exemplo contém um único par `setup()`/`loop()` e os símbolos
`ST7789ScreenConsole`, `ScreenMirrorLogger` e `Adafruit_ST7789`; o ELF canônico
não contém implementação gráfica. `src/main.cpp` permaneceu inalterado e não
foi compilado pelo environment do exemplo. `git diff --check` foi aprovado.

Nenhum teste, upload ou validação física foi executado. SCR-AC-003 a
SCR-AC-008, SCR-AC-013 e a parcela instrumentada de SCR-AC-001 permanecem
`Not Executed`; portanto, o estado normativo continua `In Progress`.

Relatório:
`docs/reports/2026-08-26T154955Z-0.2-d49f8216-implementation-report.md`.

### Correção de orientação do exemplo

Após o Arquiteto relatar que a configuração inicial exibia texto de cabeça
para baixo e solicitar apresentação horizontal, o environment passou a definir
`EXAMPLE_SCREEN_ROTATION=1`. O exemplo consome obrigatoriamente essa
configuração e passa a usar área lógica paisagem de 320 × 170, preservando as
dimensões nativas 170 × 320 e todos os defaults de plataforma.

Os builds `pio run -e example_screen_console_esp32_dev` e
`pio run -e esp32_dev` alcançaram `SUCCESS`, ambos com código 0, e
`git diff --check` foi aprovado. Nenhum teste, upload ou monitor foi executado;
a orientação resultante permanece pendente de confirmação física.

Relatório da correção:
`docs/reports/2026-08-26T162253Z-0.2-c9fad5d6-implementation-correction-report.md`.

## EKM-CHG-0041 — Ancoragem do console de tela no topo da área útil 0.3

**Estado:** Closed — validada pelo Arquiteto em `EKM-CHG-0042`

**Especificação relacionada:** `IOTSSC-SCREEN-CONSOLE@0.3`

### Objetivo

Emendar a disposição das linhas do console de tela, que passa da ancoragem na
base para a ancoragem no topo da área útil, e implementar a mudança na
implementação ST7789.

### Condições de entrada

- ordem explícita do Arquiteto encadeando autoria, análise e implementação;
- análise `Ready` da versão 0.3 registrada nesta mesma transação;
- branch `spec/screen-console-tooling` descendente de `main`, com árvore limpa;
- implementação permanece `In Progress`; upload e validação física não estão
  autorizados neste recorte.

### Autoria

O uso do exemplo `screen_console` na placa Ideaspark demonstrou que o
comportamento contratado até 0.2 estava corretamente implementado, mas não
correspondia à disposição pretendida pelo Arquiteto. A decisão `SCR-DEC-014`
inverte a ancoragem; `SCR-012`, `SCR-013`, `SCR-AC-003` e a visão geral da
seção 1 foram emendados. `SCR-014`, `SCR-AC-004` e `SCR-AC-006` permanecem
inalterados, pois a rolagem e o descarte com a área útil cheia não mudam.

### Análise

Versão 0.3 confrontada integralmente e classificada Pronta [`Ready`], sem
bloqueador normativo, arquitetural, de impacto ou de evidência prévia. A
revisão volta a `Implementable`.

### Implementação

- `ST7789ScreenConsole::render()` passa a ancorar o bloco na primeira linha da
  área útil, com a ordenada de cada banda em `position * lineHeight_`;
- a leitura de altura do painel foi removida da função por ficar sem uso; a
  leitura de `begin()` que dimensiona a capacidade visível foi preservada;
- README do exemplo `screen_console` reconciliado com a nova disposição.

### Evidências e estado

`pio run -e esp32_dev` e `pio run -e example_screen_console_esp32_dev`
alcançaram `SUCCESS` com código de saída 0. `git diff --check` foi aprovado.
Nenhum teste, upload ou validação física foi executado; `SCR-AC-003` a
`SCR-AC-008`, `SCR-AC-013` e a parcela instrumentada de `SCR-AC-001` permanecem
`Not Executed` e o estado normativo continua `In Progress`.

A orientação dos glifos observada na placa permanece assunto independente, não
governado por esta versão e não corrigido nesta transação.

Relatórios:
`docs/reports/2026-08-26T203103Z-0.3-71f40ba6-implementability-analysis.md` e
`docs/reports/2026-08-26T203432Z-0.3-71f40ba6-implementation-report.md`.

## EKM-CHG-0042 — Validação final do console de tela 0.3

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-SCREEN-CONSOLE@0.3`

### Objetivo

Registrar a validação física e a decisão final do Arquiteto, confrontar as
evidências de software e promover a versão 0.3 para integração à referência de
produção.

### Evidência humana recebida

O Arquiteto confirmou em ordem direta ter executado os testes em hardware e
validado a implementação. A confirmação cobre `SCR-AC-003` a `SCR-AC-008`,
`SCR-AC-013` e a parcela instrumentada de `SCR-AC-001`. Esta transação registra
a evidência recebida sem alegar reexecução ou substituir a decisão humana.

### Confrontação consultiva

O Consultor de Arquitetura, atuando como par do Arquiteto e sem alegação de
independência, confrontou implementação, especificação e evidências sem
identificar defeito bloqueante. `pio run -e esp32_dev` e
`pio run -e example_screen_console_esp32_dev` alcançaram `SUCCESS`. A inspeção
dos ELFs confirmou ausência da implementação gráfica na baseline, presença da
implementação ST7789 no exemplo e um único par `setup()`/`loop()`. A busca
textual não encontrou consumidor dos símbolos aposentados no código.

### Promoções

- Estado normativo: Rascunho → Vigente [`Active`];
- Estado da implementação: Em andamento → Validada [`Validated`];
- Estado da entrega: Não aplicável → Pronta para integração
  [`Ready for Integration`].

As transações `EKM-CHG-0035`, `EKM-CHG-0037`, `EKM-CHG-0038`,
`EKM-CHG-0040` e `EKM-CHG-0041` são fechadas pelo resultado validado da versão
0.3. Nenhum teste automatizado integra o recorte e nenhum upload foi executado
nesta atuação.

### Integração e encerramento

O recorte validado foi integrado e sincronizado em `main`. A composição de
integração foi derivada diretamente da referência de produção e transportou
somente os commits do console, excluindo a especificação não relacionada de
leitura de corrente contínua que era ancestral da branch original. A busca
final confirmou a ausência desse artefato e de seus registros na composição
integrada.

Com a integração em estado terminal, a entrega é promovida de Pronta para
integração [`Ready for Integration`] para Concluída [`Done`] e esta transação é
encerrada.

## EKM-CHG-0043 — Autoria da especificação de corrente fotovoltaica 0.2

**Estado:** Superseded — versão 0.2 substituída pelo Draft 0.3

**Especificação relacionada:** `IOTSSC-CURRENT-SENSOR@0.2`

### Objetivo

Incorporar a decisão do Arquiteto que substitui CUR-AC-004 por CUR-DC-004,
delimita a medição entre painel fotovoltaico e entrada do buck e contrata dois
perfis configuráveis do ACS712-30A, com estados explícitos de medição e
alimentação.

### Baseline

- Branch `spec/current-sensing-capability`, derivada de `main`.
- Árvore limpa no início da atuação.
- Relatório de análise 0.1 preservado como histórico imutável.

### Decisões incorporadas

- perfis `ACS712_30A_5V` (`MANUFACTURER_SUPPORTED`) e
  `ACS712_30A_3V3` (`PROJECT_VALIDATED`), com sensibilidades iniciais de
  `43,05 mV/A` e `43,56 mV/A`;
- cálculo referido ao ADC, aquecimento inicial de 60 segundos, recalibração com
  2 segundos de acomodação e zero separado da configuração nominal;
- faixa calibrada de `0,50–15,00 A` em magnitude, erro
  `max(0,10 A; 5%)`, faixa morta de `0,05 A`, estabilidade e resposta;
- envelope indivisível com estados de medição e alimentação, incluindo
  `NOT_MONITORED`, sobrefaixa e saturação sem valor numérico válido;
- API aditiva `app.addCurrentSensor(CurrentSensorConfig)`, com adapter e
  capability pertencentes à aplicação e ponteiro retornado não proprietário,
  conforme o padrão vigente;
- validação separada por perfil, incluindo `15,00 A`, `−0,50 A`, `−5,00 A` e
  injeção instrumental acima de 15 A;
- nenhum artefato de teste automatizado integra o recorte.

### Lacunas

`EKM-GAP-0012` consolida `CUR-GAP-001` a `CUR-GAP-006`: valores de
`maximumZeroDeviationMv`, `adcMaximumMv` e `sampleIntervalUs`; representação da
faixa estimada; alcance de conflitos de GPIO; e representação textual do
envelope. Todas são bloqueantes antes de nova análise de implementabilidade.

### Fontes alteradas

- `docs/specs/CURRENT-SENSING-CAPABILITY.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

### Evidências

- decisões explícitas do Arquiteto incorporadas ao Draft 0.2;
- baseline de ownership e falha confrontada com
  `PUBLIC-API-COMPATIBILITY`;
- ciclo, oito slots e configuração prévia confrontados com
  `CORE-RUNTIME-LIFECYCLE`;
- nenhum build, teste, upload ou validação física iniciado, por se tratar de
  atuação documental.

### Resultado

`IOTSSC-CURRENT-SENSOR@0.2` permaneceu `Draft`, `Not Started` e
`Pending Review`. CUR-DC-004 responde ao bloqueador da análise 0.1, mas a versão
não é elegível à implementação enquanto `EKM-GAP-0012` permanecer aberto.

A análise formal
`docs/reports/2026-08-27T005456Z-0.2-f5d3f2a4-implementability-analysis.md`
classificou a versão como `Not Ready — Specification Defect` e confirmou os
seis bloqueadores consolidados em `EKM-GAP-0012`. O Arquiteto decidiu seus
contratos e autorizou a versão 0.3 em `EKM-CHG-0044`.

## EKM-CHG-0044 — Autoria da especificação de corrente fotovoltaica 0.3

**Estado:** Superseded — versão 0.3 substituída pelo Draft 0.4

**Especificação relacionada:** `IOTSSC-CURRENT-SENSOR@0.3`

### Objetivo

Incorporar as decisões normativas do Arquiteto que respondem integralmente ao
relatório de implementabilidade 0.2, preservando a versão 0.2 e seu relatório
como histórico e sem alterar código de produção ou testes.

### Decisões incorporadas

- target inicial ESP32 clássico, com `FULL_RANGE` resolvida como `ADC_11db`,
  faixa utilizável de `150–3100 mV`, `sampleIntervalUs = 1000` e
  `maximumZeroDeviationMv = 100`, aplicáveis aos dois perfis elétricos;
- `adcMinimumMv` adicionado ao contrato e ausência de herança silenciosa desses
  limites por ESP32-C3, ESP32-C6, ESP32-S3 ou outro SoC;
- aquisição e calibração incrementais, com no máximo uma leitura ADC por
  oportunidade elegível de `handle()`, sem espera ativa ou lote bloqueante;
- estados públicos `NOT_READY` e `ESTIMATED`, separando presença de valor
  numérico de garantia contratada de exatidão;
- rejeição de conflitos limitada aos sensores de corrente da mesma instância,
  às capacidades e reservas do target e ao identificador já usado por qualquer
  capability, sem registro central transversal de GPIO;
- envelope textual UTF-8 em JSON compacto normalizado, com ordem fixa, três
  casas decimais, tokens estáveis, `null` e comparação byte a byte.

### Reconciliação

`EKM-GAP-0012` foi encerrada: `CUR-DEC-012` a `CUR-DEC-016` substituem os seis
parâmetros ou contratos pendentes da versão 0.2. A relação normativa passa a
`Corrects` em relação à versão 0.2. Os relatórios de análise 0.1 e 0.2
permanecem imutáveis. `PUBLIC-API-COMPATIBILITY` e
`CORE-RUNTIME-LIFECYCLE` continuam preservadas pela extensão aditiva, ownership
vigente, limite de oito slots, configuração prévia e aquisição cooperativa.

### Fontes alteradas

- `docs/specs/CURRENT-SENSING-CAPABILITY.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

### Evidências e limites

- decisões explícitas do Arquiteto incorporadas ao Draft 0.3;
- integridade e rastreabilidade documentais confrontadas com a baseline;
- nenhum código, teste, build, upload ou validação física iniciado, por se
  tratar de atuação exclusivamente documental.

### Resultado

`IOTSSC-CURRENT-SENSOR@0.3` permaneceu `Draft`, `Not Started` e `Pending Review`.
A especificação foi encaminhada para nova análise formal de implementabilidade;
nenhuma implementação está autorizada antes de classificação `Ready` aplicável
à versão 0.3 e ordem explícita posterior do Arquiteto.

A análise formal
`docs/reports/2026-08-27T011809Z-0.3-0c86c4a9-implementability-analysis.md`
classificou a versão como `Not Ready — Specification Defect`: faltavam o estado
da alimentação antes da primeira amostra válida e a transição observável após
calibração de zero inválida. O Arquiteto decidiu esses contratos, revogou o JSON
dentro de `value` e autorizou a versão 0.4 em `EKM-CHG-0045`.

## EKM-CHG-0045 — Autoria da especificação de corrente fotovoltaica 0.4

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-CURRENT-SENSOR@0.4`

### Objetivo

Incorporar as decisões normativas do Arquiteto que respondem ao relatório de
implementabilidade 0.3 e preservam `ICapability::value` como contrato escalar,
mantendo as versões e relatórios anteriores como histórico imutável e sem
alterar código de produção ou testes.

### Decisões incorporadas

- `UNKNOWN` representa alimentação monitorável ainda sem amostra válida e
  mantém o valor escalar vazio;
- `CALIBRATING` representa acomodação e amostragem de zero;
  `ZERO_CALIBRATION_FAILED` representa zero rejeitado, mantém o valor vazio e
  impede uso do zero anterior até calibração válida ou reinício;
- `ICapability::value` e `CapabilityStateChanged::value` permanecem
  `std::string` escalares: corrente disponível usa três casas decimais e estados
  sem valor usam string vazia; nenhuma string JSON é armazenada em `value`;
- `CapabilityStateChanged` recebe `measurementStatus` e `supplyStatus`
  opcionais sem invalidar campos, assinaturas ou construtores existentes;
- serializadores e sink acrescentam os estados somente quando presentes;
  capabilities existentes não recebem estados sintéticos nem mudam seus
  eventos;
- detecção de mudança da capability de corrente usa conjuntamente `value` e os
  dois estados;
- o anúncio geral permanece em seu formato atual, sem os estados operacionais.

### Reconciliação

`EKM-GAP-0013` registra e encerra os dois bloqueadores da análise 0.3. A decisão
de JSON interno da versão anterior foi expressamente revogada. A relação
normativa passa a `Corrects` em relação à versão 0.3. `PUBLIC-API-COMPATIBILITY`
é preservada pelos campos opcionais e pela manutenção integral dos contratos
preexistentes; `CORE-RUNTIME-LIFECYCLE` permanece preservada pela aquisição
cooperativa e pelos oito slots vigentes.

### Fontes alteradas

- `docs/specs/CURRENT-SENSING-CAPABILITY.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

### Evidências e limites

- decisões explícitas do Arquiteto incorporadas ao Draft 0.4;
- evento e serializadores vigentes confrontados para preservar campos,
  construtores, `capability_name`, `value` e anúncio;
- nenhum código, teste, build, upload ou validação física iniciado, por se
  tratar de atuação exclusivamente documental.

### Resultado

`IOTSSC-CURRENT-SENSOR@0.4` permanece `Draft`, `Not Started` e `Pending Review`.
A especificação foi encaminhada para nova análise formal de implementabilidade;
nenhuma implementação está autorizada antes de classificação `Ready` aplicável
à versão 0.4 e ordem explícita posterior do Arquiteto.

O relatório formal
`docs/reports/2026-08-27T015112Z-0.4-5f4b0c45-implementability-analysis.md`
classificou a versão 0.4 como `Ready`, descartou os dois bloqueadores da versão
0.3 e tornou a revisão elegível à ordem explícita de implementação registrada
em `EKM-CHG-0046`.

## EKM-CHG-0046 — Implementação da leitura de corrente fotovoltaica 0.4

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-CURRENT-SENSOR@0.4`

### Objetivo

Implementar integralmente a versão 0.4 classificada como `Ready`, preservando o
runtime Arduino/ESP32, o valor escalar e os consumidores existentes, sem criar
testes automatizados nem executar operações físicas não autorizadas.

### Implementação

- adicionados os tipos públicos, dois perfis elétricos, `ICurrentSensor`, o
  adapter `ACS712C30ACurrentSensor` e `CurrentSensorCapability`;
- implementadas máquinas cooperativas de aquecimento, calibração, recalibração
  e leitura, com ADC calibrado em mV, média, filtro, faixas, saturação, estados
  assinados e monitor opcional da alimentação;
- `CapabilityStateChanged` e `MqttSink` passaram a emitir os dois estados apenas
  quando presentes, mantendo construtores, `value`, payload legado e anúncio;
- `CapabilitiesBuilder` e `SmartSysApp::addCurrentSensor()` implementam
  validação completa, conflitos locais, ownership, identidade e rollback sem
  registro parcial; a destruição da aplicação libera os objetos da arena;
- o target rejeita SoCs não contratados e reserva ADC2 por conflito com o Wi-Fi
  do runtime, usando ADC1 e `ADC_11db` no ESP32 clássico;
- corrigida a configuração efetiva de C++17: `build_unflags` deixou de remover
  o mesmo `-std=gnu++17` declarado em `build_flags`.

### Evidências e limites

- `git diff --check`: aprovado;
- primeiro `pio run -e esp32_dev`: `FAILED` (código 1), revelando remoção
  indevida de C++17; causa corrigida no recorte;
- build intermediário: `SUCCESS` (código 0, 25,683 s);
- build final `pio run -e esp32_dev`: `SUCCESS` (código 0, 11,552 s), RAM
  `81244/327680` e flash `1829269/2031616` bytes;
- nenhum teste automatizado foi criado ou executado, conforme o recorte;
- validações físicas e instrumentadas, upload, monitor, release e deploy não
  foram executados e permanecem `Not Executed` quando aplicáveis.

O relatório
`docs/reports/2026-08-27T021250Z-0.4-968b321a-implementation-report.md`
registra decisões locais, evidências e limitações da atuação.

### Resultado

`IOTSSC-CURRENT-SENSOR@0.4` permanece `Draft`, passa a `Implemented` e conserva
a entrega `Not Applicable`. A análise aplicável é `Ready`; o resultado foi
encaminhado à revisão técnica, sem declaração de conclusão ou integração.

## EKM-CHG-0047 — Autoria e análise da correção 0.5 da corrente fotovoltaica

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-CURRENT-SENSOR@0.5`

### Objetivo

Corrigir a omissão da autoria 0.4, que contratou a capability de corrente sem
contratar seu exemplo executável, preservando integralmente comportamento,
contratos públicos, estados, faixas e critérios já implementados.

### Decisões incorporadas

- `CUR-DEC-019`: o consumo da capability passa a ser demonstrado pelo exemplo
  `current_sensor` na MCB R1, com o símbolo oficial `ITS_MCB01_J4_EXT_ADC`;
- `CUR-DEC-020`: o environment versionado usa o perfil de 3,3 V e mantém o
  perfil de 5 V selecionável em build time;
- `CUR-DEC-021`: o exemplo não monitora a alimentação, permanecendo em
  `NOT_MONITORED` sem afirmar exatidão contratada;
- `CUR-DEC-022`: a recalibração é demonstrada somente por estímulo local no
  monitor serial.

A correção acrescenta `CUR-046` a `CUR-054`, `CUR-AC-015` a `CUR-AC-017` e a
relação normativa com `IOTSSC-HW-EXAMPLES`, cujo contrato de catálogo, pinout e
seleção por environment é herdado sem alteração.

### Análise de implementabilidade

O relatório
`docs/reports/2026-08-27T131108Z-0.5-ae82abb8-implementability-analysis.md`
classificou a versão 0.5 como `Ready`, sem bloqueadores, reconciliando as
restrições não bloqueantes do relatório 0.4 e confirmando que `HWEX-023` não é
acionado porque o pinout declara símbolo inequívoco de entrada analógica.

### Fontes alteradas

- `docs/specs/CURRENT-SENSING-CAPABILITY.md`;
- `docs/reports/2026-08-27T131108Z-0.5-ae82abb8-implementability-analysis.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

## EKM-CHG-0048 — Implementação do exemplo executável de corrente fotovoltaica

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-CURRENT-SENSOR@0.5`

### Objetivo

Implementar `CUR-046` a `CUR-054` após a classificação `Ready` da versão 0.5 e
ordem explícita do Arquiteto, sem alterar a capability, o runtime, os exemplos
preexistentes ou o build padrão.

### Implementação

- criado `examples/executable/current_sensor/` com aplicação Arduino única e
  README completo, consumindo apenas a API pública e os acessos não
  proprietários da capability;
- o sinal usa exclusivamente `ITS_MCB01_J4_EXT_ADC`; o perfil elétrico, o
  identificador da capability e a cadência de apresentação vêm do environment;
- `src/ExecutableExampleRunner.cpp` recebeu o seletor exclusivo
  `IOTSMARTSYS_EXAMPLE_CURRENT_SENSOR`;
- `configs/executable_examples.ini` recebeu `example_current_sensor_mcb_r1` com
  o perfil de 3,3 V;
- `examples/README.md` incorporou o exemplo ao catálogo.

### Evidências e limites

- `pio run -e example_current_sensor_mcb_r1`: `SUCCESS` (código 0, 25,969 s);
- `pio run -e esp32_dev`: `SUCCESS` (código 0, 26,792 s);
- `pio run -e example_basic_light_mcb_r1`: `SUCCESS` (código 0, 25,768 s);
- exatamente um `setup()` e um `loop()` no firmware do exemplo;
- `pio project config --json-output` e `git diff --check`: aprovados;
- upload, monitor, hardware e validação física não foram executados e
  permanecem `Not Executed`; nenhum teste automatizado foi criado ou executado.

O relatório
`docs/reports/2026-08-27T131541Z-0.5-892dccb7-implementation-report.md`
registra decisões locais, evidências e limitações da atuação.

### Resultado

`IOTSSC-CURRENT-SENSOR@0.5` permanece `Draft`, com implementação `Implemented` e
entrega `Not Applicable`. O resultado foi encaminhado à revisão técnica, sem
declaração de conclusão ou integração.

## EKM-CHG-0049 — Autoria da cadência configurável da corrente fotovoltaica 0.6

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-CURRENT-SENSOR@0.6`

### Objetivo

Corrigir a ausência de contrato para o intervalo de avaliação da capability de
corrente, preservando a aquisição cooperativa do adapter e a compatibilidade das
APIs preexistentes.

### Decisões incorporadas

- `CurrentSensorConfig` recebe `capabilityEvaluationIntervalMs`, estritamente
  positivo e com default de `1000 ms`;
- o timestamp da última avaliação permanece estado privado da capability e não
  integra a configuração pública;
- a primeira avaliação é imediata e as posteriores respeitam o intervalo
  configurado, atualizando o timestamp mesmo sem publicação;
- `ICurrentSensor::handle()` continua sendo acionado em todo ciclo,
  independentemente da elegibilidade da avaliação.
- o construtor público preexistente da capability permanece válido com default
  de `1000 ms`, enquanto o registro por `CurrentSensorConfig` transfere o valor
  configurado por caminho aditivo.

### Fontes alteradas

- `docs/specs/CURRENT-SENSING-CAPABILITY.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

### Resultado

`IOTSSC-CURRENT-SENSOR@0.6` permanece `Draft`, com implementação `Not Started`,
entrega `Not Applicable` e revisão de implementabilidade `Pending Review`. A
versão 0.5 e suas evidências permanecem históricas. Nenhum código de produção,
teste ou configuração de build foi alterado nesta transação.

## EKM-CHG-0050 — Análise de implementabilidade da corrente fotovoltaica 0.6

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-CURRENT-SENSOR@0.6`

### Objetivo

Confrontar integralmente a versão 0.6, sua cadência configurável, a baseline
Arduino/ESP32, a compatibilidade pública e os achados anteriores.

### Resultado

Classificação **Pronta** [`Ready`], sem bloqueadores. O relatório
`docs/reports/2026-08-27T172407Z-0.6-841c79da-implementability-analysis.md`
registra a cobertura, o challenge, a reconciliação anterior e quatro restrições
não bloqueantes. A especificação permanece `Draft`, com implementação
`Not Started` e entrega `Not Applicable`; esta análise não autoriza
implementação, conclusão ou integração.

## EKM-CHG-0051 — Implementação da cadência configurável da corrente fotovoltaica 0.6

**Estado:** Closed

**Especificação relacionada:** `IOTSSC-CURRENT-SENSOR@0.6`

### Objetivo

Implementar `capabilityEvaluationIntervalMs` e CUR-055 a CUR-058 após a análise
formal `Ready` e a ordem explícita do Arquiteto, preservando a aquisição
cooperativa e as APIs públicas preexistentes.

### Implementação

- `CurrentSensorConfig` recebeu ao final do aggregate o intervalo de avaliação,
  com default de `1000 ms`, preservando inicializações posicionais existentes;
- o builder rejeita intervalo zero e transfere a configuração para a capability;
- o construtor público de três argumentos permanece válido com default de
  `1000 ms`, acompanhado por sobrecarga aditiva;
- `ICurrentSensor::handle()` continua executando em todo ciclo, enquanto a
  avaliação ocorre imediatamente na primeira oportunidade e depois somente
  quando o intervalo decorre;
- o timestamp é atualizado em cada avaliação elegível, mesmo quando o envelope
  normalizado não muda e nenhum evento é publicado.

### Evidências e limites

- `pio run -e esp32_dev`: `SUCCESS` (código 0, 26,661 s);
- `pio run -e example_current_sensor_mcb_r1`: `SUCCESS` (código 0, 11,302 s);
- `git diff --check`: aprovado;
- CUR-AC-018, upload, monitor e hardware permanecem `Not Executed`;
- nenhum teste automatizado foi criado ou executado, conforme o recorte da
  validação instrumentada estabelecido pela especificação.

O relatório
`docs/reports/2026-08-27T173207Z-0.6-0b22fd5d-implementation-report.md`
registra decisões locais, evidências, rastreabilidade e limitações da atuação.

### Resultado

`IOTSSC-CURRENT-SENSOR@0.6` permanece `Draft`, com implementação `Implemented`,
entrega `Not Applicable` e análise de implementabilidade `Ready`. O resultado
foi encaminhado à revisão técnica, sem declaração de validação, conclusão ou
integração.
