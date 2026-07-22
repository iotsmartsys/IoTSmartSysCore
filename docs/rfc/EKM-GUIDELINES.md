# EKM Guidelines — IoTSmartSysCore

**Status:** Active

**Versão:** 1.0

**Modelo EKM:** 1.3

**Última atualização:** 22/07/2026

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

Cada especificação possui dois estados independentes.

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

## 6. Transação EKM

Toda mudança funcional ou normativa relevante usa um identificador `EKM-CHG-NNNN` e um destes estados:

- `Open`;
- `Blocked`;
- `Superseded`;
- `Closed`.

Uma transação registra objetivo, baseline, fontes afetadas, requisitos, evidências, desvios e decisão de encerramento. Lacunas usam `EKM-GAP-NNNN` com os mesmos estados.

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

## 8. Definition of Done EKM

Uma transação só pode ser `Closed` quando:

- requisitos e escopo foram rastreados;
- mudanças estão reconciliadas com as fontes normativas;
- nenhuma decisão foi removida silenciosamente;
- validações exigidas foram executadas ou explicitamente registradas como pendentes;
- mapa e lacunas refletem o estado real;
- relatório permite auditar o resultado;
- operações Git/externas foram declaradas.

Build aprovado, isoladamente, não comprova conformidade EKM.

## 9. Interrupções e regressões

Ao encontrar regressão, contradição ou perda de conhecimento:

1. interromper expansão do escopo;
2. reabrir a transação ou lacuna relacionada;
3. preservar evidências;
4. corrigir implementação e/ou fonte normativa conforme decisão humana;
5. repetir as validações afetadas;
6. encerrar novamente somente após reconciliação.
