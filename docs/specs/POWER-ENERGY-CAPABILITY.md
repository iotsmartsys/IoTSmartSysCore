# Especificação — PowerEnergyCapability

**ID:** `IOTSSC-POWER-ENERGY-CAPABILITY`

**Classe da fonte:** Normativa

**Versão:** 0.4

**Estado normativo:** Vigente [`Active`]

**Estado da implementação:** Validada [`Validated`]

**Estado da entrega:** Concluída [`Done`]

**Revisão de implementabilidade:** Pronta [`Ready`]

**Bloqueio arquitetural:** Nenhum

**Relações normativas e de dependência:**

- Altera [`Amends`] `IOTSSC-POWER-ENERGY-CAPABILITY@0.3` ao substituir a
  dependência direta da capability em sensores de tensão e corrente pela
  abstração `IPowerSensor`, preservando compatibilidade pública;
- Altera de forma aditiva [`Amends`] `IOTSSC-INA3221-SENSORS@0.2` ao acrescentar
  `INA3221PowerSensor`, sem alterar os adapters de tensão e corrente vigentes;
- Depende de `IOTSSC-CURRENT-SENSOR@0.6` e
  `IOTSSC-VOLTAGE-SENSOR@0.1` somente para `CompositePowerSensor`;
- Preserva `IOTSSC-PUBLIC-API` por overload aditivo e manutenção da assinatura
  anterior;
- Preserva `IOTSSC-RUNTIME`, `IOTSSC-HW-EXAMPLES@1.1` e
  `IOTSSC-RUNTIME-CAPABILITY-CAPACITY@0.2` sem mudar lifecycle cooperativo,
  catálogo de exemplos, limite configurável ou composição MCB01 vigente.

## 1. Objetivo e contexto

Desacoplar `PowerEnergyCapability` da origem da potência. A capability deve
consumir um único `IPowerSensor`, integrar energia em watt-hora e publicar o
contrato vigente sem conhecer tensão, corrente, canal, shunt ou método de
cálculo da potência.

Devem existir duas implementações públicas:

- `CompositePowerSensor`, que compõe os últimos snapshots de um
  `IVoltageSensor` e um `ICurrentSensor`;
- `INA3221PowerSensor`, que obtém tensão de barramento e corrente pelo
  `INA3221Device` e calcula a potência do canal.

O INA3221 não possui registrador nem leitura direta de potência. O dispositivo
mede tensão de shunt e tensão de barramento; a biblioteca Adafruit vigente
calcula corrente a partir do shunt. Portanto, `INA3221PowerSensor` também deve
calcular potência por `abs(busVoltage × currentAmps)`. A abstração representa a
origem lógica do snapshot de potência, não cálculo físico no chip.

## 2. Escopo

- contratos públicos `IPowerSensor`, `PowerMeasurement` e
  `PowerMeasurementStatus`;
- `CompositePowerSensor` no Core, recebendo sensores externos por referência;
- `INA3221PowerSensor` e sua configuração pública no runtime Arduino/ESP32;
- consumo de `IPowerSensor&` por `PowerEnergyCapability`;
- lifecycle do `IPowerSensor` conduzido pela capability;
- preservação do lifecycle externo dos sensores subordinados ao composite;
- overload público novo para registrar um `IPowerSensor` externo;
- preservação do overload de tensão e corrente por criação atômica de um
  `CompositePowerSensor` pertencente à aplicação;
- preservação de potência não negativa, integração, reset, estados e
  publicação vigentes;
- testes automatizados dos novos contratos, ownership, lifecycle,
  compatibilidade e qualificações;
- atualização do mapa de conhecimento e da transação EKOM.

## 3. Fora de escopo

- potência fornecida diretamente por registrador do INA3221;
- potência assinada, direção do fluxo, potência aparente ou reativa, fator de
  potência, fase, frequência, forma de onda ou RMS;
- alterar aquisição, calibração, publicação ou API própria dos sensores de
  tensão e corrente existentes;
- fazer `CompositePowerSensor` chamar `setup()` ou `handle()` nos sensores;
- persistência ou sincronização externa da energia, tarifa, custo, demanda ou
  histórico;
