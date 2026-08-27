# Especificação — Leitura de corrente contínua fotovoltaica

**ID:** IOTSSC-CURRENT-SENSOR

**Classe da fonte:** Normativa

**Versão:** 0.2

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Não aplicável [`Not Applicable`]

**Revisão de implementabilidade:** Pendente [`Pending Review`]

**Relação normativa:** Nova [`New`]

## 1. Objetivo e contexto

Contratar a medição de corrente contínua entre o painel fotovoltaico e a entrada
do conversor buck no runtime do IoTSmartSysCore, usando o ACS712-30A.

O adapter possui aquisição, calibração, qualificação da leitura e monitoramento
opcional da alimentação; a capability publica um único envelope com valor e
estados.

São admitidos dois perfis iniciais:

- `ACS712_30A_5V`, cuja alimentação do ACS712 é suportada pelo fabricante;
- `ACS712_30A_3V3`, qualificado como `PROJECT_VALIDATED`, fora da faixa de
  alimentação oficialmente garantida para o ACS712 original e sujeito à
  validação integral do projeto.

## 2. Escopo

- Hardware Adapter de corrente contínua;
- perfis elétricos de 5 V com divisor e 3,3 V sem divisor;
- calibração de zero inicial e recalibração sob demanda;
- amostragem, média, filtro configurável, faixa morta e cadência;
- qualificação da medição, da faixa e da alimentação;
- capability e envelope único de publicação;
- API pública aditiva `SmartSysApp::addCurrentSensor()`;
- ownership do adapter e da capability pela aplicação;
- diagnóstico por `iotsmartsys::core::ILogger`;
- validação física separada para cada perfil.

## 3. Fora de escopo

- corrente alternada e valor eficaz (`RMS`);
- potência, energia acumulada, fator de potência e tensão do painel;
- persistência da calibração;
- comandos remotos de calibração por MQTT, HTTP ou provisionamento;
- proteção elétrica, corte de carga ou produção física obrigatória de 30 A
  durante a validação;
- alteração do limite de oito capabilities ou do ciclo cooperativo de
  `handle()`;
- suporte universal do ACS712 original à alimentação de 3,3 V.

## 4. Componentes e tipos públicos

| Componente | Local | Responsabilidade |
|---|---|---|
| `ICurrentSensor` | `src/Contracts/Sensors/` | Última medição estável, calibração e estados |
| `ACS712C30ACurrentSensor` | `src/Platform/Arduino/Sensors/` | Aquisição ADC, zero, conversão, filtragem, alimentação e limites |
| `CurrentSensorCapability` | `src/Contracts/Capabilities/` e `src/Core/Capabilities/` | Acionamento do adapter e publicação do envelope |
| Tipos de corrente | `src/Contracts/Sensors/` | Configuração, perfis, qualificações, estados e envelope |
| `CURRENT_SENSOR_TYPE` | `src/Contracts/Capabilities/ICapabilityType.h` | Tipo público da capability |
| Registro público | `src/App/Builders/` e `src/SmartSysApp.*` | Construção, ownership, identidade, slots e registro |

Somente o adapter conhece aquisição, calibração, conversão elétrica, filtragem,
alimentação e limites. A capability controla a cadência e não recalcula corrente.

```cpp
enum class CurrentMeasurementStatus {
    VALID,
    OUT_OF_CALIBRATED_RANGE,
    OVERCURRENT_OR_SATURATION
};

enum class CurrentSupplyStatus {
    IN_RANGE,
    SUPPLY_OUT_OF_RANGE,
    NOT_MONITORED
};

struct CurrentMeasurement {
    std::optional<float> currentA;
    CurrentMeasurementStatus measurementStatus;
    CurrentSupplyStatus supplyStatus;
};
```

A validade numérica final é:

```cpp
numericValueValid =
    measurementStatus == CurrentMeasurementStatus::VALID &&
    supplyStatus != CurrentSupplyStatus::SUPPLY_OUT_OF_RANGE;
```

## 5. Requisitos

### 5.1 Contrato do adapter

