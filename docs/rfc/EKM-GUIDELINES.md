# EKM Guidelines — IoTSmartSysCore

**Status:** Active

**Versão:** 1.2

**Modelo EKM:** 1.5

**Última atualização:** 23/07/2026

## 1. Objetivo

Preservar conhecimento suficiente para compreender, evoluir, auditar e reconstruir os comportamentos relevantes do sistema sem depender de conversas, memória individual ou inferências do implementador.

O EKM separa:

- **especificações:** o que o sistema deve fazer;
- **diretrizes:** como o conhecimento e a implementação devem ser tratados;
- **mapa:** onde está a fonte de verdade e qual sua cobertura;
- **changelog:** histórico das transações de conhecimento;
- **relatórios:** evidências de uma execução, nunca fonte normativa isolada.

## 2. Fontes de conhecimento

| Fonte | Papel | Normativa |
|---|---|---|
| `AGENTS.md` | Entrada obrigatória para agentes | Sim |
| `docs/rfc/EKM-GUIDELINES.md` | Regras EKM | Sim |
| `docs/rfc/KNOWLEDGE-MAP.md` | Índice, cobertura e lacunas | Sim |
| `docs/rfc/EKM-CHANGELOG.md` | Histórico transacional | Sim |
| `docs/specs/*.md` | Comportamentos e contratos | Sim, conforme status |
| Código e testes | Implementação e evidência executável | Não substituem a especificação |
| `docs/REPO_DOSSIER.md` | Levantamento histórico | Informativa |

Conflitos entre fontes normativas devem bloquear a mudança até reconciliação humana.

## 3. Estados das especificações

Cada especificação possui estados normativo, de implementação e de entrega independentes.

### 3.1 Estado normativo

- `Draft`: conteúdo em elaboração.
- `Proposed`: pronto para decisão.
- `Approved`: aprovado, ainda não vigente.
- `Active`: fonte de verdade vigente.
- `Superseded`: substituído por outra especificação identificada.
- `Withdrawn`: proposta retirada antes de entrar em vigor.
- `Archived`: preservado apenas como histórico.

### 3.2 Estado de implementação

- `Not Started`: implementação não iniciada.
- `In Progress`: atendimento parcial.
- `Implemented`: implementação encontrada ou concluída, sem validação suficiente para declarar conformidade plena.
- `Validated`: implementação comprovada pelos critérios da especificação.
- `Regressed`: comportamento antes atendido deixou de ser atendido.
- `Blocked`: impedimento conhecido.
- `Retired`: implementação deliberadamente removida.

Alterar um estado exige evidência registrada no changelog.

### 3.3 Estado da entrega

- `Not Ready`: ainda não satisfaz todos os requisitos de integração.
- `Ready for Integration`: implementação, validações e conhecimento estão reconciliados e podem seguir para PR/merge.
- `Done`: a versão da especificação e sua implementação foram integradas à referência de produção.

Neste projeto, a referência inicial de produção é a branch `main`. `Implemented` informa que o código foi produzido; `Validated`, que o comportamento foi comprovado; `Done`, que conhecimento e implementação chegaram à produção. Esses estados não são equivalentes.

## 4. Adoção em projeto legado

A adoção é incremental e otimizada para evitar leituras e documentação indiscriminadas.

### 4.1 Níveis de cobertura

- `Unmapped`: domínio ainda não localizado.
- `Inventoried`: arquivos e símbolos principais identificados.
- `Mapped`: fluxo e dependências principais conhecidos.
- `Reviewed`: contratos e riscos confrontados com o código.
- `Specified`: fonte normativa criada e reconciliada com o comportamento desejado.
- `Reconstructible`: especificação, implementação e validações permitem reconstruir o comportamento sem inferência relevante.

### 4.2 Estratégia

