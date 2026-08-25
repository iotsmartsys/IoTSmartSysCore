# Especificação — Leitura de corrente contínua

**ID:** IOTSSC-CURRENT-SENSOR

**Classe da fonte:** Normativa

**Versão:** 0.1

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Não aplicável [`Not Applicable`]

**Revisão de implementabilidade:** Pendente [`Pending Review`]

**Relação normativa:** Nova [`New`]

## 1. Objetivo e contexto

Contratar a medição de **corrente contínua** no runtime do IoTSmartSysCore.

O ecossistema já reporta grandezas físicas por meio do par Hardware Adapter mais
Capability: o adapter isola o sensor e entrega uma leitura estável, e a
capability reporta essa leitura ao ecossistema em cadência controlada. O
precedente equivalente mais próximo é `IGlpMeter`, implementado por
`HX711WeightMeter` e reportado por `GlpMeterKgCapability`.

Não existe hoje contrato para medição de corrente. Esta especificação define:

- `ICurrentSensor`, contrato de Hardware Adapter responsável por fornecer
  leitura estável de corrente em ampères;
- `ACS712C30ACurrentSensor`, implementação para sensor de efeito Hall
  ACS712-30A com saída analógica condicionada por divisor resistivo;
- `CurrentSensorCapability`, responsável por reportar a corrente medida.

Esta especificação descreve a funcionalidade de forma autossuficiente. Nenhuma
leitura de código preexistente é necessária para implementá-la, e ela não
descreve migração, adaptação ou preservação de qualquer código de bancada
anterior.

## 2. Escopo

- contrato de Hardware Adapter de corrente contínua;
- implementação para ACS712-30A com divisor resistivo de condicionamento;
- calibração do ponto de zero e recalibração sob demanda;
- amostragem, média, faixa morta e cadência que produzem leitura estável;
- capability que reporta a corrente medida;
- extensão aditiva da fábrica de sensores e da API pública de registro de
  capabilities;
- registro de diagnóstico por `iotsmartsys::core::ILogger`.

## 3. Fora de escopo

- corrente alternada e valor eficaz (`RMS`);
- potência, energia acumulada, fator de potência e medição de tensão;
- persistência da calibração;
- comandos remotos de calibração por MQTT, HTTP ou provisionamento;
- detecção de sobrefaixa, proteção ou corte de carga;
- alteração do limite de oito capabilities ou do ciclo cooperativo de
  `handle()`.

## 4. Componentes e responsabilidades

| Componente | Local | Responsabilidade |
|---|---|---|
| `ICurrentSensor` | `src/Contracts/Sensors/` | Contrato do Hardware Adapter de corrente |
| `ACS712C30ACurrentSensor` | `src/Platform/Arduino/Sensors/` | Aquisição analógica, calibração de zero, filtragem e faixa morta |
| `CurrentSensorCapability` | `src/Contracts/Capabilities/` e `src/Core/Capabilities/` | Avaliação periódica e publicação do estado |
| `CURRENT_SENSOR_TYPE` | `src/Contracts/Capabilities/ICapabilityType.h` | Tipo público da capability |
| `CurrentSensorCreationConfig` e `createCurrentSensor()` | `src/Contracts/Sensors/ISensorFactory.h` e `src/Infra/Factories/SensorFactory.*` | Construção do adapter |
| `CurrentSensorConfig` e registro público | `src/App/Builders/Configs/CapabilityConfig.h`, `src/App/Builders/Builders/CapabilitiesBuilder.*` e `src/SmartSysApp.*` | Registro da capability pela API pública |

A separação é normativa: o adapter é a única fronteira que conhece aquisição
analógica, calibração e filtragem; a capability não executa conversão elétrica,
apenas reporta o valor entregue pelo adapter.

## 5. Requisitos

### 5.1 Contrato do adapter

- **CUR-001:** `ICurrentSensor` deve derivar de `IHardwareAdapter` e preservar
  `setup()`, `handle()` e `lastStateReadMillis()`.
- **CUR-002:** `ICurrentSensor` deve expor `float getAmperes() const`,
  devolvendo a última leitura estável em ampères, com sinal, sem executar
  aquisição.
