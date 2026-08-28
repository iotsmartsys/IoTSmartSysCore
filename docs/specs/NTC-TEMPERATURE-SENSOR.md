# Especificação — Sensor de temperatura NTC por divisor resistivo

**ID:** `IOTSSC-NTC-TEMPERATURE-SENSOR`

**Classe da fonte:** Normativa

**Versão:** 0.1

**Estado normativo:** Vigente [`Active`]

**Estado da implementação:** Validada [`Validated`]

**Estado da entrega:** Pronta para integração [`Ready for Integration`]

**Revisão de implementabilidade:** Pronta [`Ready`]

**Bloqueio arquitetural:** Nenhum conhecido antes da análise formal

**Relações normativas e de dependência:**

- Nova [`New`] — `NtcTemperatureSensor`, sua configuração pública, perfis
  iniciais e exemplo executável;
- Altera [`Amends`] `IOTSSC-HW-EXAMPLES`, somente para acrescentar o novo
  exemplo ao catálogo;
- Preserva `IOTSSC-PUBLIC-API` e `IOTSSC-RUNTIME`;
- Preserva `ITemperatureSensor`, `TemperatureSensorCapability`,
  `TemperatureSensorConfig` e
  `SmartSysApp::addTemperatureSensorCapability()` sem alteração de contrato.

## 1. Objetivo e contexto

Adicionar um Hardware Adapter Arduino/ESP32 capaz de medir temperatura por
termistores NTC parametrizáveis e fornecer Celsius por meio da interface
existente `ITemperatureSensor`.

O primeiro recorte deve suportar o NTC 100 kΩ B3950 e o MF52-103 10 kΩ tratado
empiricamente como B3950. A medição usa divisor resistivo, média fracionária de
16 amostras do ADC e equação Beta.

## 2. Escopo

- `NtcTemperatureSensor` em `src/Platform/Arduino/Sensors/`;
- configuração pública dos parâmetros elétricos e térmicos;
- presets para NTC 100 kΩ B3950 e MF52-103 10 kΩ/B3950;
- divisor com resistor série configurável;
- aquisição pelo ADC do ESP32 no Arduino framework;
- média de exatamente 16 amostras sem truncar a fração;
- conversão pela equação Beta;
- sentinel `-1000.0f` para qualquer leitura inválida;
- criação aditiva por `SensorFactory`;
- exemplo executável `environment_ntc` na MCB R1;
- diagnóstico pela infraestrutura de logging vigente.

## 3. Fora de escopo

- alterar `ITemperatureSensor` ou `TemperatureSensorCapability`;
- equação Steinhart–Hart de três coeficientes ou tabela de lookup;
- calibração persistente ou correção automática do Beta;
- compensação geral da não linearidade do ADC;
- identificação automática do modelo ou do circuito aberto/curto como estados
  públicos distintos;
- garantia metrológica em toda a faixa declarada pelo fabricante;
- novo registro transversal ou arbitragem geral de GPIOs;
- suporte a ESP8266 ou ESP-IDF nativo;
- criação ou alteração de testes automatizados.

## 4. Arquitetura e responsabilidades

**Precedentes aplicáveis:** `DHTSensor`, `DS18B20TemperatureSensor`,
`TemperatureSensorCapability`, `SensorFactory` e o exemplo
`environment_dht`. O uso do ADC e do divisor também preserva as convenções
locais aplicáveis de `ResistiveDividerVoltageSensor`.

- `NtcTemperatureSensor` implementa somente aquisição, média, conversão
  elétrica e conversão térmica;
- `TemperatureSensorCapability` continua responsável por cadência, validação
  da faixa pública, arredondamento e publicação;
- `SensorFactory` cria o adapter e expressa seu ownership por
  `std::unique_ptr`;
- o exemplo somente configura e compõe os objetos, sem repetir o algoritmo;
- nenhuma nova camada, facade ou lifecycle é introduzido.

## 5. Configuração pública

