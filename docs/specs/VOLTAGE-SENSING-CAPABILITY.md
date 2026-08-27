# Especificação — Medição de tensão por Hardware Adapter

**ID:** `IOTSSC-VOLTAGE-SENSOR`

**Classe da fonte:** Normativa

**Versão:** 0.1

**Estado do workflow:** `Draft`

**Análise de implementabilidade:** Pendente

**Bloqueio arquitetural:** Nenhum conhecido antes da análise formal

**Relações normativas e de dependência:**

- Nova [`New`] — `VoltageSensorCapability`, `IVoltageSensor`, tipos de tensão,
  adapter de divisor resistivo, API pública e exemplo executável;
- Altera [`Amends`] `IOTSSC-CURRENT-SENSOR@0.6`, somente quanto à arbitragem
  bilateral do GPIO ADC entre sensores de corrente e tensão;
- Preserva `IOTSSC-PUBLIC-API`, `IOTSSC-CORE-RUNTIME` e
  `IOTSSC-HW-EXAMPLES`.

## 1. Objetivo e contexto

Adicionar ao IoTSmartSysCore uma capability genérica de medição de tensão,
independente de classificação conceitual como contínua ou alternada. A
capability deve receber um `IVoltageSensor`, orquestrar lifecycle, cadência e
publicação e manter no Hardware Adapter toda aquisição e conversão elétrica.

O primeiro adapter contratado mede tensão não negativa por divisor resistivo no
ADC do ESP32 clássico. Seus valores reais de R1 e R2 determinam dinamicamente o
fator do divisor; nenhuma razão específica pode ser codificada no algoritmo.

## 2. Escopo

- `VoltageSensorCapability` e type público `"Voltage Sensor (V)"`;
- `IVoltageSensor`, `VoltageMeasurement`, estados e configuração pública;
- `ResistiveDividerVoltageSensor` para Arduino no ESP32 clássico;
- leitura calibrada em milivolts, amostragem média cooperativa e conversão por
  R1/R2;
- limiar inferior configurável do ADC e sentinel público para leitura abaixo
  do mínimo;
- intervalo configurável de avaliação da capability;
- publicação escalar com duas casas e `measurementStatus` opcional no evento;
- API pública aditiva `SmartSysApp::addVoltageSensor()`;
- ownership em arena e falha atômica conforme o precedente de corrente;
- arbitragem bilateral de GPIO ADC entre sensores de corrente e tensão;
- exemplo executável `voltage_sensor` na MCB R1 com divisor 330 kΩ/10 kΩ;
- diagnóstico pela infraestrutura de logging vigente.

## 3. Fora de escopo

- medição de corrente, potência ou energia;
- valor eficaz, frequência, fase, forma de onda ou suporte inicial a tensão
  alternada;
- offset, calibração de zero, correção de ganho ou persistência de calibração;
- garantia de exatidão física dos resistores, ADC ou circuito completo;
- proteção contra sobretensão, isolamento, fusível, corte de carga ou
  dimensionamento de potência e tensão dos resistores;
- novo registro público ou arquitetura transversal de GPIO;
- alteração do limite de oito capabilities ou do lifecycle cooperativo;
- suporte automático a ESP32-C3, ESP32-C6, ESP32-S3, ESP8266 ou ESP-IDF nativo;
- alteração dos campos emitidos por capabilities preexistentes;
- artefatos de teste automatizado.

### 3.1 Arquitetura e organização

**Precedente aplicável:** `IOTSSC-CURRENT-SENSOR@0.6`, incluindo
`CurrentSensorCapability`, `ICurrentSensor`, factory, builder, ownership,
cadência, evento e exemplo executável.

**Elementos preservados:** separação Contracts/Core/Platform/App, arena sem
ownership pelo consumidor, factory como seam interno, configuração antes de
`SmartSysApp::setup()`, oito slots e processamento cooperativo de `handle()`.

**Desvio arquitetural explícito:** nenhum. A reserva comum de pinos permanece
estado privado do builder e abrange somente os sensores de corrente e tensão
que fornecem metadados de ADC nesta baseline.

### 3.2 Limite de escopo funcional

**Capacidades arquiteturais pressupostas:** lifecycle e ownership vigentes de
capabilities e adapters; `CapabilityStateChanged::measurementStatus`; catálogo
de exemplos executáveis; validação de GPIO pelo factory Arduino.

**Preparação arquitetural separada:** não aplicável. A arbitragem bilateral é
local ao builder e não cria API, lifecycle ou política transversal reutilizável.

