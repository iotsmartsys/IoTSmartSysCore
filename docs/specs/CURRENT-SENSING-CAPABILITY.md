# Especificação — Leitura de corrente contínua fotovoltaica

**ID:** IOTSSC-CURRENT-SENSOR

**Classe da fonte:** Normativa

**Versão:** 0.3

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Não aplicável [`Not Applicable`]

**Revisão de implementabilidade:** Pendente [`Pending Review`]

**Relação normativa:** Corrige a versão 0.2 [`Corrects`]

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

O único target contratado nesta versão é o ESP32 clássico com ADC em
`FULL_RANGE`, resolvido pelo adapter como `ADC_11db`. Outros SoCs exigem perfil
próprio antes de serem considerados suportados.

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
- validação física separada para cada perfil;
- target ESP32 clássico e limites utilizáveis do ADC nos dois extremos.

## 3. Fora de escopo

- corrente alternada e valor eficaz (`RMS`);
- potência, energia acumulada, fator de potência e tensão do painel;
- persistência da calibração;
- comandos remotos de calibração por MQTT, HTTP ou provisionamento;
- proteção elétrica, corte de carga ou produção física obrigatória de 30 A
  durante a validação;
- alteração do limite de oito capabilities ou do ciclo cooperativo de
  `handle()`;
- suporte universal do ACS712 original à alimentação de 3,3 V;
- suporte a ESP32-C3, ESP32-C6, ESP32-S3 ou herança automática dos limites do
  ESP32 clássico;
- registro central de GPIO para capabilities e adapters preexistentes.

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
    NOT_READY,
    ESTIMATED,
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

A presença de valor numérico é:

```cpp
numericValuePresent =
    (measurementStatus == CurrentMeasurementStatus::ESTIMATED ||
     measurementStatus == CurrentMeasurementStatus::VALID) &&
    supplyStatus != CurrentSupplyStatus::SUPPLY_OUT_OF_RANGE;
```

A exatidão contratada somente pode ser afirmada quando:

```cpp
contractAccuracyGuaranteed =
    measurementStatus == CurrentMeasurementStatus::VALID &&
    supplyStatus == CurrentSupplyStatus::IN_RANGE;
```

## 5. Requisitos

### 5.1 Contrato do adapter

- **CUR-001:** `ICurrentSensor` deve derivar de `IHardwareAdapter` e preservar
  `setup()`, `handle()` e `lastStateReadMillis()`.
- **CUR-002:** `ICurrentSensor` deve devolver a última `CurrentMeasurement`
  estável sem executar aquisição.
- **CUR-003:** antes da primeira leitura concluída, o estado é `NOT_READY`,
  `currentA` não possui valor e `lastStateReadMillis()` permanece inalterado.
- **CUR-004:** `handle()` deve ser cooperativo: executa no máximo uma leitura
  ADC por oportunidade elegível, respeita intervalo mínimo de
  `sampleIntervalUs`, não usa espera ativa e mantém estado interno acumulado
  até completar a amostra composta. Não pode adquirir 500 ou 2.000 amostras de
  forma bloqueante.
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
adcMinimumMv
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
  zero; `500` amostras por leitura; `sampleIntervalUs = 1000`;
  `lowPassAlpha = 1,0`; intervalo entre
  leituras `500 ms`; faixa morta e mínimo reportável `0,05 A`; faixa calibrada
  por magnitude de `0,50 A` a `15,00 A`; faixa física de `−30,00 A` a
  `+30,00 A`; erro absoluto `0,10 A`; erro relativo `5,0%`.
- **CUR-008:** `adcPin` e `id` não possuem default válido. No target ESP32
  clássico contratado, `adcMinimumMv = 150`, `adcMaximumMv = 3100` e
  `maximumZeroDeviationMv = 100` aplicam-se aos dois perfis elétricos.
- **CUR-009:** `FULL_RANGE` é abstração do contrato e, no ESP32 clássico, deve
  ser convertida pelo adapter para `ADC_11db`. ESP32-C3, ESP32-C6, ESP32-S3 e
  outros SoCs não herdam silenciosamente atenuação ou limites; cada target deve
  declarar seus próprios limites antes de ser suportado.
- **CUR-010:** `adcMinimumMv` e `adcMaximumMv` representam os limites
  efetivamente utilizáveis pelo ADC no target e na atenuação selecionada. Uma
  leitura do sinal de corrente menor ou igual ao limite inferior ou maior ou
  igual ao superior caracteriza saturação; os limites não equivalem
  automaticamente à alimentação nominal de 3,3 V.
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
  em mV, espaçadas por pelo menos `sampleIntervalUs`. A calibração usa
  `zeroCalibrationSamples`; a medição usa `samplesPerReading`. Cada chamada
  elegível de `handle()` acrescenta no máximo uma leitura ao acumulador, sem
  bloquear até a conclusão da média.
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

