# EKM — Diretrizes locais

**Classe da fonte:** Normativa

**Estado da fonte:** Vigente

**Versão do documento:** 1.14

**Versão do modelo EKM:** 1.18

**Escopo:** Todo o repositório

## 1. Autoridade

O Arquiteto humano tem autoridade final sobre intenção, prioridade, escopo,
arquitetura, risco, autorização, validação e integração. A ordem recebida por
prompt ou pipeline identifica papel, especificação e recorte autorizado.

Agentes não inventam requisitos nem expandem o recorte. Evidências factuais
permanecem factuais mesmo quando o Arquiteto aceita o risco.

## 2. Fontes

- especificações definem comportamento e aceite;
- estas diretrizes definem regras locais;
- o mapa localiza fontes e lacunas;
- o changelog registra decisões, lacunas, evidências e resultados;
- código e testes implementam e evidenciam;
- relatórios não criam requisitos.

Git registra commits, autoria, diferenças, branches e linhagem. Não duplique
esses dados manualmente nas fontes EKM.

Estas diretrizes adotam o modelo 1.18 do repositório de referência
`EKM-guidelines`. Regras específicas deste projeto prevalecem somente no
escopo declarado.

## 3. Fluxo

```text
Autor da Especificação
→ Engenheiro Analista
→ Engenheiro Implementador
→ Engenheiro Revisor / Tech Lead
→ decisão humana e integração
```

Implementação exige especificação Implementável [`Implementable`]. Precisa de
esclarecimento [`Needs Clarification`] retorna a decisão ao Arquiteto sem
alteração parcial da implementação.

Critérios obrigatórios devem permitir uma asserção objetiva do cenário, do
resultado observável e da evidência terminal. Compilação não substitui
execução; critério falho, não executado ou não verificável impede
`Implemented`.

Cada ator atualiza a especificação, promove os estados sustentados pela própria
etapa e entrega o resultado por commit e push. Não existe um ator separado
apenas para reconciliação.

O `AGENTS.md` seleciona as regras comuns e exatamente um perfil oficial da EKM.
O agente não carrega perfis de outros papéis nem a metodologia completa, salvo
ordem explícita de governança.

O Consultor de Arquitetura atua fora do pipeline, subordinado ao Arquiteto,
somente no recorte e nas operações autorizadas. Antes do commit final, registra
ordem, decisões, resultado e limitações e obtém confirmação explícita do
Arquiteto. Participação anterior impede alegação posterior de independência no
mesmo recorte.

## 4. Contrato Git

Todo fluxo começa em uma branch de trabalho derivada da `main`, nunca
diretamente na `main`. Toda tarefa de agente começa com árvore limpa, produz
resultado material, termina com commit e push e deixa a árvore limpa. Push com
falha significa etapa não entregue.

Antes de promover estado, declarar validação aprovada, criar o commit final,
realizar push ou emitir resposta conclusiva, o agente confirma que toda tarefa,
comando, processo, build, teste, upload ou execução delegada que iniciou chegou
a estado terminal e registra seu resultado ou limitação. Estado não terminal ou
desconhecido bloqueia o encerramento.

A tarefa não autoriza force push, reescrita de histórico, merge, tag, release ou
deploy sem ordem específica.

## 5. Preservação

- Não remover ou enfraquecer decisão vigente silenciosamente.
- Não substituir fonte normativa por resumo incompleto.
- Não resolver conflito normativo por preferência do agente.
- Preservar arquitetura, organização e separação de responsabilidades e usar o
  precedente equivalente mais próximo.
- Não criar camada, pasta estrutural, abstração transversal ou padrão
  arquitetural sem especificação Implementável que identifique o padrão atual,
  a mudança, o alcance e a justificativa ou decisão do Arquiteto.
- Na ausência ou conflito de precedentes, devolver a decisão ao Arquiteto.
- Atualizar conhecimento afetado na mesma mudança.
- Registrar lacunas que precisem sobreviver à tarefa.
- Preservar alterações preexistentes e não relacionadas.

## 6. Transações

Mudanças usam `EKM-CHG-NNNN`; lacunas usam `EKM-GAP-NNNN`.

Uma transação registra somente objetivo, decisões, lacunas, evidências materiais
e resultado. Ela é concluída quando o resultado aceito foi integrado ou, em
trabalho documental, entregue; o conhecimento está atual; e lacunas restantes
estão explícitas.

## 7. Regras específicas do projeto

- `main` é a referência de produção.
- O runtime suportado é Arduino sobre ESP32.
- ESP-IDF permanece preparação futura e não pode degradar o runtime vigente.
- ESP8266 não constitui plataforma suportada.
- APIs públicas e comportamentos normativos devem ser preservados, salvo
  mudança aprovada em especificação.
- Configuração de capabilities termina antes de `SmartSysApp::setup()`.
- O limite intencional é de oito capabilities por aplicação.
- Mudanças funcionais relevantes seguem *specification on touch*.
- Não altere `private.ini`, credenciais, secrets ou configuração privada.
- Não existe garantia automatizada EKM vigente.

## 8. Validações canônicas

Use, quando aplicáveis ao recorte:

- `git diff --check`;
- `pio run -e esp32_dev`;
- `pio test -e esp32s3_test` para testes PlatformIO/Unity.

Validações adicionais pertencem à especificação. Upload em hardware, publicação,
release e deploy exigem ordem explícita do Arquiteto. Resultado não executado,
pendente ou desconhecido não constitui evidência aprovada.
