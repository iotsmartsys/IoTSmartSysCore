# Especificação — Sensores de tensão e corrente INA3221

**ID:** `IOTSSC-INA3221-SENSORS`

**Classe da fonte:** Normativa

**Versão:** 0.2

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Implementada [`Implemented`]

**Estado da entrega:** Não pronta [`Not Ready`]

**Revisão de implementabilidade:** Pronta [`Ready`]

**Relação normativa:** Corrige [`Corrects`] `IOTSSC-INA3221-SENSORS@0.1`;
permanece uma extensão [`New`] que altera de forma aditiva
[`Amends`] `IOTSSC-CURRENT-SENSOR@0.6`, `IOTSSC-VOLTAGE-SENSOR@0.1`,
`IOTSSC-PUBLIC-API` e `IOTSSC-HW-EXAMPLES@1.1`

## 1. Objetivo e contexto

Adicionar ao runtime Arduino sobre ESP32 dois novos Hardware Adapters baseados
no monitor triplo INA3221 e na biblioteca
`adafruit/Adafruit INA3221 Library`:

- `INA3221VoltageSensor`, implementando `IVoltageSensor`;
- `INA3221CurrentSensor`, implementando `ICurrentSensor`.

Os adapters devem poder consumir o mesmo canal do mesmo dispositivo físico sem
duplicar seu ownership, sua inicialização ou sua configuração global. As
capabilities de tensão e corrente continuam responsáveis por conduzir o
lifecycle dos respectivos adapters e por publicar os snapshots nos contratos
vigentes.

Esta extensão preserva integralmente `ResistiveDividerVoltageSensor`,
`ACS712C30ACurrentSensor` e as APIs públicas que constroem esses modelos.

## 2. Relações de autoridade

- **Nova [`New`]:** `INA3221Device`, `INA3221VoltageSensor`,
  `INA3221CurrentSensor` e suas configurações públicas.
- **Altera [`Amends`] `IOTSSC-VOLTAGE-SENSOR@0.1`:** acrescenta um adapter
  I²C para `IVoltageSensor` e registro de uma implementação externa, sem mudar
  o contrato do divisor resistivo.
- **Altera [`Amends`] `IOTSSC-CURRENT-SENSOR@0.6`:** acrescenta um adapter
  I²C para `ICurrentSensor`, define a ausência de calibração de zero e permite
  registro de uma implementação externa, sem mudar o contrato do ACS712-30A.
- **Altera [`Amends`] `IOTSSC-PUBLIC-API`:** adiciona overloads compatíveis de
  `SmartSysApp` que recebem interfaces de sensores sob ownership externo.
- **Altera [`Amends`] `IOTSSC-HW-EXAMPLES@1.1`:** acrescenta o exemplo
  executável combinado `ina3221_voltage_current`.
- **Preserva `IOTSSC-POWER-ENERGY-CAPABILITY@0.3`:** nenhum cálculo de potência,
  integração, lifecycle ou API dessa capability é alterado.
- **Preserva `IOTSSC-RUNTIME`:** configuração continua anterior a
  `SmartSysApp::setup()`, processamento permanece cooperativo e o limite de
  oito capabilities não muda.

Não há decisão arquitetural durável separável da funcionalidade nesta versão.
O objeto compartilhado representa o único dispositivo físico requerido pelos
dois adapters e permanece contido na integração INA3221.

## 3. Escopo

- representação única e compartilhável do dispositivo INA3221;
- inicialização pela biblioteca Adafruit no barramento `TwoWire` recebido;
- endereço I²C e parâmetros globais de média e conversão configuráveis;
- leitura de tensão de barramento de um canal por `IVoltageSensor`;
- leitura de corrente de um canal por `ICurrentSensor`;
- resistência de shunt, polaridade, faixa, deadband e cadência configuráveis;
- snapshots estáveis e timestamps compatíveis com as interfaces existentes;
- falhas de configuração e inicialização observáveis por estado e log;
- overloads públicos para registrar sensores externos nas capabilities;
- ownership e lifecycle explícitos do dispositivo, adapters e capabilities;
- dependência pública `adafruit/Adafruit INA3221 Library@^1.0.1`;
- exemplo único e combinado na MCB R1, usando tensão e corrente do canal 0;
- documentação e validação física proporcional ao risco elétrico.

## 4. Fora de escopo

