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