- comando remoto para zerar energia;
- alterar payload ou serialização vigentes de `PowerEnergyCapability`;
- alterar `INA3221VoltageSensor` ou `INA3221CurrentSensor`;
- migrar os exemplos existentes ou `mcb01_solar_controller` para a nova API;
- alterar catálogo de exemplos, configuração elétrica MCB R1, capacidade do
  runtime, persistência NVS ou política geral de ownership;
- upload, monitor, hardware, publicação, release ou deploy.

### 3.1 Arquitetura e organização

`IPowerSensor` pertence a `src/Contracts/Sensors/` e não pode depender de tipos
Arduino ou Adafruit. `CompositePowerSensor` pertence ao Core por compor
contratos abstratos. `INA3221PowerSensor` pertence a
`src/Platform/Arduino/Sensors/` por depender do dispositivo Arduino.

`PowerEnergyCapability` continua responsável por cadência de avaliação,
integração de energia, reset e publicação. A aquisição ou composição da
potência passa ao `IPowerSensor`.

O contrato é uma extensão localizada do padrão de Hardware Adapters vigente.
A análise formal deve confirmar que o builder consegue criar atomicamente o
composite de compatibilidade na arena de adapters sem reduzir garantias de
rollback.

## 4. Componentes e tipos públicos

| Elemento | Responsabilidade |
|---|---|
| `IPowerSensor` | Lifecycle e acesso ao último snapshot estável de potência |
| `PowerMeasurement` | Potência opcional e qualificação do snapshot |
| `PowerMeasurementStatus` | `NOT_READY`, `VALID`, `ESTIMATED` e `INPUT_INVALID` |
| `CompositePowerSensor` | Compor tensão e corrente sem conduzir seus sensores |
| `INA3221PowerSensor` | Ler um canal INA3221 e calcular sua potência |
| `INA3221PowerSensorConfig` | Canal, shunt, qualificação e cadência do adapter |
| `PowerEnergyCapability` | Conduzir o sensor, integrar energia e publicar |
| `SmartSysApp::addPowerEnergyCapability()` | Registro, ownership e rollback |

Contrato mínimo:

```cpp
enum class PowerMeasurementStatus {
    NOT_READY,
    VALID,
    ESTIMATED,
    INPUT_INVALID
};

struct PowerMeasurement {
    std::optional<double> powerW;
    PowerMeasurementStatus measurementStatus{PowerMeasurementStatus::NOT_READY};
};

struct IPowerSensor : public IHardwareAdapter {
    virtual const PowerMeasurement &powerMeasurement() const = 0;
};

class CompositePowerSensor : public IPowerSensor {
public:
    CompositePowerSensor(
        IVoltageSensor &voltageSensor,
        ICurrentSensor &currentSensor,
        std::uint32_t readingIntervalMs = 1000);
};
```

## 5. Requisitos

### 5.1 Contrato comum de potência

- **PWR-001:** `IPowerSensor` deve derivar de `IHardwareAdapter` e expor
  `const PowerMeasurement& powerMeasurement() const`.
- **PWR-002:** `PowerMeasurement` deve conter `std::optional<double> powerW` e
  `PowerMeasurementStatus measurementStatus`.
- **PWR-003:** `powerMeasurement()` devolve referência ao último snapshot
  estável e não adquire hardware, conduz outro adapter nem altera o snapshot.
- **PWR-004:** `powerW` presente deve ser finito e não negativo. Violação deve
  produzir `INPUT_INVALID` na capability, sem publicação ou integração do
  valor.
- **PWR-005:** `NOT_READY` e `INPUT_INVALID` não possuem `powerW`; `VALID` e
  `ESTIMATED` possuem `powerW`.
- **PWR-006:** `lastStateReadMillis()` inicia em zero e representa a última
  avaliação concluída pelo sensor. A capability não o usa para inferir
  staleness nesta versão.

### 5.2 PowerEnergyCapability e lifecycle

- **PWR-007:** `PowerEnergyCapability` deve receber `IPowerSensor&` não
  proprietário, vivo durante toda a vida da capability.