### 5.1 Tipo e campos

- **NTC-001:** deve existir `NtcTemperatureSensorConfig`, público junto ao
  adapter, contendo no mínimo:

```cpp
int adcPin;
float nominalResistanceOhms;       // R0
float betaK;
float referenceTemperatureC = 25.0f;
float seriesResistanceOhms;
float supplyVoltageV = 3.3f;
float adcReferenceVoltageV = 3.3f;
std::uint8_t adcResolutionBits = 12;
```

- **NTC-002:** `nominalResistanceOhms` representa a resistência do NTC em
  `referenceTemperatureC`; `seriesResistanceOhms` representa o resistor entre
  a alimentação e o nó ADC.
- **NTC-003:** resistências, `betaK`, tensões e resolução devem ser finitas e
  estritamente positivas. A temperatura de referência, convertida para Kelvin,
  deve ser finita e estritamente positiva.
- **NTC-004:** o pino deve ser válido para ADC1 no ESP32 clássico, preservando
  operação com o Wi-Fi ativo. Configuração inválida deve ser rejeitada pelo
  factory sem ownership ou objeto parcial.

### 5.2 Perfis iniciais

- **NTC-005:** a configuração deve fornecer os seguintes presets públicos, ou
  funções de fábrica semanticamente equivalentes:

```cpp
NtcTemperatureSensorConfig::NTC_100K_B3950(int adcPin);
NtcTemperatureSensorConfig::MF52_103_B3950(int adcPin);
```

| Perfil | R0 | Beta | T0 | Resistor série | Alimentação/referência |
|---|---:|---:|---:|---:|---:|
| `NTC_100K_B3950` | 100000 Ω | 3950 K | 25 °C | 100000 Ω | 3,3 V |
| `MF52_103_B3950` | 10000 Ω | 3950 K | 25 °C | 10000 Ω | 3,3 V |

- **NTC-006:** os presets são conveniências aditivas; todos os parâmetros
  continuam substituíveis por configuração explícita.
- **NTC-007:** a documentação deve qualificar o MF52-103 como compatível com
  B3950 por validação empírica aproximada entre 23 °C e 30 °C. Isso não afirma
  identificação garantida do lote nem exatidão em toda a faixa operacional.

## 6. Circuito contratado

- **NTC-008:** o algoritmo inicial governa exclusivamente esta orientação do
  divisor:

```text
supplyVoltageV
      │
[seriesResistanceOhms]
      │
      ├──── adcPin
      │
     [NTC]
      │
     GND
```

- **NTC-009:** o NTC deve estar entre o nó ADC e GND. Inverter os elementos do
  divisor exige configuração ou algoritmo diferente e não é suportado
  implicitamente.

## 7. Lifecycle e aquisição ADC

- **NTC-010:** `NtcTemperatureSensor` deve implementar integralmente
  `ITemperatureSensor`, incluindo `setup()`, `handle()`,
  `lastStateReadMillis()` e `readTemperatureCelsius()`.
- **NTC-011:** `setup()` deve configurar a resolução do ADC, atenuação
  apropriada à referência configurada no ESP32 clássico e o pino como entrada.
- **NTC-012:** cada chamada de `readTemperatureCelsius()` deve adquirir
  exatamente 16 amostras por `analogRead()`.
- **NTC-013:** a soma deve usar tipo capaz de conter o lote completo e a média
  deve ser calculada em ponto flutuante:

```text
adcAverage = adcSum / 16.0
```

  Divisão inteira, arredondamento prévio ou descarte da fração é proibido.
- **NTC-014:** o lote deve ser limitado e não pode usar `delay()`, espera ativa
  prolongada ou alterar o processamento cooperativo global.
- **NTC-015:** `handle()` não possui aquisição periódica própria neste adapter.
  A aquisição síncrona e limitada ocorre em `readTemperatureCelsius()`, de modo
  que a capability existente possa consumi-lo sem chamada externa adicional.