- **CUR-003:** antes da primeira leitura concluída, `getAmperes()` deve
  devolver `0,0 A`.
- **CUR-004:** `handle()` deve ser cooperativo: retorna sem esperas
  indefinidas, sem laços de espera por evento externo e sem consumir mais do que
  a duração de uma amostra composta configurada.
- **CUR-005:** `lastStateReadMillis()` deve refletir o instante da última
  leitura concluída com sucesso.

### 5.2 Implementação ACS712-30A

- **CUR-006:** `ACS712C30ACurrentSensor` deve implementar `ICurrentSensor` e
  receber sua configuração por estrutura própria, sem depender de macros de
  build para valores de operação.
- **CUR-007:** a configuração deve oferecer, com os defaults indicados: pino
  analógico de entrada (sem default válido); sensibilidade do sensor
  `66,0 mV/A`; resistor série do divisor `10 kΩ`; resistor de terra do divisor
  `20 kΩ`; fator de calibração `1,0`; polaridade `+1,0`; faixa morta `0,10 A`;
  amostras de calibração de zero `2000`; amostras de leitura `500`; intervalo
  entre amostras `200 µs`; intervalo entre leituras `500 ms`; tempo de
  acomodação antes da calibração `2000 ms`.
- **CUR-008:** `setup()` deve configurar o conversor analógico com resolução de
  **12 bits** e a atenuação que cobre a faixa completa de entrada do pino, e em
  seguida executar a calibração de zero descrita em 6.3.
- **CUR-009:** todas as leituras de tensão devem usar a conversão calibrada em
  milivolts oferecida pela plataforma, nunca a contagem bruta do conversor.
- **CUR-010:** a implementação deve expor, fora da interface,
  `void requestZeroCalibration()` e `float getZeroMillivolts() const`.
- **CUR-011:** `requestZeroCalibration()` não pode executar a calibração de
  imediato; deve apenas agendá-la para o próximo `handle()`.
- **CUR-012:** o adapter não pode acessar porta serial, display ou qualquer
  periférico de saída diretamente; todo diagnóstico usa `ILogger`, conforme a
  seção 8.

### 5.3 Capability

- **CUR-013:** `CurrentSensorCapability` deve derivar de `ICapability` e ser
  construída com referência a `ICurrentSensor` e ponteiro para
  `ICapabilityEventSink`.
- **CUR-014:** `ICapabilityType.h` deve definir
  `CURRENT_SENSOR_TYPE` com o valor literal `"Current Sensor (A)"`.
- **CUR-015:** a identidade deve ser resolvida por
  `resolveIdentity(cfg.capability_name, CURRENT_SENSOR_TYPE, name)`, como nas
  demais capabilities que aceitam nome configurável; falha de resolução impede o
  registro.
- **CUR-016:** `setup()` deve delegar ao `setup()` do adapter e não executar
  aquisição própria.
- **CUR-017:** `handle()` deve avaliar o adapter no máximo a cada **1000 ms**,
  usando o provedor de tempo do runtime.
- **CUR-018:** o estado publicado deve ser o valor em ampères formatado com
  **três casas decimais** e sinal explícito, no padrão `"+0.000"` e `"-1.250"`.
- **CUR-019:** a publicação deve ocorrer por `updateState` quando o valor
  avaliado diferir do último valor publicado, e também na primeira avaliação
  após o registro.
- **CUR-020:** não existe tolerância de variação: o critério de publicação é a
  mudança do valor, conforme o precedente `GlpMeterKgCapability`.
- **CUR-021:** a capability deve expor `float getAmperes() const` com o último
  valor publicado.

### 5.4 API pública

- **CUR-022:** `ISensorFactory` deve receber
  `createCurrentSensor(const CurrentSensorCreationConfig &cfg)`, devolvendo
  `std::unique_ptr<ICurrentSensor>`, sem alterar assinatura existente.
- **CUR-023:** `SensorFactory` deve construir `ACS712C30ACurrentSensor`,
  propagar a configuração recebida e injetar o `ILogger` do runtime.
