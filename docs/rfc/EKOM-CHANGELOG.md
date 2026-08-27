# Histórico de mudanças EKOM — IoTSmartSysCore

Este arquivo registra transações iniciadas sob EKOM 4.6. O histórico anterior
permanece preservado em `docs/rfc/EKM-CHANGELOG.md`.

## EKOM-CHG-0001 — Migração da governança para EKOM 4.6

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** Não se aplica [`Not Applicable`]

**Objetivo:** atualizar a fundação documental do repositório da EKM 1.19 para
o EKOM 4.6 sem alterar código, testes, dependências, build, automações ou
configuração funcional.

### Decisões relacionadas

- adoção do método, regras comuns, perfis e templates vigentes do EKOM 4.6;
- preservação não retroativa do histórico e dos identificadores EKM 1.x;
- novas transações, lacunas e débitos usam o namespace `EKOM`;
- a especificação relacionada é `Not Applicable` por se tratar de governança.

### Lacunas

- nenhuma lacuna nova foi aberta.

### Débitos técnicos relacionados

- nenhum débito técnico foi aceito nesta transação.

### Relatórios e evidências materiais

- confronto documental com `EKOM-METHOD.md`, `GOVERNANCE.md`,
  `DESIGN-DECISIONS.md`, perfis e templates oficiais do EKOM 4.6;
- guarda estrutural EKOM 4.6 e `git diff --check` executados sobre o delta
  documental.
- o Consultor participou da migração e não alega revisão independente deste
  mesmo recorte.

### Resultado

O roteamento de agentes, as diretrizes locais, o mapa de conhecimento e o
adaptador do Claude Code passam a referenciar o EKOM 4.6. As fontes EKM 1.x
permanecem históricas; código, testes, dependências, build, automações e
configuração funcional não foram alterados.