- **NTC-016:** `lastStateReadMillis()` deve registrar a conclusão de toda
  tentativa de leitura, válida ou inválida.

## 8. Conversão elétrica e térmica

- **NTC-017:** para a média ADC, devem ser calculados:

```text
adcMaximum = (2 ^ adcResolutionBits) - 1
adcVoltage = (adcAverage / adcMaximum) × adcReferenceVoltageV

ntcResistance =
    seriesResistanceOhms × adcVoltage
    / (supplyVoltageV - adcVoltage)
```

- **NTC-018:** a temperatura deve usar a equação Beta:

```text
T0K = referenceTemperatureC + 273.15

1 / TK =
    1 / T0K
    + ln(ntcResistance / nominalResistanceOhms) / betaK

temperatureC = TK - 273.15
```

- **NTC-019:** nenhum valor específico de 10 kΩ, 100 kΩ, 3950 K, 3,3 V ou
  GPIO pode estar embutido no algoritmo. Esses valores pertencem à
  configuração ou aos presets.
- **NTC-020:** com NTC e resistor série iguais e alimentação/referência iguais,
  uma média no centro do ADC deve resultar em resistência próxima de R0 e
  temperatura próxima da referência configurada.
- **NTC-021:** para a orientação contratada, aquecer um NTC válido reduz a
  resistência calculada e aumenta a temperatura calculada.

## 9. Leituras inválidas

- **NTC-022:** `readTemperatureCelsius()` deve retornar exatamente
  `-1000.0f` quando ocorrer qualquer destas condições:

  - configuração inválida;
  - média ADC menor ou igual a zero;
  - média ADC maior ou igual ao fundo de escala;
  - tensão ADC maior ou igual à alimentação configurada;
  - denominador nulo ou negativo;
  - resistência não finita ou não positiva;
  - argumento inválido para o logaritmo;
  - temperatura Kelvin ou Celsius não finita.

- **NTC-023:** `-1000.0f` é sentinel reservado e nunca representa temperatura
  física. Nenhum outro sentinel deve ser introduzido nesta versão.
- **NTC-024:** a `TemperatureSensorCapability` permanece inalterada. Seu limite
  vigente de −40 °C a 125 °C rejeita `-1000.0f` e agenda nova tentativa pelo
  comportamento existente.
- **NTC-025:** o adapter deve registrar diagnóstico de leitura inválida sem
  divisão por zero, overflow, segredo ou publicação de valor intermediário
  como temperatura válida.

## 10. Factory, API e ownership

- **NTC-026:** `SensorFactory` deve receber, de forma aditiva, operação
  equivalente a:

```cpp
std::unique_ptr<iotsmartsys::platform::arduino::NtcTemperatureSensor>
createNtcTemperatureSensor(
    const iotsmartsys::platform::arduino::NtcTemperatureSensorConfig &config);
```

- **NTC-027:** a inclusão não pode alterar a assinatura vigente de
  `createTemperatureSensor(int, TemperatureSensorModel)` nem adicionar método
  virtual puro a `ISensorFactory`.
- **NTC-028:** o NTC não deve ser acrescentado a `TemperatureSensorModel` nesta
  versão, pois a API vigente desse enum recebe somente GPIO e não transporta os
  parâmetros elétricos obrigatórios.
- **NTC-029:** o factory deve retornar `nullptr` e registrar a causa quando a
  configuração for inválida. Em sucesso, o `std::unique_ptr` expressa ownership
  do consumidor.
- **NTC-030:** o consumidor deve manter o adapter vivo enquanto a
  `TemperatureSensorCapability` conservar sua referência, conforme o
  precedente `environment_dht`.

## 11. Exemplo executável

- **NTC-031:** o catálogo deve receber `environment_ntc`, seu README, seletor
  exclusivo no runner e environment `example_environment_ntc_mcb_r1`, sem
  alterar o build padrão ou os exemplos existentes.