- **CUR-024:** antes da primeira medição concluída, o estado é `NOT_READY` e
  `currentA` é ausente. Para `|I| < 0,05 A`, o estado é `ESTIMATED` e
  `currentA = 0,000 A`. Para `0,05 A ≤ |I| < 0,50 A`, o estado é `ESTIMATED`
  e `currentA` contém o valor estimado. Para `0,50 A ≤ |I| ≤ 15,00 A`, o
  estado é `VALID` quando as demais condições de validade forem satisfeitas.
- **CUR-025:** para `15,00 A < |I| ≤ 30,00 A`, o estado é
  `OUT_OF_CALIBRATED_RANGE` e `currentA` é ausente.
- **CUR-026:** para `|I| > 30,00 A` ou saturação do ADC, o estado é
  `OVERCURRENT_OR_SATURATION` e `currentA` é ausente.
- **CUR-027:** `ESTIMATED` possui valor numérico, mas nunca afirma a exatidão
  contratada. `VALID` com `NOT_MONITORED` possui valor numérico, mas também não
  afirma exatidão porque a condição de alimentação não foi comprovada. A
  exatidão contratada exige simultaneamente `VALID` e `IN_RANGE`.
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
  conflitantes. Deve rejeitar GPIO sem capacidade ADC ou reservado pelo target,
  `adcPin` igual a `supplyMonitorAdcPin`, reutilização de qualquer desses pinos
  por outro sensor de corrente registrado na mesma instância e identificador já
  usado por qualquer capability. Conflitos com adapters ou capabilities
  preexistentes sem metadados de GPIO ficam fora do escopo e suas APIs não têm
  comportamento alterado.
- **CUR-041:** o factory pode permanecer somente como detalhe interno e seam de
  injeção; a aplicação consumidora não precisa acessá-lo para registrar o sensor.
- **CUR-042:** a capability deve expor acesso não proprietário à última
  `CurrentMeasurement`, ao zero calibrado e à solicitação de recalibração.
- **CUR-043:** a publicação ocorre na primeira avaliação concluída e depois
  quando a representação normalizada do envelope mudar. Leitura sem valor
  numérico presente nunca é publicada como corrente numérica válida.
- **CUR-044:** o envelope deve ser publicado como unidade indivisível no
  `value` textual UTF-8 de `ICapability`, em JSON compacto, sem espaços ou
  quebra de linha e com propriedades na ordem fixa `currentA`,
  `measurementStatus`, `supplyStatus`. `currentA` presente usa exatamente três
  casas decimais, ponto independente de locale e zero negativo normalizado para
  `0.000`; valor ausente usa `null`; os tokens de estado são exatamente os nomes
  das enumerações; nenhuma propriedade é omitida. Exemplos normativos:

```json
{"currentA":0.742,"measurementStatus":"VALID","supplyStatus":"NOT_MONITORED"}
{"currentA":0.231,"measurementStatus":"ESTIMATED","supplyStatus":"IN_RANGE"}
{"currentA":null,"measurementStatus":"OUT_OF_CALIBRATED_RANGE","supplyStatus":"IN_RANGE"}
```
- **CUR-045:** o limite de oito capabilities, a configuração anterior a
  `SmartSysApp::setup()` e todas as assinaturas, defaults e semânticas públicas
  preexistentes permanecem preservados.

## 6. Perfis elétricos iniciais

### 6.0 Target `ESP32_CLASSIC_ADC_11DB`

| Campo | Valor |
|---|---|
| SoC | ESP32 clássico |
| `adcResolutionBits` | `12` |
| `adcAttenuation` | `FULL_RANGE`, resolvida como `ADC_11db` |
| `adcMinimumMv` | `150 mV` |
| `adcMaximumMv` | `3100 mV` |
| `sampleIntervalUs` | `1000 µs` |
| `maximumZeroDeviationMv` | `100 mV` |