- alterar ou substituir `ResistiveDividerVoltageSensor` ou
  `ACS712C30ACurrentSensor`;
- seleção por enumeração dentro de `VoltageSensorConfig` ou
  `CurrentSensorConfig`;
- transferir para `SmartSysApp` o ownership dos adapters INA3221 externos;
- potência, energia, alertas critical/warning, power-valid, timing-control ou
  somatório do INA3221;
- descoberta automática de endereços I²C;
- hot-plug, reconexão ou recuperação automática depois de falha do dispositivo;
- detecção garantida de toda falha I²C ocorrida depois de uma inicialização
  bem-sucedida, quando a biblioteca não expuser erro na leitura do registrador;
- mais de um objeto lógico controlando o mesmo endereço no mesmo barramento;
- arbitragem geral e transversal de dispositivos I²C do runtime;
- proteção elétrica, limitação ativa de corrente ou inferência da potência
  admissível do shunt pela marcação `R100`;
- suporte nativo a ESP-IDF, ESP8266 ou target diferente do ESP32 clássico;
- mudança de payload MQTT, type, formatação ou estados públicos das
  capabilities existentes;
- criação, ampliação, reestruturação ou correção de artefatos automatizados de
  teste.

## 5. Fontes técnicas e limites do componente

A implementação deve usar o pacote PlatformIO publicado da biblioteca oficial,
com versão compatível `^1.0.1`:

```text
https://registry.platformio.org/libraries/adafruit/Adafruit%20INA3221%20Library
https://github.com/adafruit/Adafruit_INA3221
tag 1.0.1
commit a07362b656bd3032c1f7509c46f8f7763ad2a449
```

O contrato considera as seguintes características documentadas:

- três canais numerados pela biblioteca como `0`, `1` e `2`;
- endereço default `0x40` e barramento `Wire` default;
- tensão de barramento nominalmente mensurável de `0 V` a `26 V`;
- tensão de shunt de aproximadamente `−163,84 mV` a `+163,8 mV`;
- resolução de `8 mV` na tensão de barramento e `40 µV` no shunt;
- alimentação do circuito integrado de `2,7 V` a `5,5 V`;
- média programável de 1 a 1024 amostras;
- tempos de conversão programáveis de aproximadamente `140 µs` a `8,244 ms`;
- `begin()` reinicializa o dispositivo e aplica configuração global;
- `getBusVoltage(channel)` devolve tensão em volts;
- `getCurrentAmps(channel)` divide a tensão de shunt pela resistência
  configurada no objeto da biblioteca.

O datasheet aplicável é Texas Instruments `SBOS576C`, revisão de setembro de
2025:

```text
https://www.ti.com/lit/ds/symlink/ina3221.pdf
```

Os limites do circuito integrado não substituem os limites do breakout, da
placa, das trilhas, do resistor de shunt, da carga ou da montagem.

## 6. Componentes e configurações públicas

### 6.1 Dispositivo compartilhado

Deve existir `INA3221Device` em `src/Platform/Arduino/Sensors/`. Ele possui a
única instância de `Adafruit_INA3221`, recebe uma referência não proprietária a
`TwoWire` e oferece aos dois adapters acesso controlado ao mesmo dispositivo.
O driver da biblioteca não deve ser exposto como parte da API pública.

A configuração pública deve representar, no mínimo:

```cpp
enum class INA3221Averaging {
    SAMPLES_1,
    SAMPLES_4,
    SAMPLES_16,
    SAMPLES_64,
    SAMPLES_128,
    SAMPLES_256,
    SAMPLES_512,
    SAMPLES_1024
};

enum class INA3221ConversionTime {
    US_140,
    US_204,
    US_332,
    US_588,
    MS_1,
    MS_2,
    MS_4,
    MS_8
};

struct INA3221DeviceConfig {
    std::uint8_t i2cAddress{0x40};
    INA3221Averaging averaging{INA3221Averaging::SAMPLES_16};
    INA3221ConversionTime busConversionTime{INA3221ConversionTime::MS_1};
    INA3221ConversionTime shuntConversionTime{INA3221ConversionTime::MS_1};
};
```

Os nomes de arquivo e a localização exata desses tipos podem ser ajustados
localmente, desde que permaneçam públicos, coesos com os adapters e sem expor
tipos da biblioteca Adafruit nas assinaturas do Core.

### 6.2 Adapter de tensão

A configuração deve representar, no mínimo:

```cpp
struct INA3221VoltageSensorConfig {
    std::uint8_t channel{3};
    float minimumVoltageV{0.0f};
    float maximumVoltageV{26.0f};
    std::uint32_t readingIntervalMs{500};
};
```

`channel` não possui default válido. O adapter recebe referências não
proprietárias a `INA3221Device` e à configuração copiada de forma segura.

### 6.3 Adapter de corrente

A configuração deve representar, no mínimo:

```cpp
struct INA3221CurrentSensorConfig {
    std::uint8_t channel{3};
    float shuntResistanceOhms{0.0f};
    float polarity{1.0f};
    float deadbandA{0.0f};
    float minimumReportableA{0.0f};
    float maximumAbsoluteCurrentA{0.0f};
    std::uint32_t readingIntervalMs{500};
};
```

`channel`, `shuntResistanceOhms` e `maximumAbsoluteCurrentA` não possuem
default válido. A configuração não contém parâmetros de calibração do ACS712.

## 7. Requisitos

### 7.1 Dispositivo, configuração global e ownership

- **INA-001:** `INA3221Device` deve possuir exatamente uma instância de
  `Adafruit_INA3221` e receber `TwoWire&` como referência não proprietária.
- **INA-002:** a aplicação consumidora deve possuir `INA3221Device` durante
  toda a vida dos adapters que o referenciam.
- **INA-003:** `INA3221Device::setup()` deve ser idempotente. A primeira chamada
  executa `begin(address, &wire)`, valida seu resultado e aplica média, tempos
  de conversão e modo contínuo de tensão de barramento e shunt. Chamadas
  posteriores não reinicializam nem reconfiguram o chip.
- **INA-004:** falha de `begin()` ou de qualquer configuração global obrigatória
  mantém o dispositivo indisponível, produz diagnóstico por
  `iotsmartsys::core::ILogger` e não bloqueia o runtime.
- **INA-005:** o endereço `0x40`, média de 16 amostras e tempos de conversão de
  aproximadamente 1 ms para barramento e shunt são os defaults públicos.
- **INA-006:** o endereço deve ser um dos quatro endereços suportados pelo
  INA3221, de `0x40` a `0x43`. Valores de configuração sem equivalente suportado pela
  biblioteca devem impedir a inicialização, sem fallback silencioso.
- **INA-007:** os dois adapters podem delegar `setup()` ao mesmo dispositivo;
  somente a idempotência de INA-003 autoriza esse compartilhamento.
- **INA-008:** o runtime não assume ownership do `TwoWire`, e destruir o
  dispositivo não deve encerrar um barramento compartilhado pertencente à
  aplicação ou à plataforma.

### 7.2 Contrato comum dos adapters

- **INA-009:** `INA3221VoltageSensor` e `INA3221CurrentSensor` devem implementar
  integralmente suas interfaces públicas e manter referências não proprietárias
  ao mesmo `INA3221Device` quando usados em conjunto.
- **INA-010:** `setup()` de cada adapter deve limpar seu snapshot para
  `NOT_READY`, zerar seu timestamp e solicitar a inicialização idempotente do
  dispositivo.
- **INA-011:** `handle()` deve ser cooperativo, executar no máximo uma leitura
  própria por oportunidade elegível, não usar `delay()`, espera ativa ou loop
  de amostragem e respeitar `readingIntervalMs`.
- **INA-012:** antes da primeira leitura válida, com dispositivo indisponível
  ou depois de valor não finito, o snapshot permanece sem valor em `NOT_READY`.
- **INA-013:** `voltageMeasurement()` e `currentMeasurement()` devolvem
  referências à última medição estável sem executar acesso I²C.
- **INA-014:** `lastStateReadMillis()` começa em zero e só avança quando o
  adapter conclui uma leitura finita, inclusive quando a faixa converte essa
  leitura em estado sem valor publicável.
- **INA-015:** canal maior que 2 ou intervalo zero caracteriza configuração
  inválida e impede uma leitura apresentada como válida.
- **INA-016:** controle de intervalo e timestamp deve tolerar rollover do
  contador de tempo da plataforma.

### 7.3 Tensão de barramento

- **INA-017:** `INA3221VoltageSensor` deve adquirir tensão exclusivamente por
  `Adafruit_INA3221::getBusVoltage(channel)` através do dispositivo
  compartilhado; não aplica divisor resistivo, razão ou offset.