## 4. Requisitos

### 4.1 Contratos e responsabilidades

- **VLT-001:** `VoltageSensorCapability` deve derivar de `ICapability` e receber
  um `IVoltageSensor&`; seu nome não pode conter qualificação DC ou AC.
- **VLT-002:** `IVoltageSensor` deve derivar de `IHardwareAdapter`, preservar
  `setup()`, `handle()` e `lastStateReadMillis()` e devolver a última
  `VoltageMeasurement` estável sem adquirir nova amostra.
- **VLT-003:** somente o adapter conhece ADC, amostragem, R1, R2, limite
  inferior, limite superior e conversão elétrica. A capability e o exemplo não
  podem recalcular tensão.
- **VLT-004:** o type público deve ser exatamente `"Voltage Sensor (V)"`,
  exposto como `VOLTAGE_SENSOR_TYPE`.

### 4.2 Configuração

- **VLT-005:** `VoltageSensorConfig` deve conter, no mínimo:

```text
id
adcPin
adcResolutionBits
adcAttenuation
adcMinimumMv
adcMaximumMv
r1Ohms
r2Ohms
samplesPerReading
sampleIntervalUs
readingIntervalMs
capabilityEvaluationIntervalMs
```

- **VLT-006:** os defaults são: `adcResolutionBits = 12`, atenuação abstrata
  `FULL_RANGE`, `adcMinimumMv = 144`, `adcMaximumMv = 3100`,
  `samplesPerReading = 100`, `sampleIntervalUs = 200`,
  `readingIntervalMs = 500` e `capabilityEvaluationIntervalMs = 1000`.
  `id`, `adcPin`, `r1Ohms` e `r2Ohms` não possuem default válido.
- **VLT-007:** `r1Ohms` representa o resistor entre a tensão de entrada e o nó
  ADC; `r2Ohms`, o resistor entre o nó ADC e GND. Ambos devem ser finitos e
  estritamente positivos, e a razão resultante deve ser finita.
- **VLT-008:** parâmetros de amostragem e intervalos devem ser estritamente
  positivos; `adcMinimumMv` deve ser não negativo e menor que
  `adcMaximumMv`. Configuração inválida impede registro parcial.
- **VLT-009:** no ESP32 clássico, `FULL_RANGE` deve ser resolvida como
  `ADC_11db`; o GPIO deve pertencer ao ADC1 utilizável com Wi-Fi, conforme o
  precedente vigente. Outros targets não herdam esses limites.

### 4.3 Aquisição e cálculo elétrico

- **VLT-010:** o adapter deve usar `analogReadMilliVolts()`, nunca contagem ADC
  bruta, e coletar no máximo uma leitura por oportunidade elegível de
  `handle()`, sem espera ativa ou lote bloqueante.
- **VLT-011:** uma medição é a média aritmética de
  `samplesPerReading` leituras, separadas por pelo menos `sampleIntervalUs`.
  Depois de uma medição concluída, o próximo lote respeita
  `readingIntervalMs`.
- **VLT-012:** para uma média `adcMilliVolts` maior ou igual a
  `adcMinimumMv` e menor que `adcMaximumMv`, o adapter deve calcular:

```text
dividerRatio = (r1Ohms + r2Ohms) / r2Ohms
voltageV = (adcMilliVolts / 1000) × dividerRatio
```

- **VLT-013:** o algoritmo não pode conter `34`, `330000`, `10000` ou qualquer
  outra razão específica. Para R1 = 330000 Ω e R2 = 10000 Ω, a configuração
  deve produzir `dividerRatio = 34` automaticamente.
- **VLT-014:** não deve existir offset ou subtração do residual ADC antes da
  conversão. Os valores configurados de R1 e R2 são usados diretamente.
- **VLT-015:** uma média estritamente menor que `adcMinimumMv` deve produzir
  `voltageV = -1000.0f` e estado `BELOW_MINIMUM`. O valor é sentinel reservado,
  não uma tensão física. Uma média exatamente igual a `adcMinimumMv` é válida.
- **VLT-016:** uma média maior ou igual a `adcMaximumMv` deve produzir estado
  `ADC_SATURATION` e ausência de `voltageV`.
- **VLT-017:** `lastStateReadMillis()` deve refletir a conclusão de todo lote,
  inclusive quando o resultado for `BELOW_MINIMUM` ou `ADC_SATURATION`.

### 4.4 Estados e publicação

