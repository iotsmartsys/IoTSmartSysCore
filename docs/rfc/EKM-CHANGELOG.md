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

- `EXECUTABLE-HARDWARE-EXAMPLES.md`: `Active` / `Not Started`;
- `EKM-GAP-0006`: `Closed`;
- nenhuma implementação iniciada.

### Critério de encerramento

Implementação e validação dos requisitos da especificação, reconciliação das evidências e atualização do estado da implementação. A aprovação documental foi concluída, mas a transação permanece `Open` até essa fase.
