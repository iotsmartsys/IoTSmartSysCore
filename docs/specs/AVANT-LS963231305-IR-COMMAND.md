# Especificação — Interpreter IR AVANT-LS963231305

**ID:** `IOTSSC-AVANT-LS963231305-IR-COMMAND`

**Classe da fonte:** Normativa

**Versão:** 0.1

**Estado normativo:** Vigente [`Active`]

**Estado da implementação:** Validada [`Validated`]

**Estado da entrega:** Concluída [`Done`]

**Revisão de implementabilidade:** Pronta [`Ready`] conforme
[relatório da versão 0.1](../reports/2026-10-21T024132Z-0.1-b8cf1b81-897a-41ec-b295-249231fcb0b3-implementability-analysis.md)

**Registro da implementação:** [relatório da versão 0.1](../reports/2026-10-21T130151Z-0.1-avantimpl01-implementation-report.md)

**Decisão de encerramento:** Marcelo Miranda, Arquiteto, em 21/10/2026:
validação em hardware declarada e ordem explícita de integração à `main` e
encerramento. [Registro separado](../reports/2026-10-21T151555Z-0.1-avantclose01-finalization-report.md).

**Última atualização:** 21/10/2026

**Branch:** `spec/avant-ls963231305-ir-command`

## 1. Objetivo e contexto

Adicionar um interpreter específico para o controle AVANT-LS963231305, capaz
de converter os oito códigos fornecidos pelo Arquiteto em comandos textuais.
Os sinais informados foram identificados como NEC de 32 bits. O interpreter
reconhece exclusivamente o valor de `IRCommand::code`, independentemente de
`type`, conforme decisão confirmada no rascunho.

O objetivo é interpretar teclas, não controlar um aparelho. `Mode`, `Auto` e
as quatro durações são identificadores textuais, sem execução de modos,
automação ou temporizadores.

## 2. Autoridades e relações

- **New:** interpreter, identificação do modelo e vocabulário AVANT definido
  nesta especificação. Não substitui nem amplia o mapeamento de outro modelo.
- **Preserves:** [PUBLIC-API-COMPATIBILITY.md](PUBLIC-API-COMPATIBILITY.md),
  especialmente API-001, API-002 e API-008; a extensão é aditiva, sem mudança
  de assinaturas, valores anteriores do enum, defaults ou consumidores.
- **Preserves:** [CORE-RUNTIME-LIFECYCLE.md](CORE-RUNTIME-LIFECYCLE.md);
  nenhuma mudança em configuração, ownership, capabilities ou processamento
  cooperativo.
- **Preserves:** [AIR-CONDITIONER-PHILCO-EXAMPLE.md](AIR-CONDITIONER-PHILCO-EXAMPLE.md);
  a proposta Philco permanece independente, sem mudança em seu interpreter,
  capability ou exemplo.

Compartilhar a interface IR não cria emenda ao contrato de outro modelo.
Não há mudança arquitetural transversal ou ADR nova proposta neste recorte.

## 3. Escopo e arquitetura

Inclui interpreter público concreto, constantes públicas dos oito comandos,
novo membro ao final de `IRControlModel` e documentação de consumo explícito.
O modelo não passa a ser default nem é selecionado automaticamente por código,
protocolo ou detecção de dispositivo. Não é contratado factory novo.

**Precedente aplicável:**
[IoTSM1541ControlInterpreter.h](../../src/Platform/Arduino/Interpreters/IoTSM1541ControlInterpreter.h)
e sua implementação, na camada Arduino, implementando
[IRCommandInterpreter](../../src/Contracts/Interpreters/IRCommandInterpreter.h).

**Elementos preservados:** separação Contracts/Platform, contrato
[IRCommand](../../src/Contracts/Sensors/IIRCommandSensor.h), sensor Arduino IR,
processor, interpreters existentes e responsabilidade da aplicação por consumir
o resultado. O interpreter não possui o sensor nem conduz seu lifecycle.

**Desvio arquitetural explícito:** nenhum.

**Capacidades arquiteturais pressupostas:** interface de interpretação e
recepção IR já existentes. A interpretação de um comando fornecido pelo
consumidor não depende de receptor físico.

**Preparação arquitetural separada:** nenhuma; consultar a análise formal
referenciada nos metadados.

### 3.1 Fora de escopo

- alterar [src/main.cpp](../../src/main.cpp), exemplos executáveis, runner,
  pinout, environments ou seleção padrão da aplicação;