Os limites valem inicialmente para ambos os perfis elétricos. O desvio máximo
de zero admite a variação ratiométrica esperada dentro de cada faixa de
alimentação, mas rejeita zero excessivamente distante do centro nominal.

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
| `maximumZeroDeviationMv` | `100 mV` |
| `adcMinimumMv` | `150 mV` |
| `adcMaximumMv` | `3100 mV` |
| `sampleIntervalUs` | `1000 µs` |

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
| `maximumZeroDeviationMv` | `100 mV` |
| `adcMinimumMv` | `150 mV` |
| `adcMaximumMv` | `3100 mV` |
| `sampleIntervalUs` | `1000 µs` |

A sensibilidade teórica inicial decorre de:

```text
66 mV/A × 3,3 V / 5,0 V = 43,56 mV/A
```

Este é o perfil principal das placas já fabricadas com fonte Hi-Link de 3,3 V.
Sua aceitação exige validação de exatidão, linearidade, estabilidade térmica,
zero, saturação e repetibilidade. O perfil de 5 V permanece alternativa de
bancada ou de futuras revisões de hardware.

## 7. Fluxo esperado

1. A aplicação preenche `CurrentSensorConfig` com um perfil elétrico e o perfil
   contratado do target ESP32 clássico.
2. Antes de `SmartSysApp::setup()`, chama `app.addCurrentSensor(config)`.
3. A aplicação valida identidade, slots, configuração e conflitos, constrói e
   passa a possuir adapter e capability.
4. `setup()` configura o ADC e inicia o aquecimento de 60 segundos.
5. Após aquecimento com corrente zero e alimentação estável, o adapter calibra o
   zero incrementalmente, com uma leitura ADC por oportunidade elegível, e
   passa a produzir medições pelo mesmo mecanismo cooperativo.
6. Cada `SmartSysApp::handle()` aciona a capability e o adapter; a capability
   publica o envelope normalizado em cadência de até `1000 ms`.
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

- **Configuração inválida:** campo obrigatório ausente, limite ADC incoerente,
  amostras iguais a zero, razão ou sensibilidade não positiva, polaridade fora
  de `+1,0` ou `−1,0`, ou `lowPassAlpha` fora de `(0,1]` impede registro
  parcial e produz diagnóstico.
- **Target ou GPIO inválido:** target não contratado, GPIO sem ADC, reservado,
  repetido no mesmo sensor ou reutilizado por outro sensor de corrente da mesma
  aplicação impede registro; identificador já usado por qualquer capability
  também impede registro.
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
  corrente numérica; leitura do sinal nos limites `adcMinimumMv` ou
  `adcMaximumMv`, ou além deles, é saturação.
- **Reinício:** descarta o zero anterior e reinicia aquecimento e calibração.
- **Falha de capacidade, identidade ou conflito:** retorna `nullptr`, registra
  a causa e não consome slot nem deixa adapter ou capability parcial.

## 10. Critérios de aceite e validações

- **CUR-AC-001:** cada perfil materializa exatamente seus valores elétricos
  iniciais, sem tratar `43,05` ou `43,56 mV/A` como constante universal.
  O target ESP32 clássico materializa `ADC_11db`, `150–3100 mV`, `1000 µs` e
  desvio máximo de zero de `100 mV`, sem herança por outro SoC. Meio: inspeção
  da configuração e do cálculo.
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
saturado é apresentado como medição válida.

- **CUR-AC-005:** com polaridade `−1,0`, os pontos assinados invertem o sinal e
  preservam módulo e tolerância de CUR-DC-004. Meio: validação física.
- **CUR-AC-006:** cada exemplo normativo de CUR-044 é produzido byte a byte,
  incluindo ordem, três casas, `null`, tokens e ausência de espaços; envelopes
  normalizados idênticos não são republicados e qualquer alteração gera nova
  publicação na avaliação seguinte, respeitando `1000 ms`. Meio: execução
  instrumentada e hardware.
- **CUR-AC-007:** antes da primeira medição, o envelope contém `NOT_READY` e
  `null`; `|I| < 0,05 A` produz `ESTIMATED` e `0.000`; `|I| = 0,05 A` produz
  estimativa não suprimida; `|I| = 0,50 A` pode produzir `VALID`. Meio:
  hardware com corrente ajustável.
- **CUR-AC-008:** solicitação de recalibração retorna sem calibrar e somente
  começa no `handle()` seguinte quando todas as pré-condições forem satisfeitas.
  Meio: hardware e logs.
- **CUR-AC-009:** cada classe de configuração inválida ou zero inválido impede
  medição e registro parcial; também são rejeitados GPIO sem ADC, reservado,
  repetido ou usado por outro sensor de corrente da instância, além de
  identificador já usado por qualquer capability. Meio: execução instrumentada.
