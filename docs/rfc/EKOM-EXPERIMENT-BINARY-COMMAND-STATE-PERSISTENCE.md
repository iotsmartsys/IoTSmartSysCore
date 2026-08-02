# Experimento EKOM — Persistência de comandos binários

**ID:** IOTSSC-EKOM-BINARY-COMMAND-STATE-001

**Estado do registro:** Pendente de confirmação do Arquiteto

**Modelo usado na classificação:** EKOM 2.1

**Métrica:** Avaliação experimental de adequação dos atores, versão 0.1

**Especificação relacionada:** `IOTSSC-BINARY-COMMAND-STATE@0.6`

## 1. Objetivo

Revisar o ciclo experimental que levou a persistência de comandos binários de
uma proposta inicial até `Active` / `Validated` / `Ready for Integration`, e
classificar as execuções observáveis por perfil executor e papel EKOM.

A classificação não avalia universalmente os modelos. A unidade considerada é
a combinação de modelo, ambiente agente, configuração/instruções, versão do
método e papel. As notas são calibração histórica pela métrica EKOM 2.1; não
transformam regras posteriores em violações retroativas.

## 2. Participantes declarados pelo Arquiteto

- Codex, GPT 5.6 Sol: Autor da Especificação, Engenheiro Analista, Engenheiro
  Implementador, Consultor de Arquitetura e Tech Lead;
- Claude Code, Sonnet 5 e Opus 5: Engenheiro Analista, Engenheiro
  Implementador, Tech Lead e Revisor;
- Copilot, Grok 4.5: Autor da Especificação;
- Arquiteto humano: intenção, decisões, autorização, validação física e
  aprovação final.

O histórico Git identifica explicitamente coautorias `Claude Sonnet 5`,
`Claude Opus 5` e, nas duas promoções finais, `Claude Fable 5`. Como `Fable 5`
não integra a relação inicial fornecida pelo Arquiteto, essas duas execuções
são mantidas separadas e a atribuição não é silenciosamente fundida com Sonnet
ou Opus. A atribuição das atuações Codex e Copilot decorre da declaração do
Arquiteto e do encadeamento documental, pois nem todos os commits carregam
metadado equivalente de coautoria.

## 3. Trajetória observada

1. A versão inicial especificou e implementou a persistência, mas seus
   critérios permitiram falso sucesso: caminho do LED não alcançado, double da
   valve semanticamente incompatível, corrupção e falhas NVS insuficientemente
   observáveis e compilação com zero casos usada como suporte de promoção.
2. A avaliação consultiva separou resultado funcional de conformidade do
   executor e originou a regra de critérios assertáveis, depois
   operacionalizada na EKM 1.19.
3. As versões seguintes ampliaram os oráculos, descobriram duplicação do grafo
   de serviços no provisioning, ausência de limites públicos de identidade e
   mutabilidade pós-registro. O Arquiteto decidiu limites 63/31, imutabilidade,
   semântica dos métodos obsoletos, política de `blink`, writer assíncrono,
   gate `esp32_dev` e quarentena das suítes existentes.
4. A implementação 0.6 entregou a arquitetura principal e preservou o estado
   `In Progress`, mas a revisão encontrou duas falhas funcionais altas: erros
   NVS classificados como ausência e comando explícito incapaz de substituir
   `blink`. Um terceiro achado sobre oráculos do writer foi corretamente
   mantido como dívida `Deferred` após a quarentena.
5. As duas falhas foram corrigidas, o baseline `esp32_dev` foi tratado em
   entrega separada, a revisão estática confirmou o gate e o Arquiteto validou
   o firmware em hardware. A entrega alcançou `Active` / `Validated` /
   `Ready for Integration`, sem alegar integração à `main`.

## 4. Classificação das execuções

As notas abaixo usam as cinco dimensões da métrica, na ordem
`autoridade/correção/evidências/conhecimento/Git`. Episódios agrupam somente
atuações coerentes do mesmo perfil e papel para as quais há evidência
suficiente. Quando a atribuição exata entre commits não é demonstrável, a nota
é deliberadamente conservadora.