- **INA-018:** `minimumVoltageV` deve ser finito e não negativo;
  `maximumVoltageV` deve ser finito, maior que `minimumVoltageV` e no máximo
  `26,0 V`.
- **INA-019:** leitura finita `V` tal que
  `minimumVoltageV <= V < maximumVoltageV` produz `voltageV = V` e estado
  `VALID`.
- **INA-020:** leitura finita abaixo de `minimumVoltageV` produz o contrato
  vigente `voltageV = -1000.0f` e estado `BELOW_MINIMUM`.
- **INA-021:** leitura finita igual ou superior a `maximumVoltageV` produz
  ausência de `voltageV` e estado `ADC_SATURATION`. Esse estado representa a
  fronteira de medição configurada e não autoriza aplicar tensão acima do
  limite elétrico do componente.
- **INA-022:** a capability preserva seu type, suas duas casas decimais, a
  serialização, a cadência e a detecção de mudança vigentes.

### 7.4 Corrente pelo shunt

- **INA-023:** `shuntResistanceOhms` deve ser finito e estritamente positivo;
  o adapter deve configurá-lo no canal antes da primeira medição.
- **INA-024:** `polarity` aceita somente `+1.0f` ou `−1.0f`; a corrente
  publicada é `polarity × getCurrentAmps(channel)` e preserva sinal.
- **INA-025:** a magnitude máxima teoricamente mensurável deve ser calculada a
  partir do limite de tensão do shunt:

```text
maximumMeasurableCurrentA = 0.1638 / shuntResistanceOhms
```

- **INA-026:** `maximumAbsoluteCurrentA` deve ser finito, positivo e não maior
  que `maximumMeasurableCurrentA`. Esse valor qualifica a medição e não
  constitui proteção elétrica ou corrente termicamente segura.
- **INA-027:** `deadbandA` e `minimumReportableA` devem ser finitos, não
  negativos e não maiores que `maximumAbsoluteCurrentA`; `deadbandA` não pode
  exceder `minimumReportableA`.
- **INA-028:** para `abs(I) < deadbandA`, o resultado é exatamente `0.0 A`, com
  estado `ESTIMATED`.
- **INA-029:** para
  `deadbandA <= abs(I) < minimumReportableA`, o valor preserva sinal e estado
  `ESTIMATED`.
- **INA-030:** para
  `minimumReportableA <= abs(I) <= maximumAbsoluteCurrentA`, o valor preserva
  sinal e estado `VALID`.
- **INA-031:** para `abs(I) > maximumAbsoluteCurrentA`, o valor fica ausente e
  o estado é `OVERCURRENT_OR_SATURATION`.
- **INA-032:** o adapter deve expor `CurrentSupplyStatus::NOT_MONITORED`; a
  detecção I²C do chip não comprova que sua alimentação está dentro de
  `2,7 V` a `5,5 V`.
- **INA-033:** `calibratedZeroAdcMv()` deve retornar `std::nullopt`, pois o INA3221
  não usa a calibração de zero ADC do ACS712.
- **INA-034:** `requestZeroCalibration()` deve ser uma operação não aplicável:
  não altera dispositivo, configuração, snapshot nem timestamp e produz WARN
  diagnóstico quando solicitada.
- **INA-035:** a capability preserva seu type, suas três casas decimais, os
  estados, a serialização, a cadência e a detecção de mudança vigentes.

### 7.5 Registro público e lifecycle

- **INA-036:** as APIs existentes permanecem válidas e continuam construindo
  exclusivamente seus modelos vigentes:

```cpp
CurrentSensorCapability *
SmartSysApp::addCurrentSensor(CurrentSensorConfig config);

VoltageSensorCapability *
SmartSysApp::addVoltageSensor(VoltageSensorConfig config);
```

- **INA-037:** `SmartSysApp` deve acrescentar overloads públicos equivalentes a:

```cpp
CurrentSensorCapability *
SmartSysApp::addCurrentSensor(
    const std::string &id,
    ICurrentSensor &sensor,
    std::uint32_t evaluationIntervalMs = 1000);

VoltageSensorCapability *
SmartSysApp::addVoltageSensor(
    const std::string &id,
    IVoltageSensor &sensor,
    std::uint32_t evaluationIntervalMs = 1000);
```

- **INA-038:** os overloads recebem adapters externos por referência, não os
  constroem, não os destroem e não os registram na arena de adapters da
  aplicação. `SmartSysApp` possui somente a capability criada.