- **CUR-024:** `CurrentSensorConfig` deve derivar da configuração de capability
  vigente para sensores e transportar ponteiro para `ICurrentSensor`.
- **CUR-025:** `CapabilitiesBuilder` e `SmartSysApp` devem oferecer registro da
  capability em forma aditiva, preservando o limite de oito capabilities e a
  exigência de configuração anterior a `SmartSysApp::setup()`.
- **CUR-026:** nenhuma assinatura, default ou comportamento público existente
  pode ser alterado por esta especificação.

## 6. Cálculos normativos

Esta seção é contrato. Os valores default são os declarados em CUR-007.

### 6.1 Amostra composta

Uma amostra composta é a média aritmética de `N` leituras consecutivas de
tensão, espaçadas pelo intervalo entre amostras:

```
V̄ = ( Σ vᵢ ) / N          vᵢ em mV,  i = 1..N
```

`N` vale `amostrasDeZero` (default `2000`) na calibração de zero e
`amostrasDeLeitura` (default `500`) na medição. O intervalo entre leituras
consecutivas é `intervaloEntreAmostras` (default `200 µs`).

### 6.2 Sensibilidade efetiva

A saída do sensor passa por um divisor resistivo com `R1` em série entre o
sensor e a entrada analógica, e `R2` entre a entrada analógica e o terra:

```
razão = R2 / ( R1 + R2 )
S     = Ssensor × razão
```

Com os defaults:

```
razão = 20 kΩ / ( 10 kΩ + 20 kΩ ) = 0,6667
S     = 66,0 mV/A × 0,6667 ≈ 44,00 mV/A
```

`S` é a sensibilidade que efetivamente chega ao conversor e deve ser sempre
derivada dos parâmetros configurados. É proibido embutir `44 mV/A`, ou qualquer
outro resultado, como constante independente de `Ssensor`, `R1` e `R2`.

### 6.3 Calibração de zero

Executada em `setup()` e a cada recalibração agendada:

1. registrar o início da calibração;
2. aguardar `tempoDeAcomodação` (default `2000 ms`);
3. medir uma amostra composta com `N = amostrasDeZero`;
4. armazenar o resultado como `V_zero`, em mV;
5. registrar o fim da calibração e o valor de `V_zero`.

A calibração pressupõe **ausência de corrente** no condutor medido durante a
medição. Garantir essa condição é responsabilidade do operador; a
especificação não contrata detecção automática dessa condição. `V_zero` é o
offset de referência usado em todas as leituras seguintes até a próxima
recalibração.

### 6.4 Corrente

Para cada leitura, com `V̄` obtido conforme 6.1 usando `amostrasDeLeitura`:

```
I = ( ( V̄ − V_zero ) / S ) × fatorCalibração × polaridade
```

- `fatorCalibração` (default `1,0`) corrige o resultado contra referência
  externa de medição;
- `polaridade` assume `+1,0` ou `−1,0` (default `+1,0`) e corrige o sentido
  conforme a montagem, sem alterar o módulo.

### 6.5 Faixa morta

Aplicada ao resultado de 6.4:

```
se |I| < faixaMorta   então   I = 0,000 A
```

Com `faixaMorta` default de `0,10 A`. O sensor de efeito Hall possui incerteza
e deriva térmica da mesma ordem de grandeza em repouso; sem essa supressão, o
repouso publicaria oscilação de ruído indefinidamente.

### 6.6 Cadência e estabilidade

- O adapter produz nova leitura no máximo a cada `intervaloEntreLeituras`
  (default `500 ms`), sempre dentro de `handle()`.
- Concluída a leitura, o valor passa a ser a **leitura estável** devolvida por
  `getAmperes()` e `lastStateReadMillis()` é atualizado.
- Entre leituras, `getAmperes()` devolve a última leitura estável, sem executar
  aquisição.

## 7. Fluxo esperado

1. A aplicação constrói o adapter pela fábrica de sensores, informando pino e
   parâmetros elétricos desejados.
2. A aplicação registra `CurrentSensorCapability` pela API pública, antes de
   `SmartSysApp::setup()`.
