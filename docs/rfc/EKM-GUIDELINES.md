# EKM — Diretrizes locais

**Classe da fonte:** Normativa

**Estado da fonte:** Vigente

**Versão do documento:** 1.6

**Versão do modelo EKM:** 1.9

**Escopo:** Todo o repositório

## 1. Autoridade

O Arquiteto humano tem autoridade final sobre intenção, prioridade, escopo,
arquitetura, risco, autorização, validação e integração. A ordem recebida por
prompt ou pipeline define a etapa autorizada.

Agentes não inventam requisitos nem expandem o recorte. Evidências factuais
permanecem factuais mesmo quando o Arquiteto aceita o risco.

## 2. Fontes

- especificações definem comportamento e aceite;
- estas diretrizes definem regras locais;
- o mapa localiza fontes e lacunas;
- o changelog registra decisões, lacunas, evidências e resultados;
- código e testes implementam e evidenciam;
- relatórios não criam requisitos.

Git registra commits, autoria, diferenças, branches e linhagem. Esses dados não
devem ser duplicados manualmente nas fontes EKM.

Estas diretrizes adotam o modelo 1.9 do repositório de referência
`EKM-guidelines`. Regras específicas deste projeto prevalecem apenas dentro de
seu escopo declarado.

## 3. Fluxo

```text
especificação
→ ordem do Arquiteto
→ análise de implementabilidade
→ ordem do Arquiteto
→ implementação e validação
→ decisão humana e integração
```

Implementação exige especificação Implementável [`Implementable`]. Precisa de
esclarecimento [`Needs Clarification`] retorna a decisão ao Arquiteto sem
alteração parcial da implementação.

A análise cobre o necessário para sustentar seu resultado. Uma lacuna
bloqueante permite concluir `Needs Clarification` quando a decisão necessária
estiver clara, registrando também outros bloqueios materiais já observados, sem
matriz universal ou investigação exaustiva.

Revisões adicionais acontecem somente quando solicitadas.

## 4. Contrato Git

Toda tarefa de agente começa com árvore limpa, produz resultado material,
termina com commit e push e deixa a árvore limpa. Push com falha significa etapa
não entregue.

A tarefa não autoriza force push, reescrita de histórico, merge, tag, release ou
deploy sem ordem específica.

Git é a fonte da linhagem técnica. Branches, SHAs, checkpoints e cadeias de
commits não são repetidos em documentos EKM, salvo quando necessários para
explicar decisão ou desvio material.

## 5. Preservação

- Não remover ou enfraquecer decisão vigente silenciosamente.
- Não substituir fonte normativa por resumo incompleto.
- Não resolver conflito normativo por preferência do agente.
- Atualizar conhecimento afetado na mesma mudança.
- Registrar lacunas que precisem sobreviver à tarefa.
- Preservar alterações preexistentes e não relacionadas.

## 6. Especificações e estados

Uma especificação incremental é a unidade de comportamento e delegação. Ela
registra objetivo, escopo, requisitos verificáveis, contratos e falhas
relevantes, critérios de aceite, relações e resultado da análise de
implementabilidade.

Estados normativos, de implementação, de entrega e de implementabilidade são
independentes e seguem o modelo EKM 1.9. Versões concluídas são preservadas;
mudanças posteriores usam `Amends`, `Supersedes`, `Corrects` ou `Retires`.

## 7. Transações

Mudanças usam `EKM-CHG-NNNN`; lacunas usam `EKM-GAP-NNNN`.

Uma transação registra somente objetivo, especificação relacionada, decisões,
lacunas, evidências materiais, estado e resultado. Ela não funciona como diário
de comandos nem como espelho do Git.

A transação é concluída quando o recorte autorizado foi entregue por commit e
push, as fontes afetadas estão atuais, as evidências materiais foram registradas
e as lacunas restantes estão explícitas. O estado da entrega da especificação
informa separadamente se houve integração.

## 8. Regras específicas do projeto

- `main` é a referência de produção.
- O runtime suportado é Arduino sobre ESP32.
- ESP-IDF permanece preparação futura e não pode degradar o runtime vigente.
- ESP8266 não constitui plataforma suportada.
- APIs públicas e comportamentos normativos devem ser preservados, salvo mudança
  aprovada em especificação.
- Configuração de capabilities termina antes de `SmartSysApp::setup()`.
- O limite intencional é de oito capabilities por aplicação.
- Mudanças funcionais relevantes seguem *specification on touch*.
- Não existe `EKM Gate` ou garantia automatizada vigente.

## 9. Adoção e proporcionalidade

O projeto mantém adoção incremental: inventaria em largura, aprofunda domínios
tocados e aplica controles proporcionais ao risco. Controles sem evidência de
ganho para conhecimento, decisão, auditabilidade, verificação ou velocidade não
se tornam obrigações universais.
