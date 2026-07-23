# Instruções para agentes

Este repositório adota o EKM (Engineering Knowledge Management). Antes de alterar código, build, testes, automação ou documentação, leia:

1. `docs/rfc/EKM-GUIDELINES.md`;
2. `docs/rfc/KNOWLEDGE-MAP.md`;
3. as especificações ativas relacionadas ao recorte em `docs/specs/`;
4. `docs/rfc/EKM-CHANGELOG.md`, quando houver transação aberta para o trabalho.

## Regras obrigatórias

- A especificação define o comportamento esperado; não invente contratos ausentes.
- Antes de qualquer alteração de implementação, execute uma análise técnica integral de implementabilidade da especificação e registre o resultado como `Implementable` ou `Needs Clarification`.
- Somente uma especificação `Implementable` pode entrar em implementação. Se qualquer requisito obrigatório exigir inferência relevante, não implemente nenhum item do recorte.
- Em `Needs Clarification`, apresente as lacunas, evidências, decisões ausentes e o ajuste recomendado na própria especificação; aguarde aprovação e repita a análise integral.
- Enquanto estiver em `Needs Clarification`, altere somente registros EKM e a especificação cuja correção tenha sido explicitamente aprovada.
- Preserve APIs públicas e comportamentos normativos, salvo autorização explícita registrada em especificação.
- Trate o worktree inicial, incluindo alterações não commitadas, como parte do baseline.
- Não apague, condense ou reescreva conhecimento normativo sem declarar a mudança.
- Toda alteração funcional deve possuir transação EKM `Open` antes da implementação e só pode ser `Closed` após reconciliação de código, especificações, mapa e validações.
- Não transforme descoberta de implementação em decisão de produto, arquitetura, contrato, compatibilidade, persistência, segurança ou comportamento.
- Neste projeto, `main` é a referência de produção. Não reescreva uma versão de especificação já integrada; crie uma nova especificação relacionada como `Amends`, `Supersedes`, `Corrects` ou `Retires`.
- Não presuma que exista validação automática da EKM. O futuro `EKM Gate` está previsto, mas ainda não foi definido nem implantado.
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