- **INA-039:** dispositivo e adapter externos devem permanecer vivos durante
  toda a vida da capability. O ponteiro devolvido é não proprietário e estável
  durante a vida da aplicação.
- **INA-040:** a capability criada continua chamando uma vez `sensor.setup()`
  em seu `setup()` e uma vez `sensor.handle()` em cada ciclo de seu `handle()`.
- **INA-041:** registro depois do início de `SmartSysApp::setup()`, identidade
  vazia, inválida ou duplicada, intervalo zero, falta de slot ou de arena para
  a capability deve retornar `nullptr`, registrar a causa e não produzir efeito
  parcial.
- **INA-042:** sensores externos I²C não consomem nem reservam GPIO no registro
  privado de sensores ADC. A aplicação consumidora responde por compatibilidade
  do barramento e por não criar dois controladores lógicos para o mesmo
  endereço.
- **INA-043:** os overloads e tipos públicos necessários ao exemplo devem ser
  alcançáveis pelo aggregate público usado por `SmartSysApp.h`, sem exigir
  include de arquivo interno do Core.

### 7.6 Dependência e build

- **INA-044:** `library.json` deve declarar
  `adafruit/Adafruit INA3221 Library` com versão compatível `^1.0.1`; a
  dependência transitiva `Adafruit BusIO` permanece resolvida pelo manifesto da
  biblioteca.
- **INA-045:** o environment do exemplo deve declarar ou herdar explicitamente
  a dependência INA3221, sem depender de instalação global da máquina.
- **INA-046:** a inclusão da dependência não habilita código INA3221 no target
  mínimo quando sensores estiverem excluídos pelo filtro vigente.
- **INA-047:** nenhuma limpeza ampla ou normalização de dependências
  preexistentes em `platformio.ini` integra esta versão.

### 7.7 Exemplo executável combinado

- **INA-048:** o catálogo deve receber `ina3221_voltage_current`, seu README,
  seletor mutuamente exclusivo no runner e environment estável
  `example_ina3221_voltage_current_mcb_r1`.
- **INA-049:** o exemplo deve usar a board `iotsmartsys_mcb_r1`, `Wire`,
  `ESP32_SDA` no GPIO 21, `ESP32_SCL` no GPIO 22, endereço `0x40` e canal `0`.
  Código e documentação devem referenciar os símbolos oficiais; literais podem
  aparecer somente como valores resolvidos para diagnóstico e montagem.
- **INA-050:** uma única instância estática de `INA3221Device` deve ser
  compartilhada por uma instância de `INA3221VoltageSensor` e uma de
  `INA3221CurrentSensor`.
- **INA-051:** o exemplo deve registrar exatamente uma
  `VoltageSensorCapability` e uma `CurrentSensorCapability` pelos overloads de
  INA-037, antes de `SmartSysApp::setup()`.
- **INA-052:** o canal 0 deve usar shunt `R100`, interpretado como
  `0,100 Ω`. A configuração deve limitar
  `maximumAbsoluteCurrentA` a no máximo `1,638 A`.
- **INA-053:** o exemplo não deve reimplementar acesso I²C, conversão, divisão
  pelo shunt, qualificação, formatação ou cadência das capabilities.
- **INA-054:** o boot deve apresentar identificador do exemplo, placa,
  barramento, símbolos e GPIOs resolvidos, endereço, canal, shunt, média,
  tempos de conversão, identidades das capabilities e intervalos, sem segredo.
- **INA-055:** em cadência de apresentação configurada pelo environment e não
  inferior a 1000 ms, o exemplo deve mostrar tensão, corrente,
  `VoltageMeasurementStatus`, `CurrentMeasurementStatus` e
  `CurrentSupplyStatus`, sem bloquear o ciclo cooperativo.
- **INA-056:** retorno `nullptr` de qualquer registro deve ser observado,
  registrado e nunca desreferenciado; a outra capability não pode ocultar a
  falha parcial do exemplo.
- **INA-057:** o README deve documentar objetivo, APIs, ownership, lifecycle,
  placa, endereço, canal, tabela de pinos, esquema high-side, orientação do
  shunt, comandos de build/upload/monitor, sequência manual, resultados,
  limites e riscos elétricos.