- **PWR-008:** `setup()` deve chamar exatamente uma vez `setup()` no sensor de
  potência e depois reinicializar o estado próprio.
- **PWR-009:** cada `handle()` da capability deve chamar exatamente uma vez
  `handle()` no sensor, inclusive quando a avaliação ainda não for elegível.
- **PWR-010:** a primeira avaliação ocorre imediatamente no primeiro `handle()`
  após `setup()`; as seguintes respeitam `readingIntervalMs` e rollover.
- **PWR-011:** cada avaliação elegível obtém exatamente uma vez o snapshot de
  `powerMeasurement()`, sem aquisição extra.
- **PWR-012:** estados do sensor mapeiam para os estados homônimos da medição de
  energia. Combinação incoerente de valor e estado, ausência de valor exigido,
  não finitude ou potência negativa produz `INPUT_INVALID`.
- **PWR-013:** o type permanece exatamente `"Power Energy (W/Wh)"`, exposto
  como `POWER_ENERGY_TYPE`.

### 5.3 CompositePowerSensor

- **PWR-014:** `CompositePowerSensor` recebe `IVoltageSensor&` e
  `ICurrentSensor&` não proprietários, vivos durante toda sua vida.
- **PWR-015:** `setup()` limpa somente snapshot, timestamp e controle interno;
  não chama nem verifica o setup dos sensores subordinados.
- **PWR-016:** `handle()` não chama `handle()` nos sensores, solicita aquisição
  ou usa timestamps para inferir atualização.
- **PWR-017:** cada oportunidade elegível consome uma vez o último snapshot de
  tensão e uma vez o de corrente e produz um snapshot de potência.
- **PWR-018:** tensão `NOT_READY`, corrente `NOT_READY`/`CALIBRATING` ou
  alimentação `UNKNOWN` produz `NOT_READY`.
- **PWR-019:** tensão `BELOW_MINIMUM`/`ADC_SATURATION`, corrente
  `ZERO_CALIBRATION_FAILED`/`OUT_OF_CALIBRATED_RANGE`/
  `OVERCURRENT_OR_SATURATION`, alimentação `SUPPLY_OUT_OF_RANGE`, ausência de
  valor exigido ou número não finito produz `INPUT_INVALID`.
- **PWR-020:** tensão `VALID`, corrente `VALID`, alimentação `IN_RANGE` e
  valores finitos produzem `VALID`. Corrente `ESTIMATED` ou alimentação
  `NOT_MONITORED` produz `ESTIMATED` quando os valores são numéricos.
- **PWR-021:** `INPUT_INVALID` precede `NOT_READY`, que precede `ESTIMATED` e
  `VALID`.
- **PWR-022:** em `VALID` ou `ESTIMATED`, a potência é
  `abs(voltageV × currentA)`; corrente negativa não gera potência negativa.
- **PWR-023:** a primeira avaliação ocorre no primeiro `handle()` após
  `setup()`; as demais respeitam intervalo com default `1000 ms`. Intervalo
  zero é inválido.

### 5.4 INA3221PowerSensor

- **PWR-024:** `INA3221PowerSensor` implementa `IPowerSensor`, recebe
  `INA3221Device&` não proprietário e copia sua configuração.
- **PWR-025:** a configuração representa, no mínimo:

```cpp
struct INA3221PowerSensorConfig {
    std::uint8_t channel{3};
    float shuntResistanceOhms{0.0f};
    float polarity{1.0f};
    float deadbandA{0.0f};
    float minimumReportableA{0.0f};
    float maximumAbsoluteCurrentA{0.0f};
    float minimumVoltageV{0.0f};
    float maximumVoltageV{26.0f};
    std::uint32_t readingIntervalMs{500};
};

class INA3221PowerSensor : public IPowerSensor {
public:
    INA3221PowerSensor(
        INA3221Device &device,
        const INA3221PowerSensorConfig &config);
};
```

- **PWR-026:** canal deve estar entre 0 e 2; intervalo e shunt devem ser
  positivos; polaridade aceita somente `+1.0f` ou `-1.0f`; limites devem ser
  finitos, ordenados e preservar os limites de conversão de
  `IOTSSC-INA3221-SENSORS@0.2`.