- **CUR-001:** `ICurrentSensor` deve derivar de `IHardwareAdapter` e preservar
  `setup()`, `handle()` e `lastStateReadMillis()`.
- **CUR-002:** `ICurrentSensor` deve devolver a última `CurrentMeasurement`
  estável sem executar aquisição.
- **CUR-003:** antes da primeira leitura concluída, `currentA` não possui valor
  e `lastStateReadMillis()` permanece inalterado.
- **CUR-004:** `handle()` deve ser cooperativo, sem espera indefinida, laço de
  espera por evento externo ou trabalho acima de uma amostra composta
  configurada.
- **CUR-005:** `lastStateReadMillis()` deve refletir a última leitura concluída
  com sucesso, inclusive quando seu estado impede valor numérico válido.

### 5.2 Configuração

- **CUR-006:** `CurrentSensorConfig` é um único contrato para ambos os perfis e
  deve contemplar, no mínimo:

```text
id
adcPin
adcResolutionBits
adcAttenuation
adcMaximumMv
supplyNominalMv
supplyValidMinimumMv
supplyValidMaximumMv
qualification
outputToAdcRatio
nominalZeroAdcMv
sensitivityAdcMvPerA
polarity
zeroCalibrationMode
startupWarmupMs
recalibrationSettleMs
zeroCalibrationSamples
maximumZeroDeviationMv
calibratedMinimumA
calibratedMaximumA
physicalMinimumA
physicalMaximumA
deadbandA
minimumReportableA
samplesPerReading
sampleIntervalUs
lowPassAlpha
readingIntervalMs
maximumAbsoluteErrorA
maximumRelativeErrorPercent
supplyMonitorAdcPin
supplyMonitorToVccRatio
```

- **CUR-007:** os defaults comuns são: resolução de 12 bits; atenuação abstrata
  `FULL_RANGE`; polaridade `+1,0`; modo `STARTUP_AND_ON_REQUEST`; aquecimento
  inicial `60000 ms`; acomodação de recalibração `2000 ms`; `2000` amostras de
  zero; `500` amostras por leitura; `lowPassAlpha = 1,0`; intervalo entre
  leituras `500 ms`; faixa morta e mínimo reportável `0,05 A`; faixa calibrada
  por magnitude de `0,50 A` a `15,00 A`; faixa física de `−30,00 A` a
  `+30,00 A`; erro absoluto `0,10 A`; erro relativo `5,0%`.
- **CUR-008:** `adcPin` e `id` não possuem default válido.
  `maximumZeroDeviationMv` é obrigatório por perfil, `adcMaximumMv` é
  obrigatório por target e `sampleIntervalUs` é obrigatório por
  perfil/target. Os três permanecem `TBD` nesta versão e não podem receber
  defaults universais silenciosos.
- **CUR-009:** `FULL_RANGE` é abstração do contrato. O adapter do target deve
  convertê-la para a atenuação específica da plataforma, sem presumir faixa
  útil idêntica em todos os ESP32.
- **CUR-010:** `adcMaximumMv` representa o limite efetivamente utilizável pelo
  ADC no target e na atenuação selecionada; não equivale automaticamente à
  alimentação nominal de 3,3 V.
- **CUR-011:** `lowPassAlpha = 1,0` desabilita filtragem exponencial adicional;
  inicialmente, a redução de ruído decorre somente da média composta.

### 5.3 Aquisição e calibração

- **CUR-012:** o adapter deve usar a conversão calibrada em milivolts oferecida
  pela plataforma, nunca a contagem bruta do ADC.
- **CUR-013:** a calibração inicial começa somente após `startupWarmupMs` com o
  sensor continuamente energizado, alimentação estável e corrente zero.
- **CUR-014:** cada recalibração posterior exige sensor continuamente
  energizado, alimentação estável, garantia externa de corrente zero e
  acomodação por `recalibrationSettleMs`.
- **CUR-015:** o zero medido deve ser mantido como estado de calibração separado
  de `nominalZeroAdcMv` e usado até nova calibração ou reinício.
- **CUR-016:** `requestZeroCalibration()` apenas agenda a recalibração para o
  próximo `handle()`; solicitação durante calibração em curso é descartada com
  WARN.