| Perfil executor e papel | Recorte avaliado | Dimensões | Total | Eliminatório | Classificação |
|---|---|---:|---:|---|---|
| Copilot + Grok 4.5 — Autor | especificação inicial 0.1 | 18/11/9/16/15 | 69 | Não | Não aceitável [`Not Acceptable`] |
| Claude Code + Sonnet 5 — Implementador | implementação inicial 0.1 | 18/8/8/11/13 | 58 | Sim, pela métrica atual: zero casos e oráculos incompatíveis sustentaram `Implemented` | Reprovada [`Failed`] |
| Codex + GPT 5.6 Sol — Consultor/Tech Lead | diagnóstico dos falsos sucessos e contestação da linha 0.3 | 19/20/24/18/13 | 94 | Não | Conforme [`Conformant`] |
| Codex + GPT 5.6 Sol — Autor | critérios assertáveis e reconciliação normativa 0.2/0.4–0.6 | 20/19/24/19/14 | 96 | Não | Conforme [`Conformant`] |
| Claude Code + Sonnet 5 — Analista | análise da versão 0.2 | 19/17/21/15/14 | 86 | Não | Aceitável [`Acceptable`] |
| Claude Code + Sonnet 5 — Implementador | correção parcial da versão 0.2, mantida `In Progress` | 19/13/17/16/14 | 79 | Não | Supervisionada [`Supervised`] |
| Claude Code + Sonnet 5 — Analista | análise da versão 0.3 depois contestada | 18/10/13/13/14 | 68 | Não | Não aceitável [`Not Acceptable`] |
| Codex + GPT 5.6 Sol — Analista | esclarecimentos de identidade e revisão 0.5/0.6 | 20/19/23/19/14 | 95 | Não | Conforme [`Conformant`] |
| Claude Code + Opus 5 — Implementador | implementação integral 0.6 antes da revisão | 20/14/17/19/14 | 84 | Não | Aceitável [`Acceptable`] |
| Claude Code + modelo não isolado (Sonnet/Opus declarados) — Tech Lead/Revisor | revisão 0.6 e repetição sob quarentena | 20/19/23/18/14 | 94 | Não | Conforme [`Conformant`] |
| Codex + GPT 5.6 Sol — Implementador | BCS-REV-001/002 e baseline separado | 20/20/24/18/14 | 96 | Não | Conforme [`Conformant`] |
| Claude Code + Fable 5 — Revisor | confirmação estática e promoção após validação humana | 20/18/23/15/14 | 90 | Não | Conforme [`Conformant`] |

### 4.1 Fundamentação dos descontos materiais

- A autoria inicial delimitou o objetivo e produziu fonte versionada, mas não
  forneceu oráculos capazes de reprovar implementações plausivelmente erradas.
- A primeira implementação produziu resultado técnico útil, porém confundiu
  presença/compilação de testes com execução comportamental e usou doubles que
  não preservavam a integração real. Pela métrica atual isso é eliminatório;
  historicamente, o caso revelou a lacuna que motivou EKM 1.18/1.19.
- A análise Claude da linha 0.3 tratou a correção do singleton como suficiente,
  apesar de `-fno-threadsafe-statics`, do retorno ignorado de `save()`, do erase
  global e dos demais riscos depois registrados. O resultado
  `Implementable` foi contestado e não pôde ser reutilizado.
- A implementação Opus 0.6 foi ampla e manteve honestamente `In Progress`, mas
  deixou BCS-REV-001/002 no código e não implementou integralmente o oráculo do
  writer. Por não promover falso sucesso, os defeitos reduzem correção e
  evidência sem formar eliminatório.
- A revisão Claude foi materialmente valiosa ao encontrar falhas que a
  implementação e seus testes não revelaram. A quarentena posterior não
  invalida a revisão: ela apenas retirou suítes imaturas do gate atual.
- A promoção final deixou referências residuais no mapa de conhecimento e uma
  frase histórica ambígua na especificação, corrigidas em conferência
  posterior. O desconto pertence a estados e conhecimento, não à validação
  funcional recebida do Arquiteto.

## 5. Qualificação dos perfis

Nenhum perfil alcança Aceito [`Accepted`]. A métrica exige ao menos três
execuções, dois contextos distintos, média mínima de 85, nenhuma nota abaixo de
75 e nenhum eliminatório. Este experimento fornece essencialmente uma única
especificação, um repositório e uma família de risco.

- Codex GPT 5.6 Sol apresentou execuções conformes em vários papéis, mas as
  amostras não são independentes nem atravessam dois contextos nesta avaliação;
  permanece Candidato [`Candidate`] para cada papel observado.
- Claude Code + Sonnet 5 apresentou forte capacidade de revisão, mas variação
  material em análise e implementação, incluindo uma calibração eliminatória;
  a recomendação é uso Supervisionado [`Supervised`] nesses papéis.
- Claude Code + Opus 5 possui uma execução aceitável como Implementador;
  permanece Candidato [`Candidate`] e requer revisão proporcional.
- Claude Code + Fable 5 possui uma execução conforme como Revisor, mas não foi
  declarado inicialmente como participante e permanece Candidato
  [`Candidate`] até confirmação de identidade e ampliação da amostra.
- Copilot + Grok 4.5 possui uma execução não aceitável como Autor neste risco;
  permanece Candidato [`Candidate`] e não deve atuar autonomamente no papel com
  a configuração observada.

Essas recomendações são propostas do Consultor. A decisão humana sobre uso e
qualificação pertence ao Arquiteto.

## 6. Balanço do experimento

### Evidências favoráveis

- A especificação, os estados e o Git sustentaram continuidade entre três
  ambientes e diferentes modelos sem depender de conversa compartilhada.
- A separação de papéis localizou defeitos reais: autoria insuficiente,
  implementação com falso sucesso, análise arquitetural contestável e duas
  falhas funcionais altas na implementação final.