- **PWR-027:** `setup()` limpa o snapshot e solicita setup idempotente do
  dispositivo e configuração consistente do shunt. Configuração inválida,
  dispositivo indisponível ou conflito de shunt mantém `NOT_READY` e produz
  diagnóstico sem bloquear o runtime.
- **PWR-028:** cada oportunidade elegível obtém uma tensão de barramento e uma
  corrente do mesmo canal através de `INA3221Device`, sem expor o driver.
- **PWR-029:** valor não finito ou dispositivo indisponível produz `NOT_READY`.
  Tensão fora da faixa ou corrente acima do máximo produz `INPUT_INVALID`.
- **PWR-030:** corrente abaixo de `deadbandA` é normalizada para zero; entre o
  deadband e `minimumReportableA` preserva magnitude. Ambas produzem
  `ESTIMATED`.
- **PWR-031:** a potência numérica é
  `abs(busVoltage × polarity × currentAmps)`. Como a alimentação do chip não é
  monitorada, toda leitura de outro modo válida permanece `ESTIMATED`.
- **PWR-032:** o adapter executa no máximo uma avaliação por oportunidade, sem
  `delay()`, espera ativa ou loop de amostragem, e tolera rollover.

### 5.5 Configuração, API e compatibilidade

- **PWR-033:** `PowerEnergyConfig` preserva `id` sem default válido e
  `readingIntervalMs` com default `1000 ms`.
- **PWR-034:** deve existir o overload principal:

```cpp
PowerEnergyCapability *
SmartSysApp::addPowerEnergyCapability(
    PowerEnergyConfig config,
    IPowerSensor &powerSensor);
```

- **PWR-035:** no overload principal, `SmartSysApp` possui somente a
  capability; o sensor externo permanece sob ownership da aplicação, embora
  seu lifecycle seja conduzido pela capability.
- **PWR-036:** a assinatura vigente permanece pública e compatível:

```cpp
PowerEnergyCapability *
SmartSysApp::addPowerEnergyCapability(
    PowerEnergyConfig config,
    IVoltageSensor &voltageSensor,
    ICurrentSensor &currentSensor);
```

- **PWR-037:** o overload de compatibilidade cria um `CompositePowerSensor`
  pertencente à aplicação e uma capability que o referencia. Rejeição restaura
  slot, arena, adapter e identidade sem efeito parcial.
- **PWR-038:** capability e composite não conduzem os sensores de tensão e
  corrente recebidos pelo overload de compatibilidade.
- **PWR-039:** identidade inválida ou duplicada, intervalo zero, registro
  tardio, falta de slot ou arena retorna `nullptr`, registra a causa e não
  produz efeito parcial.
- **PWR-040:** ambos os overloads devolvem ponteiro não proprietário e estável
  para a capability durante a vida da aplicação.

### 5.6 Potência, energia e publicação preservadas

- **PWR-041:** energia inicia em `0 Wh` e é reinicializada por `setup()` ou
  `resetEnergy()`; não é persistida.
- **PWR-042:** a primeira avaliação utilizável estabelece a baseline sem
  acrescentar energia. Entre avaliações utilizáveis consecutivas:

```text
deltaEnergyWh = ((previousPowerW + powerW) / 2)
                × elapsedMs / 3600000
energyWh = energyWh + deltaEnergyWh
```

- **PWR-043:** `NOT_READY` ou `INPUT_INVALID` não acrescenta energia e elimina
  a baseline; a próxima leitura não recupera o intervalo.
- **PWR-044:** `ESTIMATED` pode acumular energia sem promoção a `VALID`.
  `resetEnergy()` zera energia e baseline sem chamada adicional ao sensor.
- **PWR-045:** potência ou integração não finita preserva energia finita já
  acumulada, elimina a baseline e produz `INPUT_INVALID`.
- **PWR-046:** permanecem públicos `powerEnergyMeasurement()` e
  `resetEnergy()`, com `powerW`, `energyWh` e `measurementStatus`.
- **PWR-047:** potência usa duas casas, energia três, ponto independente de
  locale e zero negativo normalizado; estado inválido publica valor vazio.