- **CUR-017:** uma amostra composta é a média aritmética das leituras calibradas
  em mV, espaçadas por `sampleIntervalUs`. A calibração usa
  `zeroCalibrationSamples`; a medição usa `samplesPerReading`.
- **CUR-018:** concluída a média, o filtro passa-baixa configurado por
  `lowPassAlpha` é aplicado antes da qualificação e publicação.

### 5.4 Cálculo elétrico

- **CUR-019:** o cálculo é independente da tensão de alimentação:

```text
I = polaridade ×
    (tensãoADC − zeroADC) /
    sensibilidadeEfetivaADC
```

- **CUR-020:** `zeroADC` e `sensitivityAdcMvPerA` referem-se sempre ao sinal
  após o divisor, quando existente.
- **CUR-021:** sensibilidades iniciais são valores de perfil configuráveis, não
  constantes universais do ACS712-30A, e devem poder ser substituídas pelo valor
  obtido na calibração física de cada unidade.
- **CUR-022:** se `|I| < deadbandA`, o valor produzido é exatamente zero.
- **CUR-023:** correntes negativas com magnitude superior a `0,10 A` devem
  preservar o sinal; aviso de corrente reversa é permitido, mas não obrigatório.

### 5.5 Faixa e estados

- **CUR-024:** para `0,50 A ≤ |I| ≤ 15,00 A`, a medição pode ser `VALID`
  quando as demais condições de validade forem satisfeitas.
- **CUR-025:** para `15,00 A < |I| ≤ 30,00 A`, o estado é
  `OUT_OF_CALIBRATED_RANGE` e `currentA` não possui valor publicável como
  medição válida.
- **CUR-026:** para `|I| > 30,00 A` ou saturação do ADC, o estado é
  `OVERCURRENT_OR_SATURATION` e `currentA` não possui valor publicável como
  medição válida.
- **CUR-027:** entre `0,05 A` e `0,50 A` em magnitude, o resultado pode ser
  apresentado apenas como estimativa, sem garantia de exatidão. A representação
  desse resultado permanece pendente na seção 13.
- **CUR-028:** a resolução apresentada deve ser `0,01 A` ou melhor; resolução
  não implica exatidão.
- **CUR-029:** com corrente constante, a variação pico a pico das leituras
  filtradas durante 30 segundos não pode ultrapassar `0,05 A`.
- **CUR-030:** após alteração da corrente, a leitura deve entrar na faixa de
  tolerância aplicável em até `1000 ms`.

### 5.6 Monitoramento da alimentação

- **CUR-031:** `SUPPLY_OUT_OF_RANGE` somente pode ser produzido a partir de
  medição independente da alimentação do ACS712.
- **CUR-032:** com `supplyMonitorAdcPin` ausente, o estado é `NOT_MONITORED`; a
  aplicação pode publicar valor numericamente válido, mas não pode afirmar que
  a alimentação foi verificada.
- **CUR-033:** com monitoramento presente, a tensão é obtida da leitura
  calibrada do ADC e de `supplyMonitorToVccRatio`. Valor fora do intervalo
  configurado produz `SUPPLY_OUT_OF_RANGE` e invalida `currentA`,
  independentemente do estado elétrico do sinal.
- **CUR-034:** `IN_RANGE` somente pode ser produzido quando a medição
  independente comprovar valor dentro dos limites inclusivos do perfil.

### 5.7 Capability e API pública

- **CUR-035:** `CurrentSensorCapability` deve derivar de `ICapability`, acionar
  o adapter em cada ciclo e avaliar a publicação no máximo a cada `1000 ms`,
  usando o provedor de tempo do runtime.
- **CUR-036:** `CURRENT_SENSOR_TYPE` possui o literal `"Current Sensor (A)"`.
- **CUR-037:** a identidade deve ser resolvida a partir de
  `CurrentSensorConfig.id` e `CURRENT_SENSOR_TYPE`; falha de identidade,
  conflito, falta de slot, configuração inválida ou falha de construção impede
  qualquer registro parcial.
- **CUR-038:** a API pública é:

```cpp
CurrentSensorCapability *
SmartSysApp::addCurrentSensor(CurrentSensorConfig config);
```

O ponteiro retornado é não proprietário, pertence à aplicação, permanece
estável durante a vida da aplicação e não pode ser liberado pelo consumidor.
`nullptr` indica falha; a causa detalhada deve ser registrada separadamente por
`ILogger`.

- **CUR-039:** `SmartSysApp` possui adapter e capability, copia ou mantém a
  configuração de forma segura e libera os recursos em seu encerramento.
- **CUR-040:** a aplicação deve impedir ou rejeitar identificadores e pinos
  conflitantes. O alcance de consumidores considerado conflito de pino
  permanece pendente na seção 13.
- **CUR-041:** o factory pode permanecer somente como detalhe interno e seam de
  injeção; a aplicação consumidora não precisa acessá-lo para registrar o sensor.
- **CUR-042:** a capability deve expor acesso não proprietário à última
  `CurrentMeasurement`, ao zero calibrado e à solicitação de recalibração.
- **CUR-043:** a publicação ocorre na primeira avaliação concluída e quando
  qualquer campo do envelope mudar. Leitura inválida nunca é publicada como
  corrente numérica válida.
- **CUR-044:** o envelope deve ser publicado como unidade indivisível. Sua
  representação no canal textual de `ICapability::updateState` permanece
  pendente na seção 13.
- **CUR-045:** o limite de oito capabilities, a configuração anterior a
  `SmartSysApp::setup()` e todas as assinaturas, defaults e semânticas públicas
  preexistentes permanecem preservados.

## 6. Perfis elétricos iniciais

### 6.1 `ACS712_30A_5V`

| Campo | Valor |
|---|---|
| Alimentação nominal | `5000 mV` |
| Faixa válida de alimentação | `4900–5100 mV`, inclusiva |
| Divisor na saída | `10 kΩ / 20 kΩ` |
| `outputToAdcRatio` | `0,666667` |
| `nominalZeroAdcMv` | `1666,7 mV` |
| `sensitivityAdcMvPerA` inicial | `43,05 mV/A` |
| `qualification` | `MANUFACTURER_SUPPORTED` |
| `maximumZeroDeviationMv` | `TBD` |
| `adcMaximumMv` | `TBD` por target |
| `sampleIntervalUs` | `TBD` por perfil/target |

`MANUFACTURER_SUPPORTED` qualifica somente a alimentação do ACS712. Módulo,
divisor, ADC, calibração e firmware continuam sujeitos à validação do projeto.

### 6.2 `ACS712_30A_3V3`

| Campo | Valor |
|---|---|
| Alimentação nominal | `3300 mV` |
| Faixa inicial de validação | `3200–3400 mV`, inclusiva |
| Divisor na saída | inexistente |
| `outputToAdcRatio` | `1,0` |
| `nominalZeroAdcMv` | `1650,0 mV` |
| `sensitivityAdcMvPerA` inicial | `43,56 mV/A` |
| `qualification` | `PROJECT_VALIDATED` |
| `maximumZeroDeviationMv` | `TBD` |
| `adcMaximumMv` | `TBD` por target |
| `sampleIntervalUs` | `TBD` por perfil/target |

A sensibilidade teórica inicial decorre de:

```text
66 mV/A × 3,3 V / 5,0 V = 43,56 mV/A
```

Este é o perfil principal das placas já fabricadas com fonte Hi-Link de 3,3 V.
Sua aceitação exige validação de exatidão, linearidade, estabilidade térmica,
zero, saturação e repetibilidade. O perfil de 5 V permanece alternativa de
bancada ou de futuras revisões de hardware.

## 7. Fluxo esperado

1. A aplicação preenche `CurrentSensorConfig` com um perfil e os valores
   específicos do target ainda sem default universal.
2. Antes de `SmartSysApp::setup()`, chama `app.addCurrentSensor(config)`.
3. A aplicação valida identidade, slots, configuração e conflitos, constrói e
   passa a possuir adapter e capability.