- criar capabilities ou associar teclas a relés, luzes ou outros atuadores;
- transmitir IR, executar temporizadores, persistir estado ou publicar eventos;
- alterar `IRCommand`, adicionar quantidade de bits ou validar `type`;
- modificar tratamento de repetição NEC, consumo de `triggered` ou sensor;
- corrigir ou alterar modelos anteriores e suas correspondências;
- criar, ampliar, reestruturar, corrigir ou executar testes;
- operar hardware, fazer upload, release, deploy ou integração à `main`;
- alterar configuração privada, credenciais ou migrar a governança EKOM;
- ampliar suporte para ESP8266 ou ESP-IDF nativo.

## 4. Requisitos

- **AVIR-001:** disponibilizar um interpreter público concreto para
  AVANT-LS963231305, na camada Arduino, derivado de `IRCommandInterpreter`,
  consumível pela assinatura vigente
  `std::string interpret(const IRCommand &command)`.
- **AVIR-002:** para cada código da tabela da seção 5, devolver exatamente o
  texto correspondente, respeitando capitalização e ausência de espaços.
- **AVIR-003:** devolver exatamente `"unknown"` para qualquer outro valor de
  `code`, sem exceção, ação sobre hardware ou substituição por comando válido.
  Esse retorno é o resultado observável de não reconhecimento, conforme o
  precedente; não é uma confirmação de execução.
- **AVIR-004:** determinar o resultado somente pelo valor integral de `code`,
  sem truncamento, máscara, conversão de ordem dos bits, filtro de `type` ou
  condição sobre `triggered`.
- **AVIR-005:** não modificar o comando recebido, marcar leitura, reter estado
  de teclas anteriores ou executar efeitos funcionais. Chamadas repetidas com
  o mesmo código devem produzir o mesmo texto.
- **AVIR-006:** acrescentar um membro identificando AVANT-LS963231305 ao final
  de `IRControlModel`, preservando nomes, ordem e valores numéricos anteriores.
  O acréscimo não contrata criação automática de interpreter.
- **AVIR-007:** expor constantes públicas dos oito retornos, seguindo o padrão
  de constantes do interpreter IoTSM1541. Cada constante deve coincidir com o
  texto retornado para sua tecla.
- **AVIR-008:** preservar fontes e comportamento dos interpreters anteriores,
  sensor, processor e aplicação existente. Códigos compartilhados entre
  modelos não podem alterar o resultado produzido pelo modelo já selecionado.
- **AVIR-009:** documentar inclusão do header, construção explícita, chamada a
  `interpret`, uso das constantes, tabela de códigos, retorno desconhecido e
  limites de protocolo e repetição. O exemplo documental não altera aplicação
  executável nem requer credenciais ou hardware.

Nomes C++ do novo tipo, membro e constantes devem seguir o precedente local;
a implementação os documenta sem introduzir aliases ou configuração extra.

## 5. Mapeamento e contrato de consumo

| Tecla | Protocolo informado | Bits informados | `code` | Retorno exato |
|---|---|---|---|---|
| ON | NEC | 32 | `0xFF22DD` | `"On"` |
| OFF | NEC | 32 | `0xFFC23D` | `"Off"` |
| MODE | NEC | 32 | `0xFFE01F` | `"Mode"` |
| AUTO | NEC | 32 | `0xFF906F` | `"Auto"` |
| 30MIN | NEC | 32 | `0xFF6897` | `"30Min"` |
| 2H | NEC | 32 | `0xFFB04F` | `"2H"` |
| 5H | NEC | 32 | `0xFF10EF` | `"5H"` |
| 10H | NEC | 32 | `0xFF5AA5` | `"10H"` |

Os zeros à esquerda não alteram o valor numérico: `0x00FF22DD` e `0xFF22DD`
representam o mesmo código. `IRCommand` não fornece quantidade de bits;
protocolo e largura acima documentam a captura, não condições de aceitação.

Fluxo: consumidor escolhe o interpreter AVANT → fornece `IRCommand` →
recebe texto → decide como utilizá-lo. O processamento de `triggered` e a
marcação de leitura permanecem responsabilidade dos componentes existentes e
da aplicação, fora do interpreter.

## 6. Falhas e condições de borda

- Código conhecido com `type` vazio, `"NEC"` ou outro texto: mesmo retorno.
- Código conhecido com `triggered` falso ou verdadeiro: mesmo retorno.
- `0`, código não listado ou `0xFFFFFFFFFFFFFFFFULL`: `"unknown"`.
- Valor com bits superiores adicionais, mesmo que os bits inferiores coincidam
  com código conhecido: `"unknown"`.
- Repetição NEC bruta não é convertida pelo interpreter. O sensor continua
  responsável pelo tratamento vigente antes de entregar um comando.
- Ausência de chamada não produz evento nem altera estado. Interpretações
  sucessivas não dependem de chamada anterior, inclusive após `"unknown"`.

## 7. Critérios de aceite e validações