3. `SmartSysApp::setup()` aciona `setup()` da capability, que aciona `setup()`
   do adapter: conversor configurado e zero calibrado.
4. Cada `SmartSysApp::handle()` aciona `handle()` da capability e do adapter.
5. O adapter mede a cada `intervaloEntreLeituras` e mantém a leitura estável.
6. A capability avalia a cada `1000 ms` e publica quando o valor muda.
7. Uma recalibração solicitada por `requestZeroCalibration()` executa no
   próximo `handle()` do adapter; durante a acomodação e a medição, a leitura
   estável anterior permanece disponível.

## 8. Diagnóstico

Todo registro de diagnóstico usa `iotsmartsys::core::ILogger`, injetado no
adapter pela fábrica, com a tag do componente:

- **INFO:** início da calibração de zero; fim da calibração com o `V_zero`
  resultante em mV;
- **DEBUG:** leitura concluída, com `V̄`, a diferença `V̄ − V_zero` e a corrente
  resultante;
- **WARN:** recalibração solicitada enquanto outra está em curso;
- **ERROR:** configuração inválida detectada em `setup()`.

Nenhum componente desta especificação escreve diretamente em porta serial ou em
periférico de exibição.

## 9. Falhas e condições de borda

- **Configuração inválida** — pino não definido, `Ssensor ≤ 0`, `R1 ≤ 0`,
  `R2 ≤ 0`, `amostrasDeZero = 0` ou `amostrasDeLeitura = 0`: `setup()` registra
  ERROR, o adapter permanece inerte, `getAmperes()` devolve `0,0 A`,
  `lastStateReadMillis()` permanece inalterado e nenhuma leitura falsa é
  publicada.
- **Recalibração concorrente** — uma nova solicitação recebida enquanto outra
  calibração está em curso é descartada com WARN; não há enfileiramento.
- **Corrente acima da faixa do sensor** — o valor é reportado como medido; a
  especificação não contrata saturação artificial nem sinalização de
  sobrefaixa.
- **Reinício do runtime** — o zero é recalibrado do princípio, pois não há
  persistência de calibração.
- **Ausência de corrente com ruído residual** — a faixa morta de 6.5 garante
  valor exatamente `0,000 A`, sem publicações sucessivas de ruído.

## 10. Critérios de aceite e validações

- **CUR-AC-001:** com os defaults de CUR-007, a sensibilidade efetiva calculada
  conforme 6.2 é `44,00 mV/A`, com tolerância de `± 0,01 mV/A`. Meio:
  inspeção do cálculo com os parâmetros default.
- **CUR-AC-002:** alterando `R1`, `R2` ou `Ssensor`, o valor de `S` acompanha a
  fórmula de 6.2, demonstrando que nenhuma sensibilidade fixa foi embutida.
  Meio: inspeção com dois conjuntos distintos de parâmetros.
- **CUR-AC-003:** após a calibração de zero, sem corrente aplicada, o estado
  publicado é exatamente `+0.000`. Meio: hardware, com o condutor sem corrente.
- **CUR-AC-004:** com corrente contínua conhecida aplicada, o valor reportado
  corresponde à referência externa de medição dentro da tolerância acordada na
  execução da validação física. Meio: hardware, contra referência externa.
- **CUR-AC-005:** com `polaridade = −1,0` e a mesma corrente de CUR-AC-004, o
  valor reportado apresenta sinal invertido e mesmo módulo. Meio: hardware.
- **CUR-AC-006:** com corrente estável, a capability não republica o mesmo
  valor; alterada a corrente, publica na avaliação seguinte, respeitando o
  intervalo de `1000 ms`. Meio: hardware, observando as publicações.
- **CUR-AC-007:** corrente cujo módulo é inferior a `faixaMorta` produz
  `+0.000`, e corrente imediatamente superior produz valor não nulo. Meio:
  hardware, com corrente ajustável.
- **CUR-AC-008:** `requestZeroCalibration()` retorna sem executar a calibração,
  e a calibração ocorre no `handle()` seguinte, observável pelos registros INFO
  de 8. Meio: hardware, pelos logs.