4. `setup()` configura o ADC e inicia o aquecimento de 60 segundos.
5. Após aquecimento com corrente zero e alimentação estável, o adapter calibra o
   zero e passa a produzir medições.
6. Cada `SmartSysApp::handle()` aciona a capability e o adapter; a capability
   publica o envelope em cadência de até `1000 ms`.
7. Recalibração solicitada pela capability começa no próximo `handle()`, após
   verificar suas pré-condições e cumprir acomodação de 2 segundos.

## 8. Diagnóstico

Todo diagnóstico usa `iotsmartsys::core::ILogger`, com tag do componente:

- **INFO:** perfil selecionado; início e fim de calibração; zero resultante;
- **DEBUG:** tensão média, zero, diferença, corrente, estados e alimentação
  medida quando disponível;
- **WARN:** recalibração concorrente; corrente reversa quando adotado; faixa não
  calibrada; alimentação não monitorada;
- **ERROR:** configuração inválida; zero fora do limite configurado; alimentação
  fora da faixa; sobrecorrente ou saturação; falha de registro e sua causa.

Nenhum componente escreve diretamente em porta serial, display ou periférico de
saída.

## 9. Falhas e condições de borda

- **Configuração inválida:** campo obrigatório ausente, limite incoerente,
  amostras iguais a zero, razão ou sensibilidade não positiva, polaridade fora
  de `+1,0` ou `−1,0`, ou `lowPassAlpha` fora de `(0,1]` impede registro
  parcial e produz diagnóstico.
- **Zero inválido:** diferença entre zero calibrado e nominal acima de
  `maximumZeroDeviationMv` impede medições válidas.
- **Recalibração sem pré-condição:** ausência de garantia de corrente zero,
  perda de alimentação contínua ou alimentação instável impede a recalibração;
  o zero anterior permanece.
- **Alimentação não monitorada:** produz `NOT_MONITORED`, nunca `IN_RANGE` ou
  `SUPPLY_OUT_OF_RANGE`.
- **Alimentação fora da faixa:** produz `SUPPLY_OUT_OF_RANGE` e remove o valor
  numérico válido.
- **Faixa não calibrada e saturação:** produzem os estados da seção 5.5, sem
  corrente numérica válida.
- **Reinício:** descarta o zero anterior e reinicia aquecimento e calibração.
- **Falha de capacidade, identidade ou conflito:** retorna `nullptr`, registra
  a causa e não consome slot nem deixa adapter ou capability parcial.

## 10. Critérios de aceite e validações

- **CUR-AC-001:** cada perfil materializa exatamente seus valores elétricos
  iniciais, sem tratar `43,05` ou `43,56 mV/A` como constante universal.
  Meio: inspeção da configuração e do cálculo.
- **CUR-AC-002:** alterar `zeroADC`, `sensitivityAdcMvPerA` ou polaridade
  altera o resultado segundo CUR-019, sem dependência direta da alimentação.
  Meio: inspeção com dois conjuntos distintos.
- **CUR-AC-003:** após 60 segundos de aquecimento e calibração sem corrente, a
  indicação de zero permanece entre `−0,05 A` e `+0,05 A`. Meio: validação
  física por perfil.

### CUR-DC-004 — Exatidão da corrente fotovoltaica

O módulo deve medir a corrente contínua entre o painel fotovoltaico e a entrada
do conversor buck utilizando o ACS712-30A.

Na faixa `0,50 A ≤ |I_ref| ≤ 15,00 A`, o erro absoluto deve satisfazer:

```text
E_max = max(0,10 A; 0,05 × |I_ref|)
|I_medido − I_ref| ≤ E_max
```

Valores abaixo de `0,05 A` em magnitude são zero. Valores entre `0,05 A` e
`0,50 A` podem ser estimativas sem garantia de exatidão. A resolução é
`0,01 A` ou melhor. Com corrente constante, a variação pico a pico durante 30
segundos não ultrapassa `0,05 A`; após alteração, a leitura entra na tolerância
em até 1 segundo. Faixa, saturação, sinal negativo e validade numérica seguem as
seções 5.5 e 5.6.

#### Procedimento por perfil