1. Registrar branch, commit e estado real do worktree.
2. Mapear o repositório em largura: módulos, entradas, builds, testes e releases.
3. Aprofundar apenas domínios prioritários ou tocados pela mudança.
4. Aplicar **specification on touch**: toda funcionalidade relevante modificada deve atingir ao menos `Specified`.
5. Registrar lacunas sem tentar documentar todo o legado de uma só vez.

## 5. Requisitos mínimos de uma especificação

Uma especificação deve conter:

- identificador, título e estados;
- objetivo, contexto e escopo;
- comportamento e requisitos identificáveis;
- invariantes e contratos preservados;
- falhas e condições de borda relevantes;
- fora de escopo;
- critérios de aceite e validações;
- relações com outras fontes normativas;
- lacunas ou desvios conhecidos.

Implementadores não podem preencher lacunas de produto ou arquitetura por suposição.

### 5.1 Technical Readiness Review

Antes de modificar código, build, testes, automação ou documentação de implementação, o executor deve analisar integralmente a especificação e seu baseline. A análise deve verificar, no mínimo:

- clareza, consistência e testabilidade dos requisitos;
- comportamento esperado, falhas, condições de borda e fora de escopo;
- contratos e conhecimento que precisam ser preservados;
- dependências, configurações e pré-condições;
- compatibilidade, regressões e validações obrigatórias;
- mudanças necessárias que não estejam explicitamente autorizadas.

O resultado é binário:

- `Implementable`: todos os requisitos obrigatórios podem ser implementados sem inferência relevante;
- `Needs Clarification`: ao menos um requisito depende de decisão ausente, contraditória ou insuficientemente especificada.

Inferência relevante é qualquer escolha capaz de alterar comportamento observável, produto, arquitetura, API, protocolo, persistência, concorrência, segurança, compatibilidade, configuração operacional ou critério de aceite. Decisões mecânicas e privadas somente são permitidas quando forem comprovadamente equivalentes e não modificarem esses aspectos.

### 5.2 Atomicidade da implementação

Uma especificação forma uma unidade atômica de implementação:

1. nenhum artefato de implementação pode ser alterado antes do resultado `Implementable`;
2. se qualquer requisito obrigatório resultar em `Needs Clarification`, nenhum item da especificação pode ser implementado;
3. o executor deve registrar requisito afetado, evidência, lacuna ou conflito, decisão ausente, impacto das alternativas e ajuste recomendado na especificação;
4. o ajuste deve ocorrer na fonte normativa e ser aprovado pelo responsável;
5. após qualquer ajuste, a análise integral deve ser repetida;
6. implementação parcial exige uma nova especificação ou divisão de escopo explicitamente aprovada, nunca decisão unilateral do executor.

A análise não autoriza o executor a corrigir automaticamente a especificação nem a escolher uma alternativa. Sua finalidade é concentrar esclarecimentos antes da execução, permitindo que a implementação aprovada prossiga sem decisões normativas improvisadas.

Durante `Needs Clarification`, somente o registro da análise, a transação ou lacuna EKM e a correção normativa explicitamente aprovada podem ser alterados. Código, build, testes, automação e demais artefatos de implementação permanecem intocados.

### 5.3 Evolução antes e depois da produção

Antes de `Done`, a mesma especificação pode ser revisada, retornar a `Pending Review` ou `In Progress` e repetir a Technical Readiness Review.

Após `Done`, a identidade formada por ID e versão é imutável. Seu conteúdo não pode ser reescrito para representar comportamento posterior. Evoluções devem ser novas especificações e declarar uma relação explícita:

- `Amends`: complementa ou altera parte do comportamento;
- `Supersedes`: substitui integralmente;
- `Corrects`: corrige requisito ou comportamento defeituoso;
- `Retires`: remove deliberadamente a funcionalidade.

Eventos posteriores e mudanças de vigência são registrados no mapa e changelog, sem reescrever a especificação integrada. A fonte vigente é determinada pela especificação original combinada com suas relações ativas.