- **PWR-048:** `CapabilityStateChanged`, `energyWh`, `measurementStatus`,
  ausência de `supplyStatus` e compatibilidade byte a byte de eventos sem
  energia permanecem inalterados.
- **PWR-049:** a primeira avaliação publica; depois, somente mudança na
  representação de valor, estado ou energia produz evento.

## 6. Fluxos e condições de borda

```text
registro antes de SmartSysApp::setup()
→ setup da capability conduz setup do IPowerSensor e zera energia
→ cada handle conduz uma vez o IPowerSensor
→ avaliação elegível consome um PowerMeasurement
→ potência utilizável estabelece ou avança integração trapezoidal
→ potência indisponível/inválida rompe a baseline
→ publicação ocorre somente quando a representação muda
```

No overload de compatibilidade, o builder cria composite e capability
atomicamente; a aplicação externa continua conduzindo tensão e corrente; a
capability conduz somente o composite, que calcula `abs(V × I)` dos snapshots.

Condições de borda:

- sensor externo nunca acionado pode manter o composite em `NOT_READY`;
- snapshots antigos podem continuar sendo compostos, pois não há staleness;
- tensão ou corrente zero válida produz `0.00 W`;
- reset entre avaliações elimina a baseline anterior;
- conflito de shunt não pode ser resolvido silenciosamente;
- falha de criação não pode deixar adapter órfão nem consumir capacidade;
- cadências do sensor e da integração podem diferir; integra-se o snapshot
  disponível em cada avaliação elegível.

## 7. Critérios de aceite e validações

### PWR-AC-001 — Contrato e lifecycle

**Cobre:** PWR-001 a PWR-013.

- double controlado comprova uma chamada de `setup()`, uma chamada de
  `handle()` por ciclo e nenhuma aquisição por `powerMeasurement()`;
- primeira avaliação, cadência e combinações coerentes ou incoerentes de
  valor/estado produzem os resultados contratados;
- **meio:** teste PlatformIO/Unity e inspeção.

### PWR-AC-002 — Composição

**Cobre:** PWR-014 a PWR-023 e PWR-038.

- doubles reproduzem a matriz de estados vigente;
- `24 V` com `2 A` ou `-2 A` produz `48 W`;
- contadores confirmam zero chamadas de lifecycle aos sensores subordinados;
- **meio:** teste PlatformIO/Unity com provider de tempo.

### PWR-AC-003 — INA3221PowerSensor

**Cobre:** PWR-024 a PWR-032.

- configuração válida produz potência por magnitude e estado `ESTIMATED`;
- indisponibilidade, não finitude, faixa, sobrecorrente, deadband, intervalo,
  canal e conflito de shunt produzem os resultados contratados;
- nenhuma API inexistente de potência do driver Adafruit é requerida;
- **meio:** teste PlatformIO/Unity com seam ou double do dispositivo e build
  Arduino/ESP32.

### PWR-AC-004 — API, ownership e rollback

**Cobre:** PWR-033 a PWR-040.

- overload novo registra sensor externo sem assumir ownership;
- overload anterior continua compilando e cria um composite da aplicação;
- falhas não deixam efeito parcial;
- destruir a aplicação destrói capability e composite interno, mas não
  sensores externos;
- **meio:** teste PlatformIO/Unity, inspeção e build.

### PWR-AC-005 — Integração e publicação

**Cobre:** PWR-041 a PWR-049.

- baseline de `100 W`, após `3600 ms` seguida por `200 W`, acrescenta
  `0,150 Wh`;
- estado inválido rompe baseline, e reset zera energia sem chamada extra;
- formatação, supressão e serialização preservam o contrato 0.3;
- **meio:** teste PlatformIO/Unity com tempo e sink controlados.

### PWR-AC-006 — Builds e consumidores preservados

**Cobre:** PWR-013, PWR-034 a PWR-040 e o recorte compilável.

- `pio run -e esp32_dev` termina com sucesso;
- `pio run -e example_power_energy_mcb_r1` termina com sucesso pelo overload
  compatível;