1. Alimentar ESP32 e ACS712 dentro da faixa do perfil, verificando a alimentação
   por instrumento independente.
2. Manter corrente zero por pelo menos 60 segundos e executar a calibração.
3. Aplicar `0,00 A`, `0,50 A`, `1,00 A`, `2,50 A`, `5,00 A`, `10,00 A`,
   `13,50 A`, `15,00 A`, `−0,50 A` e `−5,00 A`.
4. Em cada ponto, registrar ao menos 30 leituras filtradas consecutivas durante
   pelo menos 30 segundos.
5. Comparar média e sinal com multímetro ou shunt de referência cujo erro
   máximo seja `±1%`; verificar erro máximo, resolução e variação pico a pico.
6. Aplicar uma alteração controlada e verificar entrada na tolerância em até
   `1000 ms`.
7. Injetar sinais equivalentes às faixas acima de 15 A e à saturação do ADC,
   sem exigir produção física de 30 A, verificando estados e ausência de valor
   numérico válido.
8. Repetir `0,00 A` após o maior valor físico para verificar deslocamento do
   zero.

O critério é aprovado somente quando, nos dois perfis, todos os pontos
aplicáveis respeitam erro, sinal, estabilidade e tempo; o zero retorna à faixa;
a alimentação observada permanece dentro do perfil; e nenhum valor inválido ou
saturado é apresentado como medição válida. Enquanto qualquer `TBD` da seção
13 permanecer aberto, o procedimento não constitui gate terminal executável.

- **CUR-AC-005:** com polaridade `−1,0`, os pontos assinados invertem o sinal e
  preservam módulo e tolerância de CUR-DC-004. Meio: validação física.
- **CUR-AC-006:** com corrente constante, envelopes idênticos não são
  republicados; alterado valor ou estado, o novo envelope é publicado na
  avaliação seguinte, respeitando `1000 ms`. Meio: hardware.
- **CUR-AC-007:** `|I| < 0,05 A` produz zero exato; `|I| = 0,05 A` não é
  suprimido automaticamente. Meio: hardware com corrente ajustável.
- **CUR-AC-008:** solicitação de recalibração retorna sem calibrar e somente
  começa no `handle()` seguinte quando todas as pré-condições forem satisfeitas.
  Meio: hardware e logs.
- **CUR-AC-009:** cada classe de configuração inválida ou zero inválido impede
  medição e registro parcial conforme a seção 9. Meio: execução instrumentada.
- **CUR-AC-010:** `handle()` do adapter e da capability preserva o ciclo
  cooperativo e o atendimento de outra capability ativa. Meio: hardware.
- **CUR-AC-011:** `pio run -e esp32_dev` alcança estado terminal com sucesso.
  Meio: build canônico.
- **CUR-AC-012:** APIs públicas preexistentes permanecem compatíveis; o novo
  ponteiro é estável, não proprietário, retorna `nullptr` em falha e a
  aplicação libera seus objetos. Meio: inspeção e execução instrumentada.
- **CUR-AC-013:** com monitor independente presente, tensões dentro e fora dos
  limites produzem `IN_RANGE` e `SUPPLY_OUT_OF_RANGE`; sem monitor, somente
  `NOT_MONITORED` é possível. Meio: hardware ou injeção instrumentada.
- **CUR-AC-014:** sinais equivalentes a `15 A < |I| ≤ 30 A`, `|I| > 30 A`
  e saturação produzem os estados correspondentes, com `currentA` ausente.
  Meio: injeção instrumentada.

A execução física ou instrumentada de CUR-AC-003, CUR-DC-004,
CUR-AC-005 a CUR-AC-010 e CUR-AC-012 a CUR-AC-014 exige hardware ou bancada e
permissão operacional explícita. Enquanto não executada, permanece
`Not Executed`.

## 11. Testes

Nenhum artefato de teste automatizado integra o recorte desta versão. As
evidências contratadas são inspeção, build canônico e validação física ou
instrumentada sob autorização explícita. Criar harness persistente ou alterar
suítes exige nova decisão de escopo.

## 12. Conhecimento afetado