## 6. Transação EKM

Toda mudança funcional ou normativa relevante usa um identificador `EKM-CHG-NNNN` e um destes estados:

- `Open`;
- `Blocked`;
- `Superseded`;
- `Closed`.

Uma transação registra objetivo, baseline, fontes afetadas, requisitos, evidências, desvios e decisão de encerramento. Lacunas usam `EKM-GAP-NNNN` com os mesmos estados.

A transação deve preservar o resultado e as evidências da Technical Readiness Review que autorizou a implementação.

## 7. Baseline e reconciliação

O baseline é o estado observado no início da tarefa, não apenas `HEAD`. Devem ser preservados:

- arquivos rastreados e não rastreados relevantes;
- alterações já existentes;
- contratos observáveis;
- conhecimento normativo vigente.

Antes do encerramento, reconciliar separadamente:

1. código-fonte;
2. build e automação;
3. testes e evidências;
4. especificações e governança;
5. todas as diferenças em relação ao worktree inicial.

## 8. Definition of Ready for Integration e Done

Uma mudança funcional alcança `Ready for Integration` quando:

- a Technical Readiness Review válida declarou a especificação `Implementable` antes da primeira alteração de implementação;
- requisitos e critérios de aceite foram atendidos;
- validações obrigatórias foram executadas com sucesso;
- implementação e fontes de conhecimento foram reconciliadas;
- mapa, lacunas, relatório e operações externas refletem o estado real;
- não existe pendência obrigatória ou bloqueante.

`Done` somente é alcançado quando a versão pronta e sua implementação são integradas à `main`, com evidência da integração. Validação pendente pode justificar `Implemented`, mas não `Ready for Integration` ou `Done` quando for critério obrigatório.

## 9. Definition of Done da transação EKM

Uma transação só pode ser `Closed` quando:

- a Technical Readiness Review válida declarou a especificação `Implementable` antes da primeira alteração de implementação;
- requisitos e escopo foram rastreados;
- mudanças estão reconciliadas com as fontes normativas;
- nenhuma decisão foi removida silenciosamente;
- validações exigidas foram executadas ou explicitamente registradas como pendentes;
- mapa e lacunas refletem o estado real;
- relatório permite auditar o resultado;
- operações Git/externas foram declaradas.

Build aprovado, isoladamente, não comprova conformidade EKM.

Para mudanças funcionais regidas pelo modelo 1.5, o encerramento requer `Done`. Transações exclusivamente investigativas ou de governança podem usar critério próprio aprovado, desde que não declarem entrega funcional.

## 10. Automação e garantias previstas

A EKM prevê um futuro mecanismo automatizado, denominado provisoriamente `EKM Gate`, para reduzir dependência de disciplina individual e proteger a integração à produção.

O objetivo futuro é verificar automaticamente aspectos comprováveis, como estrutura, metadados, relações entre especificações, imutabilidade de versões em produção, evidência de Technical Readiness, rastreabilidade, transições de estado e reconciliação exigida antes do merge.

O Gate e os mecanismos de Automação e Garantias estão em estado `Planned / Not Defined`:

- arquitetura, formato de metadados, regras executáveis e implantação ainda não foram especificados;
- não existe garantia automática vigente;
- sua ausência não pode ser apresentada como validação nem bloquear retroativamente o processo manual atual;
- completude semântica e decisões de intenção continuam sob responsabilidade humana, mesmo após futura automação.

A implementação dependerá de especificação própria, validação experimental e integração deliberada às políticas da `main`.

## 11. Interrupções e regressões

Ao encontrar regressão, contradição ou perda de conhecimento:

1. interromper expansão do escopo;
2. reabrir a transação ou lacuna relacionada;
3. preservar evidências;
4. corrigir implementação e/ou fonte normativa conforme decisão humana;
5. repetir as validações afetadas;
6. encerrar novamente somente após reconciliação.