- **INA-058:** o README deve distinguir o limite teórico de conversão de
  aproximadamente `±1,638 A` do limite térmico real do módulo. A marcação
  `R100` não comprova potência nominal, corrente segura nem capacidade das
  trilhas.
- **INA-059:** a validação inicial documentada deve usar corrente absoluta não
  superior a `0,5 A`, salvo nova decisão explícita baseada na documentação
  elétrica do módulo e da bancada.
- **INA-060:** o exemplo deve advertir que o INA3221 aceita no máximo `26 V` em
  operação e que software não substitui proteção contra sobrecorrente,
  sobretensão, curto, inversão ou transientes.

## 8. Fluxos e condições de borda

### 8.1 Fluxo combinado

```text
construção estática do dispositivo e dos dois adapters
→ registro das duas capabilities com referências externas
→ SmartSysApp::setup()
→ setup da primeira capability
→ setup idempotente do INA3221Device e setup do primeiro adapter
→ setup da segunda capability
→ INA3221Device já inicializado; setup somente do segundo adapter
→ SmartSysApp::handle()
→ cada capability aciona seu adapter cooperativamente
→ snapshots independentes do mesmo canal
→ publicação somente pelas capabilities vigentes
```

### 8.2 Condições de borda

- a ordem de setup das duas capabilities não altera a configuração global;
- compartilhar dispositivo não transfere ownership para a aplicação;
- duas capabilities podem ter intervalos de avaliação diferentes dos
  intervalos de leitura dos adapters;
- leitura de tensão zero é válida quando `minimumVoltageV` é zero;
- corrente negativa preserva sinal e não implica falha;
- `NOT_MONITORED` permite valor numérico conforme o contrato corrente, mas não
  afirma exatidão garantida da alimentação;
- solicitação de calibração pelo acesso público da capability permanece segura
  e não aplicável ao INA3221;
- remover fisicamente o dispositivo depois do setup não possui detecção ou
  recuperação garantida nesta versão;
- usar shunts diferentes no mesmo canal por adapters concorrentes é configuração
  inválida da aplicação e fica proibido pelo contrato de ownership único;
- excesso de corrente não é interrompido pelo software e pode danificar o
  shunt, o módulo, a placa ou a carga antes da próxima leitura.

## 9. Critérios de aceite

| Critério | Requisitos | Cenário e resultado observável | Meio |
|---|---|---|---|
| INA-AC-001 — Inicialização única | INA-001 a INA-008 | Dois adapters compartilham o dispositivo; `begin()` e configuração global ocorrem uma vez, independentemente da ordem de setup. Falha deixa ambos indisponíveis sem bloquear. | Inspeção e execução instrumentada sem artefato persistente. |
| INA-AC-002 — Adapter de tensão | INA-009 a INA-022 | Canal 0 produz snapshot estável; valores abaixo, dentro e no limite da faixa geram respectivamente `BELOW_MINIMUM`, `VALID` e `ADC_SATURATION`; consulta não lê I²C. | Execução instrumentada e inspeção. |
| INA-AC-003 — Adapter de corrente | INA-023 a INA-035 | Shunt `0,100 Ω` é configurado; sinal e polaridade são preservados; deadband, estimativa, faixa válida e excesso produzem estados contratados; calibração permanece não aplicável. | Execução instrumentada e inspeção. |
| INA-AC-004 — Lifecycle cooperativo | INA-009 a INA-016 e INA-040 | Cada oportunidade executa no máximo uma leitura própria, sem espera; snapshots e timestamps só mudam na conclusão; rollover não paralisa leitura. | Inspeção e execução instrumentada. |
| INA-AC-005 — API e ownership | INA-036 a INA-043 | APIs antigas permanecem compiláveis; overloads registram capabilities externas antes de setup; falhas retornam `nullptr` sem efeito parcial; destruir a aplicação não destrói adapters. | Inspeção, execução instrumentada e build canônico. |
| INA-AC-006 — Dependência | INA-044 a INA-047 | Um consumidor limpo resolve `^1.0.1`; exemplo não depende de biblioteca global; target mínimo preserva exclusão dos sensores. | Inspeção e builds proporcionais. |
| INA-AC-007 — Exemplo combinado | INA-048 a INA-056 | O environment compila exatamente um `setup()`/`loop()`, cria um dispositivo, dois adapters e duas capabilities, usa `Wire`, `ESP32_SDA`, `ESP32_SCL`, endereço `0x40`, canal 0 e shunt R100. | Build do exemplo e inspeção. |
| INA-AC-008 — Documentação e segurança | INA-057 a INA-060 | README contém montagem, lifecycle, comandos, resultados e distingue limite de conversão de limite térmico; ensaio inicial não excede 0,5 A. | Inspeção e validação física autorizada. |
| INA-AC-009 — Hardware | INA-049 a INA-060 | Na MCB R1, tensão e corrente do canal 0 acompanham instrumentos independentes, preservam sinal e estados e não bloqueiam conectividade ou aplicação. | Upload, monitor e validação física com ordem operacional própria. |