| Critério | Requisitos | Cenário e resultado esperado | Evidência planejada |
|---|---|---|---|
| AVIR-AC-001 | AVIR-001, AVIR-006, AVIR-007 | Incluir o novo tipo público e confrontar interface, enum e constantes: assinatura compatível, oito constantes corretas e valores anteriores do enum preservados. | Inspeção dos headers e delta; build canônico da implementação com código de saída 0 e compilação do novo componente. |
| AVIR-AC-002 | AVIR-002, AVIR-004 | Confrontar cada uma das oito entradas com a tabela, variando conceitualmente `type` e `triggered`: oito retornos exatos, sem filtro desses campos ou transformação de `code`. | Inspeção individual dos oito caminhos e das condições de entrada. |
| AVIR-AC-003 | AVIR-003, AVIR-004, AVIR-005 | Confrontar `0`, `0x123456`, `0xFFFFFFFFFFFFFFFFULL` e `0x100FF22DD`, além de chamadas sucessivas conhecidas/desconhecidas: desconhecidos retornam `"unknown"`, sem truncamento, mutação ou estado entre chamadas. | Inspeção do caminho padrão e de todo o corpo do interpreter. |
| AVIR-AC-004 | AVIR-005, AVIR-008 | Comparar a entrega com a baseline: aplicação, sensor, processor e modelos anteriores preservados; `0xFFE01F` e `0xFF906F` não mudam de significado no IoTSM1541. | Inspeção do delta e confronto com o precedente; nenhuma mudança funcional fora do novo modelo. |
| AVIR-AC-005 | AVIR-009 | Ler a documentação e confrontar seus símbolos com os headers: consumo explícito, tabela completa, constantes e bordas documentados, sem prometer ações ou validação de protocolo. | Inspeção da documentação de uso e referências. |

**Artefatos de teste no recorte:** nenhum. Os critérios acima contratam
inspeção estática; não exigem criar ou executar suíte, harness ou firmware de
ensaio. A suficiência dessa evidência permanece decisão do Arquiteto.

O build canônico da futura implementação é `pio run -e esp32_dev`. Build
aprovado não comprova comportamento em execução ou recepção física. Falha
ou ausência de build não pode ser apresentada como sucesso. Inspeção parcial
não aprova um critério completo; evidência ausente permanece não verificada.

Na autoria documental aplicam-se `git diff --check` e a guarda estrutural EKOM
aos documentos alterados. Testes, captura real, upload e validação em hardware
permanecem `Not Executed`, dependentes de ordem operacional própria.

## 8. Conhecimento afetado

- [KNOWLEDGE-MAP.md](../rfc/KNOWLEDGE-MAP.md): indexar a autoridade do novo
  modelo e sua relação de interpretação sem atuação sobre dispositivos.
- [EKOM-CHANGELOG.md](../rfc/EKOM-CHANGELOG.md): registrar autoria e estado.
- Na implementação, acrescentar documentação de consumo junto à documentação
  técnica do projeto, com referência a esta especificação.
- Preservar especificações anteriores, histórico de relatórios e
  [src/main.cpp](../../src/main.cpp).

## 9. Decisões, pendências e regras de construção

O Arquiteto confirmou retorno no estilo `On`/`Off`, reconhecimento somente por
código e escopo limitado a interpreter, enum e documentação. O rascunho
consolidado também delimita ausência de efeitos, constantes públicas e
preservação dos componentes existentes. Em 20/10/2026, após manter o rascunho
provisório, ordenou o registro desta especificação.

Não há decisão funcional pendente identificada na autoria. Análise,
implementação e revisão produzem registros separados. O Arquiteto declarou
validação em hardware e determinou o encerramento em 21/10/2026; os estados
`Active`, `Validated` e `Done` refletem essa decisão, não uma aprovação
autônoma do agente ou alegação de revisão independente.

**Contrato de engenharia e qualificação:** o
[contrato 0.1 aprovado](../rfc/REPOSITORY-ENGINEERING-CONTRACT.md) e o
[recorte Ready e habilitado](../rfc/REPOSITORY-READINESS.md) suprem a condição
ausente na autoria. Decisões `AVIR-ENG-DEC-001` e `AVIR-ENG-DEC-002`,
transação `EKOM-CHG-0027`. Não há dispensa ou migração global de governança.
Em 21/10/2026, o Arquiteto ordenou implementar esta versão já analisada e
qualificada, sem alteração de seus requisitos ou critérios.

**Regras e precedentes:** [AGENTS.md](../../AGENTS.md),
[diretrizes locais](../rfc/EKOM-GUIDELINES.md), autoridades da seção 2 e
precedente da seção 3. Nenhuma exceção arquitetural é solicitada.

**Lacunas e débitos relacionados:** `EKM-GAP-0001` continua como limitação
preexistente de evidência de compatibilidade pública; esta extensão não a
encerra. Nenhum débito técnico novo é aceito por esta especificação.