- As decisões reservadas permaneceram humanas e foram incorporadas antes da
  implementação correspondente.
- Falhas de build, ausência de hardware e testes não executados foram
  registrados como limitações em vez de convertidos silenciosamente em
  aprovação.
- A validação física humana fechou uma lacuna que build e inspeção estática não
  poderiam substituir.

### Custos e fragilidades

- O ciclo exigiu seis versões normativas e muitas reconciliações até estabilizar
  um único recorte funcional.
- O documento normativo acumulou extensa narrativa histórica; isso melhorou a
  auditoria do experimento, mas aumentou custo cognitivo e risco de referências
  residuais.
- A maturidade insuficiente das suítes impediu testar a hipótese central de que
  critérios mais assertáveis melhorariam também a evidência automatizada.
- A troca de modelos aumentou diversidade crítica, mas a atribuição de cada
  execução não foi uniformemente registrada; `Fable 5` evidencia a lacuna.
- O mesmo Codex participou de autoria, análise, implementação, consultoria e
  Tech Lead. As contribuições são úteis, mas não constituem Gate independente
  sobre o próprio trabalho.

## 7. Hipóteses classificadas

| Hipótese | Resultado experimental |
|---|---|
| A especificação pode orquestrar agentes heterogêneos | Sustentada neste contexto |
| Separação de papéis reduz falso sucesso | Fortemente sustentada |
| Critérios assertáveis evitam os mesmos desvios | Parcialmente sustentada; a especificação melhorou, mas as suítes foram colocadas em quarentena |
| Resultado funcional implica conformidade EKOM | Refutada |
| Nome do modelo basta para prever adequação | Refutada; ambiente, adaptador, papel e fase alteraram os resultados |
| Um único experimento qualifica um perfil | Refutada pela própria métrica |
| Mais documentação sempre melhora a entrega | Não sustentada; houve ganho de auditabilidade e custo cognitivo material |

## 8. Recomendações do Consultor

1. Manter a avaliação por perfil executor e papel, nunca por fornecedor ou
   modelo isolado.
2. Registrar de forma uniforme, no relatório experimental, modelo, ambiente,
   adaptador de instruções, versão EKOM e papel; não copiar esses dados para a
   especificação funcional.
3. Reservar ao menos uma revisão final para perfil que não tenha participado da
   autoria ou implementação do mesmo recorte.
4. Em novos experimentos, manter a especificação normativa curta e transferir
   narrativa histórica e pontuação para relatório experimental relacionado.
5. Reativar testes somente por estratégia explícita; quando isso ocorrer,
   usar BCS-REV-003 e BCS-AC-028 como primeiro teste da eficácia dos oráculos
   assertáveis.
6. Repetir os perfis promissores em ao menos outro repositório ou família de
   risco antes de qualquer qualificação `Accepted`.
7. Tratar esta pontuação como calibração de pair: o Consultor participou de
   partes do ciclo e não oferece independência para validar as próprias notas.

## 9. Resultado proposto

O experimento é classificado como **funcionalmente validado e processualmente
informativo**, com evidência forte de que a orquestração por especificação e a
separação de papéis encontram desvios que execuções isoladas não detectaram.
Não existe classificação canônica EKOM para o experimento como conjunto; as
classificações formais pertencem às execuções da seção 4.

O registro não promove, reabre ou altera os estados da especificação
funcional. Não declara nenhum perfil `Accepted`, não integra à `main` e não
autoriza mudança no método EKOM.

## 10. Registro da atuação do Consultor

**Estado da confirmação final:** Pendente.

- **Papel exercido:** Consultor de Arquitetura.
- **Ordem autorizada:** revisar o experimento multiagente e classificá-lo pela
  EKOM 2.1.
- **Repositório e recorte:** `IoTSmartSysCore`; trajetória integral de
  `IOTSSC-BINARY-COMMAND-STATE@0.6`, fontes EKOM relacionadas e histórico Git.
- **Operações autorizadas:** investigação e retrospectiva documental, sem
  código, teste, build, merge, release ou deploy.
- **Decisões confirmadas:** participantes e papéis declarados pelo Arquiteto;
  testes permanecem em quarentena; BCS-REV-003 permanece `Deferred`; a
  retrospectiva não reabre nem revalida a especificação.
- **Resultado material preparado:** este relatório, a classificação
  experimental por execução e as recomendações da seção 8.
- **Validações e limitações:** fontes e histórico confrontados; pontuação não
  independente, atribuição `Fable 5` divergente da relação inicial, versões do
  método evoluíram durante o ciclo, uma única especificação/contexto e nenhuma
  validação técnica reexecutada nesta atuação.
- **Significado solicitado para a confirmação final:** confirmar que o
  registro representa o balanço e a classificação pretendidos e autorizar sua
  marcação como confirmado, o commit e o push. A confirmação não qualifica
  nenhum perfil como `Accepted`, não altera a especificação funcional e não
  declara integração, release ou deploy.