- **NTC-032:** o exemplo deve usar exclusivamente o símbolo oficial
  `ITS_MCB01_J4_EXT_ADC`. Sua ausência deve causar erro de build compreensível;
  literal ou macro própria de GPIO é proibido.
- **NTC-033:** o exemplo canônico deve usar o preset `MF52_103_B3950`, resistor
  série de 10 kΩ e capability `environment_temperature`.
- **NTC-034:** o sensor deve ser criado pelo `SensorFactory`, mantido em
  `std::unique_ptr` e fornecido ao `TemperatureSensorConfig` antes de
  `SmartSysApp::setup()`.
- **NTC-035:** o `loop()` deve chamar continuamente somente
  `SmartSysApp::handle()`; o exemplo não deve adquirir ADC, calcular média,
  resistência ou temperatura.
- **NTC-036:** o boot deve informar exemplo, placa, GPIO resolvido, perfil, R0,
  Beta, T0, resistor série, tensões e número de amostras, sem segredo.
- **NTC-037:** o README deve conter montagem, GND comum, alternativa 100 kΩ,
  comandos de build/upload/monitor, procedimento manual, resultado esperado,
  riscos do ADC e ressalva empírica do MF52-103.
- **NTC-038:** `examples/README.md`, `configs/executable_examples.ini`,
  `src/ExecutableExampleRunner.cpp` e a matriz de build dos exemplos devem ser
  reconciliados com a nova entrada.

## 12. Diagnóstico

- **NTC-039:** uma leitura válida deve poder ser diagnosticada pela
  infraestrutura de logging vigente com média ADC fracionária, tensão,
  resistência e temperatura calculadas.
- **NTC-040:** uma leitura inválida deve registrar a causa e o sentinel sem
  tornar logging requisito para o cálculo ou para a publicação da capability.

## 13. Critérios de aceite

| Critério | Requisitos | Cenário e resultado observável | Evidência |
|---|---|---|---|
| NTC-AC-001 | NTC-001 a NTC-004 | A configuração expõe os campos e defaults contratados; valores ou GPIO inválidos são rejeitados antes da criação. | Inspeção da API, entradas controladas e logs do factory. |
| NTC-AC-002 | NTC-010 a NTC-016 | O adapter satisfaz `ITemperatureSensor`; `setup()` configura o ADC, cada leitura conclui 16 amostras sem espera e uma soma não múltipla de 16 preserva a fração. | Inspeção, validação instrumentada e build canônico. |
| NTC-AC-003 | NTC-008, NTC-009, NTC-017 a NTC-020 | No divisor contratado, resistência calculada igual a R0 produz T0 dentro da precisão de ponto flutuante, sem constante específica no algoritmo. | Validação instrumentada com entrada controlada e inspeção. |
| NTC-AC-004 | NTC-021 | Reduzir progressivamente a resistência de entrada aumenta progressivamente a temperatura calculada. | Validação instrumentada e confronto físico. |
| NTC-AC-005 | NTC-022 a NTC-025, NTC-040 | Cada borda inválida retorna exatamente `-1000.0f`, é rejeitada pela capability e registra causa, sem NaN, infinito ou exceção. | Validação instrumentada e logs. |
| NTC-AC-006 | NTC-005 a NTC-007 | Os dois perfis usam o mesmo algoritmo e materializam os parâmetros contratados. | Inspeção da configuração e cálculo controlado. |
| NTC-AC-007 | NTC-026 a NTC-030 | Configuração inválida retorna `nullptr`; sucesso mantém ownership inequívoco e lifetime suficiente. | Inspeção e build do consumidor. |
| NTC-AC-008 | NTC-031 a NTC-038 | O novo environment seleciona somente `environment_ntc`, usa o GPIO oficial e não altera os demais builds. | Builds dos environments afetados e inspeção do ELF/runner. |
| NTC-AC-009 | NTC-033, NTC-039 | Próximo de 25 °C, o MF52-103 apresenta resistência próxima de 10 kΩ e acompanha monotonicamente aquecimento e resfriamento. | Validação física registrada com instrumento de referência. |
| NTC-AC-010 | NTC-007, NTC-037 | A documentação limita corretamente a evidência empírica do MF52-103 à faixa observada. | Inspeção do README e registro físico. |