- `docs/rfc/KNOWLEDGE-MAP.md`: versão, perfis e lacunas bloqueantes;
- `docs/rfc/EKM-CHANGELOG.md`: autoria da versão 0.2;
- relatório de implementabilidade 0.1: histórico imutável cujo bloqueador de
  tolerância é respondido por CUR-DC-004.

## 13. Relações, decisões e lacunas

### Relações normativas

- `docs/specs/PUBLIC-API-COMPATIBILITY.md` — preservada por API aditiva,
  ownership da aplicação, ponteiro não proprietário e falha sem efeito parcial.
- `docs/specs/CORE-RUNTIME-LIFECYCLE.md` — preservada por configuração antes
  de `setup()`, oito slots e processamento cooperativo.

A relação permanece **Nova** [`New`]. O perfil de 3,3 V é contrato próprio
desta fonte e não amplia o suporte geral de plataforma.

### Decisões do Arquiteto

- **CUR-DEC-001:** medição exclusiva de corrente contínua fotovoltaica entre
  painel e entrada do buck.
- **CUR-DEC-002:** zero não persistido, aquecimento inicial de 60 segundos e
  recalibração posterior com acomodação de 2 segundos e pré-condições.
- **CUR-DEC-003:** perfis `ACS712_30A_5V` e `ACS712_30A_3V3`, qualificados
  respectivamente como `MANUFACTURER_SUPPORTED` e `PROJECT_VALIDATED`.
- **CUR-DEC-004:** sensibilidades iniciais configuráveis de `43,05 mV/A` e
  `43,56 mV/A`, substituíveis por calibração individual.
- **CUR-DEC-005:** API única `app.addCurrentSensor(CurrentSensorConfig)`, com
  ownership pela aplicação e factory somente interno.
- **CUR-DEC-006:** estados de medição e alimentação separados em envelope
  indivisível; alimentação não monitorada não invalida sozinha a leitura.
- **CUR-DEC-007:** faixa calibrada `0,50–15,00 A` em magnitude, erro
  `max(0,10 A; 5%)`, faixa morta `0,05 A`, estabilidade `0,05 A` pico a pico e
  resposta de até 1 segundo.
- **CUR-DEC-008:** sobrefaixa e saturação não publicam corrente numérica válida;
  validação acima de 15 A pode usar injeção instrumental.
- **CUR-DEC-009:** nenhum artefato de teste automatizado integra a versão.
- **CUR-DEC-010:** `FULL_RANGE` é abstração por target; `adcMaximumMv` não é
  presumido a partir da tensão nominal.
- **CUR-DEC-011:** o perfil 3,3 V é principal nas placas existentes; 5 V é
  alternativa de bancada ou hardware futuro.
- **CUR-DEC-012:** `maximumZeroDeviationMv`, `adcMaximumMv` e frequência de
  amostragem podem permanecer `TBD` no Draft, mas bloqueiam implementabilidade.

### Lacunas bloqueantes

- **CUR-GAP-001:** definir `maximumZeroDeviationMv` para cada perfil.
- **CUR-GAP-002:** definir `adcMaximumMv` para cada target suportado.
- **CUR-GAP-003:** definir `sampleIntervalUs` por perfil/target.
- **CUR-GAP-004:** definir a representação no envelope para estimativas entre
  `0,05 A` e `0,50 A`; os três estados confirmados não distinguem estimativa de
  medição `VALID` com exatidão garantida.
- **CUR-GAP-005:** delimitar quais consumidores e reservas de GPIO participam
  da rejeição de pinos conflitantes, sem alterar silenciosamente comportamento
  público preexistente.
- **CUR-GAP-006:** definir a representação textual estável do envelope no
  `value` de `ICapability`, cuja API vigente publica texto.

## 14. Estado da especificação

Versão 0.2 registrada em `Draft`, com implementação `Not Started` e revisão de
implementabilidade `Pending Review`.

CUR-DC-004 substitui o critério anterior de exatidão e resolve a ausência de
tolerância registrada na análise 0.1. A versão não é elegível à implementação enquanto
CUR-GAP-001 a CUR-GAP-006 permanecerem abertos.