- **VLT-018:** os estados públicos são `NOT_READY`, `VALID`,
  `BELOW_MINIMUM` e `ADC_SATURATION`. Antes do primeiro lote completo,
  `voltageV` é ausente e o estado é `NOT_READY`.
- **VLT-019:** em `VALID`, `ICapability::value` contém a tensão com exatamente
  duas casas decimais, ponto independente de locale e zero negativo
  normalizado. Em `BELOW_MINIMUM`, contém exatamente `"-1000.00"`. Em
  `NOT_READY` ou `ADC_SATURATION`, contém `""`.
- **VLT-020:** a capability deve preencher somente
  `CapabilityStateChanged::measurementStatus`; `supplyStatus` permanece
  ausente. Os tokens devem coincidir exatamente com os nomes dos estados.
- **VLT-021:** a primeira avaliação é imediata. Depois dela, a capability só
  compara publicação quando decorrer `capabilityEvaluationIntervalMs`; o
  timestamp é atualizado mesmo sem mudança. Evento posterior só é emitido
  quando `value` ou `measurementStatus` mudar.
- **VLT-022:** `IVoltageSensor::handle()` deve ser chamado em todo ciclo,
  mesmo quando a avaliação da capability ainda não estiver elegível. O
  timestamp de avaliação é privado e deve tolerar rollover do provedor de tempo.
- **VLT-023:** `VoltageSensorCapability` deve expor acesso não proprietário a
  `const VoltageMeasurement& voltageMeasurement() const`.

### 4.5 API, ownership e conflitos

- **VLT-024:** a API pública deve ser:

```cpp
VoltageSensorCapability *
SmartSysApp::addVoltageSensor(VoltageSensorConfig config);
```

- **VLT-025:** o ponteiro retornado é não proprietário e estável durante a
  vida da aplicação. `nullptr` indica falha, cuja causa deve ser registrada por
  `ILogger` sem consumir slot, adapter, arena ou identidade parcialmente.
- **VLT-026:** `SmartSysApp` deve possuir adapter e capability e rejeitar
  registro iniciado depois de `setup()`.
- **VLT-027:** o factory deve receber `VoltageSensorConfig` e fornecer tamanho,
  alinhamento, construção e destruição do adapter. Defaults virtuais aditivos
  devem preservar factories existentes que não suportem esse adapter.
- **VLT-028:** o builder deve rejeitar identidade já usada por qualquer
  capability e GPIO inválido, reservado ou já reservado por outro sensor de
  tensão ou corrente. A mesma regra bilateral deve valer quando o sensor de
  corrente for registrado depois do sensor de tensão.
- **VLT-029:** a reserva compartilhada não altera APIs públicas de GPIO nem
  passa a arbitrar adapters preexistentes sem metadados de pino.
- **VLT-030:** o construtor de `VoltageSensorCapability` sem intervalo explícito
  deve materializar `1000 ms`; uma sobrecarga aditiva recebe o intervalo da
  configuração.

### 4.6 Exemplo executável

- **VLT-031:** o catálogo deve receber `voltage_sensor`, seu README, seletor
  exclusivo no runner e environment `example_voltage_sensor_mcb_r1`, sem
  alterar o build padrão ou exemplos existentes.
- **VLT-032:** o exemplo deve consumir `SmartSysApp::addVoltageSensor()` e
  `voltageMeasurement()`, sem acessar o adapter nem implementar média, divisor,
  limite ou formatação da capability.
- **VLT-033:** o sinal deve usar exclusivamente o símbolo oficial
  `ITS_MCB01_J4_EXT_ADC`. A ausência do símbolo deve causar erro de build
  compreensível; literais ou macros próprias de GPIO são proibidos.
- **VLT-034:** o exemplo deve configurar explicitamente R1 = 330000 Ω e
  R2 = 10000 Ω, identificador `pv-voltage`, limiar default de 144 mV e
  cadência de avaliação default de 1000 ms.
- **VLT-035:** o boot deve informar exemplo, placa, GPIO resolvido,
  identificador, R1, R2, razão calculada e limiar, sem segredo. O loop chama
  `SmartSysApp::handle()` continuamente e apresenta a última medição em
  cadência própria sem adquirir ou converter.
- **VLT-036:** o README deve documentar a ligação VIN–R1–ADC–R2–GND, GND comum,
  significado de `-1000.00`, mínimo de 4,896 V para o divisor 34:1 com limiar
  de 144 mV, limite derivado do ADC e riscos de tensão, potência e proteção
  fora do software.

## 5. Fluxos, estados e contratos

