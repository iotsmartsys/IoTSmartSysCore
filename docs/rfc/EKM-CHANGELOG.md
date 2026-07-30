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