Ausência de execução, captura ou hardware deve permanecer explicitamente
`Not Executed`; build não comprova comportamento físico.

## 10. Testes, builds e permissões

Por decisão confirmada pelo Arquiteto no rascunho desta versão, nenhum
artefato automatizado de teste deve ser criado, ampliado, reestruturado ou
corrigido. Os meios instrumentados dos critérios são evidências temporárias de
execução e não autorizam registrar harness ou suíte persistente.

A futura ordem de Implementação inclui os builds construíveis proporcionais:

```text
pio run -e esp32_dev
pio run -e example_ina3221_voltage_current_mcb_r1
```

Execução instrumentada, upload, monitor serial e validação em hardware exigem
ordem operacional própria. Enquanto não autorizados ou não executados, seus
resultados permanecem `Not Executed`.

## 11. Conhecimento afetado

- registrar esta fonte no índice e na cobertura de capabilities do mapa de
  conhecimento;
- registrar a fronteira de ownership do dispositivo e dos adapters externos;
- registrar o exemplo combinado como pendente de implementação;
- registrar a autoria em `docs/rfc/EKOM-CHANGELOG.md`;
- encaminhar a versão 0.2 para Análise de Implementabilidade.

## 12. Decisões, fatos e pendências

**Fatos observados:** `VoltageSensorCapability` e `CurrentSensorCapability`
recebem suas interfaces por referência e acionam `setup()` e `handle()`. As
APIs atuais de `SmartSysApp`, contudo, recebem configurações específicas e o
factory constrói somente o divisor resistivo e o ACS712-30A. O INA3221 já
aparece em dependências de alguns environments e existe um exemplo legado
excluído, mas nenhum desses artefatos contrata adapters públicos ou ownership
compartilhado. O pinout do ESP32 clássico define `ESP32_SDA` como GPIO 21 e
`ESP32_SCL` como GPIO 22.

**Decisões confirmadas pelo Arquiteto:** dispositivo compartilhado; overloads
genéricos de `SmartSysApp`; exemplo único e combinado; MCB R1; `Wire`; SDA 21;
SCL 22; endereço `0x40`; canal 0; shunt com marcação `R100`, interpretado como
`0,100 Ω`; nenhum artefato automatizado de teste neste primeiro recorte.

**Decisões funcionais desta versão:** lifecycle idempotente do dispositivo;
ownership externo; preservação das APIs existentes; média de 16 amostras;
conversão contínua de barramento e shunt; calibração de zero não aplicável;
alimentação não monitorada; limite de conversão derivado do shunt; ensaio
inicial limitado a 0,5 A; dependência PlatformIO `^1.0.1`, confirmada pelo
Arquiteto após a versão 0.1 confundir a tag Git mais recente com a versão
publicada no registro.

**Restrições conhecidas:** a biblioteca Adafruit não oferece um resultado de
erro separado em todas as leituras de registrador; portanto esta versão não
afirma detecção garantida de toda desconexão posterior ao setup. `R100` informa
resistência, não potência ou limite térmico.

**Decisões pendentes:** nenhuma conhecida no contrato registrado. A versão 0.2
resolve a origem e a versão da dependência. Detalhes
locais de organização de headers, nomes auxiliares e validação interna que
preservem esta API pertencem à Implementação.

**Débitos técnicos:** nenhum débito foi aceito pelo Arquiteto nesta autoria.

## 13. Estado da especificação

A versão 0.2 está em Rascunho [`Draft`], com implementação Implementada
[`Implemented`], Análise de Implementabilidade Pronta [`Ready`] e entrega Não
pronta [`Not Ready`]. A dependência dos manifestos foi corrigida e os builds
canônicos proporcionais foram aprovados. A versão segue para Revisão; testes,
upload, monitor e validação em hardware permanecem não autorizados e não
executados.