- `pio run -e ESP32_MCB01` termina com sucesso sem migração da aplicação;
- **meio:** builds PlatformIO e inspeção de símbolos.

### PWR-AC-007 — Testes automatizados

**Cobre:** PWR-001 a PWR-049 por PWR-AC-001 a PWR-AC-005.

- criar ou ampliar grupo PlatformIO/Unity dedicado à abstração de potência;
- cobrir sucesso, falha, ausência de evidência, lifecycle, ownership, rollback
  e bordas numéricas;
- compilação e execução dos casos são evidências distintas;
- **meio:** inspeção, compilação sem upload e, mediante ordem operacional,
  execução em environment de teste compatível.

### 7.1 Testes e permissões

Por decisão explícita do Arquiteto, a criação ou ampliação dos testes descritos
integra esta versão e deve manter rastreabilidade. A futura ordem de
implementação autoriza criar esses artefatos, mas não executá-los. Compilação
sem upload, execução, upload, monitor e hardware permanecem operações
distintas; quando não autorizadas, devem constar como `Not Executed`.

## 8. Conhecimento afetado

- atualizar índice, fronteira, árvore e diagrama em
  `docs/rfc/KNOWLEDGE-MAP.md`;
- registrar a autoria 0.4 em `docs/rfc/EKOM-CHANGELOG.md`;
- encaminhar esta versão para Análise de Implementabilidade formal;
- preservar a versão 0.3 como baseline histórica validada.

## 9. Relações, decisões e pendências

**Fatos observados:** a versão 0.3 recebe diretamente sensores de tensão e
corrente, classifica estados e calcula `abs(V × I)`. A biblioteca Adafruit
INA3221 1.0.1 expõe tensão de barramento, tensão de shunt e corrente calculada,
mas não potência. `INA3221Device` já encapsula driver, setup idempotente, shunt,
tensão e corrente. O builder possui arenas e rollback para capabilities e
adapters.

**Decisões confirmadas pelo Arquiteto:** introduzir `IPowerSensor`; criar
`CompositePowerSensor` e `INA3221PowerSensor`; conduzir o `IPowerSensor` pela
capability; preservar o overload anterior; criar testes automatizados.

**Decisões preservadas:** potência por magnitude, energia volátil, integração
trapezoidal, intervalos inválidos não recuperados, default `1000 ms`, type e
publicação da versão 0.3.

**Autoridades confrontadas:** `AGENTS.md`, `docs/rfc/EKOM-GUIDELINES.md`, mapa
de conhecimento, `IOTSSC-PUBLIC-API`, `IOTSSC-RUNTIME`, contratos de corrente e
tensão, `IOTSSC-INA3221-SENSORS@0.2`, versão 0.3 desta fonte,
`IOTSSC-HW-EXAMPLES@1.1` e `IOTSSC-RUNTIME-CAPABILITY-CAPACITY@0.2`.

**Relação de autoridade:** a versão altera cálculo e lifecycle internos da
versão 0.3, preserva sua API e cria overload aditivo. Emenda a exclusão de
potência do contrato INA3221 0.2 somente para acrescentar adapter calculado em
software; não reinterpreta o chip como fonte direta. Exemplos e aplicação MCB01
permanecem sob suas autoridades e fora da migração.

**ADRs relacionadas:** nenhuma conhecida. A abstração permanece local ao
domínio de potência e segue Contracts/Core/Platform. A análise deve
reclassificar se encontrar impacto transversal material.

**Lacunas e débitos:** nenhum aceito na autoria. Rollback conjunto, seam de
teste do dispositivo e custo de arena são questões de implementabilidade, não
decisões funcionais antecipadas.

## 10. Estado da especificação

A versão 0.4 está Vigente [`Active`], com implementação Validada [`Validated`],
entrega Concluída [`Done`] e Análise de Implementabilidade Pronta [`Ready`]. A
revisão confrontou 49 requisitos, 7 critérios e zero débitos relacionados e
classificou o recorte como aderente com limitação de evidência documental. O
Arquiteto declarou testes e validação executados, considerou o resultado
suficiente e determinou o encerramento. A promoção foi concluída por
fast-forward e sincronizada em `main`.