## 14. Testes e evidências

Nenhum artefato de teste automatizado integra a versão 0.1. Essa decisão
preserva a quarentena vigente das suítes e não dispensa as evidências abaixo:

- integridade textual e inspeção do delta;
- build canônico `pio run -e esp32_dev` durante a Implementação;
- build `pio run -e example_environment_ntc_mcb_r1` durante a Implementação;
- rebuild dos exemplos `environment_dht` e `voltage_sensor` como consumidores
  materiais adjacentes do factory/catálogo;
- validação instrumentada das equações, fração e sentinels;
- validação física do MF52-103 e, quando disponível, do NTC 100 kΩ B3950.

Build não comprova comportamento físico. Durante a implementação, upload,
monitor, instrumentação e hardware permaneceram `Not Executed`. Posteriormente,
o Arquiteto declarou ter executado os testes em hardware, validado o código e
considerado a evidência suficiente para encerrar e integrar a versão 0.1; os
registros brutos não foram recebidos pelo Consultor nesta atuação.

## 15. Decisões confirmadas

- **NTC-DEC-001:** o adapter implementa a interface existente e não altera a
  capability.
- **NTC-DEC-002:** a orientação inicial é resistor série ao positivo e NTC ao
  GND.
- **NTC-DEC-003:** cada leitura usa média fracionária de 16 amostras ADC.
- **NTC-DEC-004:** a conversão térmica usa a equação Beta.
- **NTC-DEC-005:** toda leitura inválida retorna exatamente `-1000.0f`.
- **NTC-DEC-006:** os perfis iniciais são 100 kΩ B3950 e MF52-103 10 kΩ
  empiricamente tratado como B3950.
- **NTC-DEC-007:** o exemplo canônico usa o MF52-103 no ADC oficial J4 da MCB
  R1 e segue o precedente estrutural de `environment_dht`.
- **NTC-DEC-008:** nenhum teste automatizado novo integra a versão 0.1.

## 16. Relações

- `docs/specs/PUBLIC-API-COMPATIBILITY.md`;
- `docs/specs/CORE-RUNTIME-LIFECYCLE.md`;
- `docs/specs/EXECUTABLE-HARDWARE-EXAMPLES.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `EKOM-CHG-0005`;
- `EKOM-CHG-0006`;
- `EKOM-CHG-0007`.

## 17. Encaminhamento

A versão 0.1 está Vigente [`Active`], com implementação Validada
[`Validated`], entrega Pronta para integração [`Ready for Integration`] e
análise de implementabilidade `Ready`.

### Validação final da versão 0.1

O Arquiteto confirmou em ordem direta ter executado os testes em hardware,
validado o código e considerado o resultado suficiente para encerrar a
especificação e integrá-la à `main`. Essa decisão atribui suficiência ao
conjunto NTC-AC-001 a NTC-AC-010 sem alegação de reexecução pelo Consultor nem
disponibilidade dos registros brutos nesta atuação.

A confrontação consultiva não identificou conflito arquitetural ou normativo
bloqueante. Como o Consultor participou das etapas anteriores, o resultado não
é apresentado como revisão independente. O relatório consultivo é
`docs/reports/2026-08-28T151931Z-0.1-d95b28d5-final-validation-report.md`.

Por decisão explícita do Arquiteto, o estado normativo passa de `Draft` para
Vigente [`Active`], a implementação de Implementada [`Implemented`] para
Validada [`Validated`] e a entrega para Pronta para integração
[`Ready for Integration`]. A entrega somente passa a Concluída [`Done`] depois
da integração e sincronização efetivas com `main`.