- **CUR-AC-010:** cada oportunidade elegível de `handle()` executa no máximo
  uma leitura ADC, respeita `1000 µs`, não espera ativamente e preserva o
  atendimento de outra capability durante o acúmulo de 500 ou 2.000 amostras.
  Meio: hardware e instrumentação de chamadas ADC.
- **CUR-AC-011:** `pio run -e esp32_dev` alcança estado terminal com sucesso.
  Meio: build canônico.
- **CUR-AC-012:** APIs públicas preexistentes permanecem compatíveis; o novo
  ponteiro é estável, não proprietário, retorna `nullptr` em falha e a
  aplicação libera seus objetos. Meio: inspeção e execução instrumentada.
- **CUR-AC-013:** com monitor independente presente, tensões dentro e fora dos
  limites produzem `IN_RANGE` e `SUPPLY_OUT_OF_RANGE`; sem monitor, somente
  `NOT_MONITORED` é possível. Meio: hardware ou injeção instrumentada.
- **CUR-AC-014:** sinais equivalentes a `15 A < |I| ≤ 30 A`, `|I| > 30 A`,
  tensão menor ou igual a `150 mV` e tensão maior ou igual a `3100 mV` produzem
  os estados correspondentes, com `currentA` ausente. Meio: injeção
  instrumentada.

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

- `docs/rfc/KNOWLEDGE-MAP.md`: versão e encerramento das lacunas da versão 0.2;
- `docs/rfc/EKM-CHANGELOG.md`: autoria da versão 0.3;
- relatórios de implementabilidade 0.1 e 0.2: históricos imutáveis cujos
  bloqueadores foram incorporados às versões posteriores.

## 13. Relações e decisões

### Relações normativas

- `docs/specs/PUBLIC-API-COMPATIBILITY.md` — preservada por API aditiva,
  ownership da aplicação, ponteiro não proprietário e falha sem efeito parcial.
- `docs/specs/CORE-RUNTIME-LIFECYCLE.md` — preservada por configuração antes
  de `setup()`, oito slots e processamento cooperativo.

A versão 0.3 **Corrige** [`Corrects`] a versão 0.2 ao incorporar as decisões que
encerram seus bloqueadores. A fonte permanece uma extensão aditiva em relação
às APIs preexistentes. O perfil de 3,3 V é contrato próprio desta fonte e não
amplia o suporte geral de plataforma.

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
- **CUR-DEC-010:** `FULL_RANGE` é abstração por target; `adcMinimumMv` e
  `adcMaximumMv` não são presumidos a partir da tensão nominal.
- **CUR-DEC-011:** o perfil 3,3 V é principal nas placas existentes; 5 V é
  alternativa de bancada ou hardware futuro.
- **CUR-DEC-012:** o target inicial é o ESP32 clássico com `FULL_RANGE`
  convertido para `ADC_11db`, limites utilizáveis de `150–3100 mV`, intervalo
  de amostragem de `1000 µs` e desvio máximo de zero de `100 mV`; nenhum outro
  SoC herda esses valores silenciosamente.
- **CUR-DEC-013:** aquisição e calibração são máquinas cooperativas, com no
  máximo uma leitura ADC por oportunidade elegível de `handle()` e estado
  acumulado entre chamadas.
- **CUR-DEC-014:** `NOT_READY` e `ESTIMATED` completam os estados públicos;
  presença numérica e garantia de exatidão são qualificações distintas.
- **CUR-DEC-015:** conflitos obrigatórios de GPIO ficam contidos aos sensores
  de corrente da mesma instância, às capacidades e reservas do target e à
  igualdade entre pinos do próprio sensor; identidade conflita com qualquer
  capability.
- **CUR-DEC-016:** o `value` textual é o JSON compacto normalizado de CUR-044 e
  sua representação byte a byte governa detecção de mudança.

As decisões `CUR-DEC-012` a `CUR-DEC-016` encerram no contrato da versão 0.3 as
lacunas `CUR-GAP-001` a `CUR-GAP-006` registradas na versão 0.2. Não há lacuna
normativa aberta conhecida nesta versão; a conclusão de implementabilidade
permanece reservada à nova análise formal.

## 14. Estado da especificação

Versão 0.3 registrada em `Draft`, com implementação `Not Started` e revisão de
implementabilidade `Pending Review`.

Esta revisão material incorpora as decisões do Arquiteto que respondem ao
relatório de implementabilidade 0.2. A versão somente se torna elegível à
implementação se uma nova análise formal a classificar como `Ready` e houver
ordem explícita do Arquiteto para implementar a versão 0.3.
