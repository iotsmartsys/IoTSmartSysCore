# Especificação — Estado físico do controle de garagem

**ID:** IOTSSC-GARAGE-CONTROL

**Classe da fonte:** Normativa

**Versão:** 0.1

**Estado normativo:** Proposta [`Proposed`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Não pronta [`Not Ready`]

**Revisão de implementabilidade:** Implementável [`Implementable`]

**Relação normativa:** Nova [`New`]

## 1. Objetivo e contexto

Corrigir a máquina de estados da `GarageControlCapability` para que os estados
publicados representem a posição física confirmada e o movimento observado, em
vez de manter indefinidamente a intenção de um comando.

Hoje, um comando `open` altera imediatamente o estado para `opening`. Enquanto
esse estado permanece ativo, uma leitura do sensor de fechamento é ignorada.
Assim, se o portão não partir, retornar ao fechamento ou apresentar oscilação no
sensor, a capability pode permanecer em `opening` mesmo com o fim de curso
fechado ativo.

## 2. Escopo

- leitura dos sensores de abertura e fechamento;
- debounce das entradas de fim de curso;
- transições entre `unknown`, `opened`, `closed`, `opening` e `closing`;
- efeito dos comandos `open` e `close` sobre essas transições;
- movimento iniciado por comando ou externamente;
- compatibilidade da configuração e da API pública existente;
- testes automatizados da máquina de estados e validação em hardware.

## 3. Fora de escopo

- alterar a sequência elétrica dos relés;
- alterar os comandos `lock`, `unlock`, `stop` e `stop_unlock`;
- criar telemetria ou estados novos de falha;
- detectar velocidade, obstrução ou sentido por sensores adicionais;
- adicionar timeout de percurso;
- alterar MQTT, persistência ou protocolo de comandos;
- mudar o limite de capabilities ou o ciclo de vida do `SmartSysApp`.

## 4. Requisitos

- **GAR-001:** os sensores de fim de curso devem continuar usando entrada
  `PULL_UP`, com nível `LOW` indicando extremo ativo.
- **GAR-002:** uma mudança de sensor somente deve alterar a máquina de estados
  após permanecer estável pelo intervalo configurado de debounce.
- **GAR-003:** `GarageControlConfig` deve oferecer
  `sensorDebounceTimeMs`, com default de 50 ms, sem alterar o significado de
  `debounceTimeMs`, que continua controlando a duração do pulso dos relés.
- **GAR-004:** com apenas o sensor de fechamento ativo e estável, o estado
  publicado deve ser `closed`.
- **GAR-005:** com apenas o sensor de abertura ativo e estável, o estado
  publicado deve ser `opened`.
- **GAR-006:** um comando `open` emitido enquanto o sensor de fechamento
  permanece ativo deve acionar o relé, mas deve manter `closed` até a liberação
  estável desse sensor.
- **GAR-007:** após a liberação estável do sensor de fechamento, com nenhum fim
  de curso ativo, o estado deve ser `opening`.
- **GAR-008:** um comando `close` emitido enquanto o sensor de abertura
  permanece ativo deve acionar o relé, mas deve manter `opened` até a liberação
  estável desse sensor.
- **GAR-009:** após a liberação estável do sensor de abertura, com nenhum fim de
  curso ativo, o estado deve ser `closing`.
- **GAR-010:** durante `opening`, a ativação estável do sensor de abertura deve
  produzir `opened`, e a reativação estável do sensor de fechamento deve
  produzir `closed`.
- **GAR-011:** durante `closing`, a ativação estável do sensor de fechamento
  deve produzir `closed`, e a reativação estável do sensor de abertura deve
  produzir `opened`.
- **GAR-012:** a liberação estável de um extremo conhecido deve permitir inferir
  movimento externo: saída de `closed` produz `opening` e saída de `opened`
  produz `closing`.
- **GAR-013:** um comando recebido com ambos os fins de curso inativos deve
  definir a direção solicitada: `open` produz `opening` e `close` produz
  `closing`.
- **GAR-014:** um comando de direção oposta durante o percurso deve atualizar a
  direção solicitada, sem impedir que um fim de curso ativo e estável determine
  o estado terminal.
- **GAR-015:** oscilações mais curtas que o debounce não podem publicar estados
  intermediários nem inverter a direção inferida.
- **GAR-016:** se ambos os sensores permanecerem ativos simultaneamente após o
  debounce, o estado deve ser `unknown`; nenhum extremo possui precedência.
- **GAR-017:** após inicialização, a capability deve publicar `closed` ou
  `opened` assim que a respectiva combinação permanecer estável pelo debounce;
  se ambos estiverem inativos ou contraditoriamente ativos, deve permanecer
  `unknown` até existir evidência suficiente.
- **GAR-018:** com sensores ausentes ou parciais, a capability deve usar os
  extremos disponíveis e os comandos recebidos, sem fabricar confirmação de um
  extremo não observável.
- **GAR-019:** a correção deve preservar nomes de comandos e estados, duração
  configurada dos pulsos, ownership, limite de capabilities e compatibilidade
  de código-fonte dos consumidores atuais.
- **GAR-020:** toda publicação deve continuar ocorrendo somente quando o estado
  lógico mudar; leituras estáveis repetidas não devem gerar eventos duplicados.

## 5. Fluxos, estados e contratos

### 5.1 Sensores configurados

| Sensor de abertura | Sensor de fechamento | Evidência física |
|---|---|---|
| Inativo | Ativo | `closed` |
| Ativo | Inativo | `opened` |
| Inativo | Inativo | percurso ou posição intermediária |
| Ativo | Ativo | combinação contraditória → `unknown` |

As combinações somente são consideradas após o debounce.

### 5.2 Comando a partir de um extremo

```text
closed
→ comando open e pulso no relé
→ permanece closed enquanto o fim de curso fechado estiver ativo
→ fim de curso fechado liberado de forma estável
→ opening
→ fim de curso aberto ativo de forma estável
→ opened
```

O fechamento é simétrico:

```text
opened → comando close → opened → closing → closed
```

### 5.3 Retorno ao extremo de origem

```text
closed → opening → closed
opened → closing → opened
```

O sensor terminal estável encerra a direção anterior. A intenção do comando não
pode manter `opening` ou `closing` contra a posição física confirmada.

### 5.4 Sensores ausentes

- sem sensores, `open` e `close` continuam publicando respectivamente
  `opening` e `closing`, sem confirmação automática de término;
- somente com sensor de abertura, `opened` pode ser confirmado, mas `closed`
  não pode ser fabricado;
- somente com sensor de fechamento, `closed` pode ser confirmado, mas `opened`
  não pode ser fabricado.

## 6. Falhas e condições de borda

- Se o relé de abertura for acionado e o portão não sair do fim de curso
  fechado, o estado permanece `closed`.
- Se o relé de fechamento for acionado e o portão não sair do fim de curso
  aberto, o estado permanece `opened`.
- Bounce inferior ao intervalo configurado não altera o estado lógico.
- Dois sensores ativos de forma estável são tratados como evidência
  contraditória e produzem `unknown`.
- Reinicialização no meio do percurso, sem extremo ativo, começa em `unknown`.
- Esta versão não cria estado de timeout ou falha de motor.

## 7. Critérios de aceite e validações

| Requisito | Evidência esperada |
|---|---|
| GAR-001 a GAR-003 | Inspeção de builder, config e adapters; teste do default e da separação entre os dois debounces |
| GAR-004 a GAR-011 | Testes automatizados das transições comandadas e dos retornos aos extremos |
| GAR-012 a GAR-014 | Testes de movimento externo, posição intermediária e reversão |
| GAR-015 | Teste com oscilações abaixo e acima de 50 ms |
| GAR-016 e GAR-017 | Testes de sensores contraditórios e inicialização em cada combinação |
| GAR-018 | Testes sem sensores e com somente um sensor |
| GAR-019 | Build Arduino/ESP32 e inspeção da API pública |
| GAR-020 | Teste do número e da ordem dos eventos publicados |

Também são obrigatórios:

- `git diff --check`;
- build do environment `esp32_dev`;
- teste em hardware com abertura, fechamento, falha de partida e retorno ao
  extremo de origem.

O build e os testes automatizados permitem declarar `Implemented`. A validação
em hardware é necessária para declarar `Validated`.

## 8. Conhecimento afetado

- `src/Core/Capabilities/GarageControlCapability.cpp`;
- `src/Contracts/Capabilities/GarageControlCapability.h`;
- `src/App/Builders/Configs/CapabilityConfig.h`;
- `src/App/Builders/Builders/CapabilitiesBuilder.cpp`;
- testes e mocks da `GarageControlCapability`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

## 9. Relações, decisões e lacunas

- `IOTSSC-PUBLIC-API`;
- `IOTSSC-RUNTIME`;
- `EKM-CHG-0007`;
- `EKM-GAP-0009`.

A proposta preserva a interface existente e adiciona somente uma configuração
com default compatível. A validação física permanece uma lacuna até execução no
hardware real.

## 10. Revisão de implementabilidade

**Resultado:** Implementável [`Implementable`]

**Resumo da análise:** os requisitos GAR-001 a GAR-020 podem ser implementados
sem decisão normativa, de produto ou arquitetura adicional. A correção pode
manter a API existente, adicionar a configuração com default compatível,
separar a intenção do comando do estado físico e validar o debounce com o
provedor de tempo já disponível na base.

**Decisões ausentes:** nenhuma.

**Evidências consultadas:**

- `GarageControlCapability` concentra leitura, inferência, publicação e comandos
  nos métodos `handleSensorState()`, `open()`, `close()` e `handle()`;
- `GarageControlConfig` herda os defaults públicos de `InputHardwareConfig` e
  admite a adição de `sensorDebounceTimeMs` sem invalidar inicializações
  existentes;
- `CapabilitiesBuilder` já centraliza a criação dos dois inputs com `PULL_UP` e
  a passagem de configuração para a capability;
- `IInputHardwareAdapter::readDigitalState()` fornece os níveis necessários sem
  alteração do contrato do adapter;
- `ICapability` disponibiliza `timeProvider`, permitindo debounce determinístico
  por tempo e substituição do provider em testes;
- `ICapabilityEventSink` e `value` permitem verificar ordem, conteúdo e
  duplicidade das publicações;
- a assinatura pública existente pode ser preservada por overload compatível ou
  argumento adicional com default;
- o environment `esp32s3_test` usa Unity e compila o código-fonte do projeto,
  permitindo adicionar mocks de input, output, tempo e event sink;
- `IOTSSC-PUBLIC-API` autoriza evolução compatível de configs e exige build e
  validação representativa;
- `IOTSSC-RUNTIME` exige apenas preservar configuração antes de `setup()`,
  processamento cooperativo em `handle()` e limite de oito capabilities;
- não existe outra especificação normativa da máquina de estados da garagem em
  conflito com este recorte;
- a validação física exigida permanece pendente para promoção a `Validated`,
  mas não impede produzir e comprovar o estado `Implemented`.

## 11. Evidências da implementação

Não aplicável nesta etapa. A implementação não foi iniciada.