- **CUR-AC-009:** com configuração inválida, `setup()` registra ERROR, nenhuma
  leitura é publicada e `getAmperes()` permanece `0,0 A`. Meio: hardware ou
  execução instrumentada, pelos logs.
- **CUR-AC-010:** `handle()` do adapter e da capability retorna sem bloquear o
  ciclo cooperativo do `SmartSysApp`, preservando o atendimento das demais
  capabilities registradas. Meio: hardware, observando o runtime com outra
  capability ativa.
- **CUR-AC-011:** o build canônico `pio run -e esp32_dev` alcança estado
  terminal com sucesso. Meio: build canônico.
- **CUR-AC-012:** nenhuma assinatura pública preexistente foi alterada. Meio:
  inspeção do delta contra `PUBLIC-API-COMPATIBILITY`.

A validação física de CUR-AC-003 a CUR-AC-010 exige hardware e permissão
operacional explícita do Arquiteto. Enquanto não executada, permanece
`Not Executed` e não pode ser convertida em evidência aprovada.

## 11. Testes

**Nenhum artefato de teste integra o recorte desta versão.** Esta
especificação não exige criar, ampliar, reestruturar ou executar suítes
automatizadas, e nenhuma execução de teste é condição de aceite.

As evidências previstas são o build canônico (CUR-AC-011), a inspeção do delta
(CUR-AC-001, CUR-AC-002, CUR-AC-012) e a validação física sob ordem explícita
(CUR-AC-003 a CUR-AC-010).

## 12. Conhecimento afetado

- `docs/rfc/KNOWLEDGE-MAP.md`: nova fonte normativa e cobertura de capabilities;
- `docs/rfc/EKM-CHANGELOG.md`: transação de autoria desta especificação.

## 13. Relações, decisões e lacunas

### Relações normativas

- `docs/specs/PUBLIC-API-COMPATIBILITY.md` — preservada. Esta especificação é
  extensão aditiva e não altera assinatura, default ou comportamento público
  existente.
- `docs/specs/CORE-RUNTIME-LIFECYCLE.md` — preservada. `setup()` e `handle()`
  cooperativos, limite de oito capabilities e configuração anterior a
  `SmartSysApp::setup()` permanecem inalterados.

Nenhuma fonte vigente governa medição de corrente. A relação é **Nova**
[`New`], sem emenda, correção, substituição ou aposentadoria de contrato
anterior.

### Decisões do Arquiteto

- **CUR-DEC-001:** o recorte cobre exclusivamente corrente contínua. Corrente
  alternada e valor eficaz ficam fora de escopo e dependem de especificação
  futura.
- **CUR-DEC-002:** a calibração de zero não é persistida. O zero é calibrado na
  inicialização e sob solicitação explícita.
- **CUR-DEC-003:** o critério de publicação segue o precedente
  `GlpMeterKgCapability`: avaliação a cada `1000 ms` e publicação por mudança do
  valor, sem tolerância de variação.
- **CUR-DEC-004:** a identidade da capability é resolvida por `resolveIdentity`
  com `CURRENT_SENSOR_TYPE`.
- **CUR-DEC-005:** fábrica de sensores, configuração de capability, builder e
  `SmartSysApp` integram o recorte, em forma aditiva.
- **CUR-DEC-006:** a implementação recebe o nome `ACS712C30ACurrentSensor`.
- **CUR-DEC-007:** nenhuma criação ou execução de teste integra o recorte.
- **CUR-DEC-008:** não há exigência de canal ou faixa específica de pino
  analógico. A escolha do pino é parâmetro de configuração da aplicação, e esta
  especificação não decide com base em ocupação de GPIO por outro hardware.
- **CUR-DEC-009:** `CURRENT_SENSOR_TYPE` tem o valor literal
  `"Current Sensor (A)"`.

### Lacunas

Nenhuma lacuna bloqueante registrada nesta versão.

## 14. Estado da especificação

Versão 0.1 registrada em `Draft`, com as decisões `CUR-DEC-001` a
`CUR-DEC-009` incorporadas. A versão segue para Análise de Implementabilidade.

Nenhuma implementação foi iniciada e nenhuma validação foi executada.
