# Instruções para agentes

Este repositório adota o EKM (Engineering Knowledge Management). Antes de alterar código, build, testes, automação ou documentação, leia:

1. `docs/rfc/EKM-GUIDELINES.md`;
2. `docs/rfc/KNOWLEDGE-MAP.md`;
3. as especificações ativas relacionadas ao recorte em `docs/specs/`;
4. `docs/rfc/EKM-CHANGELOG.md`, quando houver transação aberta para o trabalho.

## Regras obrigatórias

- A especificação define o comportamento esperado; não invente contratos ausentes.
- Preserve APIs públicas e comportamentos normativos, salvo autorização explícita registrada em especificação.
- Trate o worktree inicial, incluindo alterações não commitadas, como parte do baseline.
- Não apague, condense ou reescreva conhecimento normativo sem declarar a mudança.
- Toda alteração funcional deve possuir transação EKM `Open` antes da implementação e só pode ser `Closed` após reconciliação de código, especificações, mapa e validações.
- Se surgir ambiguidade que exija decisão de produto ou arquitetura, registre o bloqueio e solicite decisão humana.
- Não execute `git add`, commit, tag, push, criação de branch ou PR sem autorização explícita.
- Preserve alterações preexistentes e não relacionadas.

## Relatório obrigatório

O relatório final deve informar, de forma objetiva:

- resultado executivo;
- requisitos atendidos e não atendidos;
- arquivos de código, build e conhecimento alterados;
- contratos adicionados, modificados ou removidos;
- validações executadas e pendentes;
- riscos, desvios e lacunas descobertas;
- estado final da transação EKM;
- operações Git ou externas realizadas.