```text
configuração antes de setup
→ validação atômica, reserva ADC e construção
→ setup do ADC; NOT_READY
→ amostragem cooperativa de 100 leituras
→ média < 144 mV: BELOW_MINIMUM / -1000.00
→ 144 mV ≤ média < 3100 mV: cálculo dinâmico / VALID
→ média ≥ 3100 mV: ADC_SATURATION / valor vazio
→ avaliação imediata e depois a cada intervalo configurado
→ publicação somente quando valor ou estado mudar
```

Para o exemplo 330 kΩ/10 kΩ:

```text
minimumInputV = 0,144 × 34 = 4,896 V
maximumInputBoundaryV = 3,100 × 34 = 105,400 V
```

O limite superior calculado não afirma segurança elétrica nem capacidade dos
resistores ou da placa. `ADC_SATURATION` começa na fronteira configurada.

Exemplos normativos:

```json
{
  "capability_name": "pv-voltage",
  "type": "Voltage Sensor (V)",
  "value": "24.72",
  "measurementStatus": "VALID"
}
```

```json
{
  "capability_name": "pv-voltage",
  "type": "Voltage Sensor (V)",
  "value": "-1000.00",
  "measurementStatus": "BELOW_MINIMUM"
}
```

## 6. Falhas e condições de borda

- R1/R2 zero, negativos, não finitos ou razão não finita impedem registro;
- limiares incoerentes, intervalos ou contagens iguais a zero impedem registro;
- target, atenuação ou GPIO incompatível impedem registro;
- colisão de identidade, GPIO, slot, adapter ou arena retorna `nullptr` sem
  efeito parcial;
- leitura de 143 mV ou menor com default produz `-1000.00`; 144 mV é válida;
- leitura de 3100 mV ou maior com default produz saturação e valor vazio;
- rollover não pode bloquear definitivamente aquisição ou avaliação;
- falha abaixo do mínimo não pode ser reinterpretada como tensão negativa;
- o sentinel `-1000.00` é reservado para `BELOW_MINIMUM` em todo
  `IVoltageSensor` consumido por esta capability.

## 7. Critérios de aceite e validações

### VLT-AC-001 — Divisor dinâmico

**Cobre:** VLT-007, VLT-012 a VLT-014.

- **Dado que** duas configurações usam 330 kΩ/10 kΩ e 47 kΩ/10 kΩ;
- **Quando** ambas recebem a mesma média ADC válida de 1000 mV;
- **Então** produzem respectivamente 34,00 V e 5,70 V, sem constante própria
  do primeiro divisor;
- **Evidência:** inspeção instrumentada do adapter com leituras controladas.

### VLT-AC-002 — Fronteira inferior e sentinel

**Cobre:** VLT-006, VLT-015, VLT-018 a VLT-020.

- **Dado que** `adcMinimumMv = 144` e o divisor é 34:1;
- **Quando** lotes completos resultam em 143 mV, 144 mV e 142 mV;
- **Então** 143 e 142 produzem `BELOW_MINIMUM` e `"-1000.00"`, enquanto
  144 produz `VALID` e `"4.90"`;
- **Evidência:** execução instrumentada e validação no hardware com o nó ADC
  confirmado por instrumento independente.

### VLT-AC-003 — Saturação superior

**Cobre:** VLT-016, VLT-018 a VLT-020.

- **Dado que** `adcMaximumMv = 3100`;
- **Quando** um lote resulta em 3099 mV e outro em 3100 mV;
- **Então** o primeiro é `VALID` e o segundo é `ADC_SATURATION` com valor vazio;
- **Evidência:** execução instrumentada com leituras controladas.

### VLT-AC-004 — Cooperação e cadência

**Cobre:** VLT-010, VLT-011, VLT-021, VLT-022 e VLT-030.

- **Dado que** a capability usa intervalo de 1000 ms;
- **Quando** o runtime chama `handle()` continuamente e a medição muda entre
  avaliações;
- **Então** o adapter coleta no máximo uma amostra por oportunidade, continua
  sendo acionado em todo ciclo, a primeira avaliação é imediata e nenhuma
  avaliação posterior ocorre antes do intervalo;
- **Evidência:** execução instrumentada com provider de tempo e adapter
  controlados, sem criar artefato de teste automatizado.

### VLT-AC-005 — Publicação por mudança

**Cobre:** VLT-019 a VLT-022.

- **Dado que** avaliações elegíveis observam sequencialmente a mesma dupla
  valor/estado, depois apenas mudança de estado e depois mudança de valor;
- **Quando** os eventos são capturados;
- **Então** a primeira dupla é publicada, a repetição é suprimida e cada
  mudança posterior produz exatamente um evento com duas casas e status;
- **Evidência:** execução instrumentada do sink.

### VLT-AC-006 — Registro atômico e reserva bilateral

**Cobre:** VLT-024 a VLT-029.

- **Dado que** sensores de corrente e tensão tentam usar o mesmo ADC nas duas
  ordens possíveis e que outra tentativa possui configuração inválida;
- **Quando** cada registro é solicitado antes de `setup()`;
- **Então** somente o primeiro usuário do GPIO é aceito, os demais retornam
  `nullptr` e nenhuma falha consome slot, arena, adapter ou identidade;
- **Evidência:** execução instrumentada do builder e inspeção de ownership.

### VLT-AC-007 — Exemplo público

**Cobre:** VLT-031 a VLT-036.

- **Dado que** o environment do exemplo seleciona a MCB R1;
- **Quando** o firmware é construído e executado;
- **Então** existe um único `setup()`/`loop()`, o GPIO vem do pinout oficial, a
  API pública registra `pv-voltage`, R1/R2 resultam em razão 34 e o monitor
  apresenta valor e estado sem recalcular tensão;
- **Evidência:** build do exemplo, inspeção de símbolos e validação manual em
  hardware conforme o README.

### 7.1 Evidências planejadas

- **Artefatos de teste no recorte:** nenhum, por decisão explícita do Arquiteto;
- inspeção instrumentada com adapters, provider de tempo e sink controlados,
  sem registrar uma suíte automatizada;
- build canônico e build de `example_voltage_sensor_mcb_r1` na Implementação;
- validação manual posterior na MCB R1 com instrumento independente no nó ADC;
- exatidão física da tensão de entrada não integra o aceite desta versão.

## 8. Conhecimento afetado

- adicionar esta fonte ao índice de autoridade e à cobertura de capabilities no
  `docs/rfc/KNOWLEDGE-MAP.md`;
- registrar no mapa que `-1000.00` significa leitura abaixo de
  `VoltageSensorConfig::adcMinimumMv`, apontando esta especificação como
  autoridade;
- registrar autoria, análise, implementação e revisão como transações EKOM
  separadas;
- atualizar documentação do catálogo e README do exemplo na implementação;
- preservar relatórios, especificações e transações EKM históricas.

## 9. Relações, decisões, lacunas e débitos

**Fatos observados:** `CurrentSensorCapability` já estabelece adapter por
referência, lifecycle cooperativo, cadência configurável, valor escalar,
`measurementStatus`, ownership em arena, factory, validação de ADC e exemplo na
MCB R1. Não existem testes automatizados específicos para o sensor de corrente;
as suítes gerais permanecem em quarentena. `ITS_MCB01_J4_EXT_ADC` resolve o
GPIO 34 no ADC1 do ESP32 clássico.

**Intenção e decisões confirmadas:** capability e type possuem os nomes exatos
contratados; R1/R2 determinam a razão; não existe offset; limiar ADC inferior é
configurável com default 144 mV; abaixo dele publica `-1000.00`; formatação usa
duas casas; GPIO é arbitrado bilateralmente com corrente; nenhum teste
automatizado integra a versão; o exemplo integra o recorte.

**Solução proposta:** implementar por extensão aditiva do precedente de
corrente, com adapter inicial `ResistiveDividerVoltageSensor` e reserva privada
comum aos dois tipos analógicos no builder.

**Decisões pendentes:** nenhuma conhecida no contrato registrado.

**Relações:** `IOTSSC-CURRENT-SENSOR@0.6`, `IOTSSC-PUBLIC-API`,
`IOTSSC-CORE-RUNTIME`, `IOTSSC-HW-EXAMPLES` e `EKOM-CHG-0002`.

**ADRs relacionadas:** nenhuma; não foi identificada mudança arquitetural
durável separável da funcionalidade.

**Autoridades confrontadas:** `AGENTS.md`, `docs/rfc/EKOM-GUIDELINES.md`,
`docs/rfc/KNOWLEDGE-MAP.md`, `PUBLIC-API-COMPATIBILITY.md`,
`CORE-RUNTIME-LIFECYCLE.md`, `EXECUTABLE-HARDWARE-EXAMPLES.md` e
`CURRENT-SENSING-CAPABILITY.md`.

**Relatórios esperados:** análise, implementação e revisão; validação física
posterior conforme risco e recorte.
