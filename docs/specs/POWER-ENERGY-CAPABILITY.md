# Especificação — PowerEnergyCapability

**ID:** `IOTSSC-POWER-ENERGY-CAPABILITY`

**Classe da fonte:** Normativa

**Versão:** 0.1

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Não iniciada [`Not Started`]

**Revisão de implementabilidade:** Pendente [`Pending Review`]

**Bloqueio arquitetural:** Nenhum conhecido antes da análise formal

**Relações normativas e de dependência:**

- Nova [`New`] — `PowerEnergyCapability`, `PowerEnergyConfig`, tipos de
  medição, API pública e campo opcional de energia no evento;
- Depende de `IOTSSC-CURRENT-SENSOR@0.6` e
  `IOTSSC-VOLTAGE-SENSOR@0.1` somente quanto aos contratos públicos de
  `ICurrentSensor`, `IVoltageSensor` e suas últimas medições;
- Preserva `IOTSSC-PUBLIC-API` e `IOTSSC-RUNTIME` por extensão aditiva.

## 1. Objetivo e contexto

Adicionar uma capability que componha as últimas medições disponibilizadas por
um `IVoltageSensor` e um `ICurrentSensor`, calcule potência ativa por magnitude
em watts e acumule energia em watt-hora.

A capability não é Hardware Adapter, não adquire amostras e não controla o
lifecycle dos sensores recebidos. A aplicação consumidora continua responsável
por chamar corretamente `setup()` e `handle()` em cada sensor, diretamente ou
por meio de `VoltageSensorCapability` e `CurrentSensorCapability`.

## 2. Escopo

- classe pública `PowerEnergyCapability` derivada de `ICapability`;
- referências não proprietárias a `IVoltageSensor` e `ICurrentSensor`;
- configuração pública `PowerEnergyConfig` com identidade e intervalo de
  leitura;
- cálculo de potência não negativa em watts;
- integração temporal e acumulação volátil de energia em Wh;
- estados próprios para prontidão, validade, estimativa e entrada inválida;
- acesso à última `PowerEnergyMeasurement` e operação `resetEnergy()`;
- publicação escalar da potência e campo opcional de energia no evento;
- registro aditivo pelo builder e por `SmartSysApp`;
- ownership da capability pela aplicação e ownership externo dos sensores;
- diagnóstico por `ILogger` para falhas do registro público.

## 3. Fora de escopo

- aquisição, configuração, calibração ou validação física de tensão ou
  corrente;
- chamada ou verificação de `setup()` e `handle()` dos sensores recebidos;
- detecção de leitura desatualizada ou imposição de cadência aos sensores;
- potência assinada, direção do fluxo, potência aparente ou reativa, fator de
  potência, fase, frequência, forma de onda ou RMS;
- persistência, restauração ou sincronização externa da energia acumulada;
- tarifa, custo, demanda, janela histórica ou agregações por período;
- comando remoto para zerar energia;
- novo Hardware Adapter, factory de sensor, GPIO, arbitragem de pino ou exemplo
  executável em hardware;
- alteração dos valores ou estados publicados pelas capabilities de corrente e
  tensão existentes;
- alteração do limite de oito capabilities ou do lifecycle cooperativo;
- criação, ampliação, reestruturação ou correção de artefatos de teste.

### 3.1 Arquitetura e organização

**Precedente geral:** `CurrentSensorCapability`, quanto à derivação de
`ICapability`, intervalo configurável, primeira avaliação imediata, formatação
escalar, publicação por mudança, acesso à medição e registro público atômico.

**Diferença intencional:** `PowerEnergyCapability` somente consome snapshots.
Seu `setup()` e seu `handle()` não propagam chamadas aos sensores. Essa
separação evita duplicar o lifecycle quando os mesmos sensores já são acionados
por suas capabilities próprias.

**Elementos preservados:** separação Contracts/Core/App, configuração antes de
`SmartSysApp::setup()`, ownership em arena da capability, oito slots,
processamento cooperativo e compatibilidade pública aditiva.

### 3.2 Limite de escopo funcional

Não há preparação arquitetural independente conhecida. A composição usa as
interfaces públicas vigentes e atribui explicitamente à aplicação consumidora
o lifecycle e a duração das referências. A análise formal deve confrontar se o
registro de uma capability com adapters externamente possuídos é plausível no
builder vigente sem criar nova política transversal de ownership.

## 4. Componentes e tipos públicos

| Elemento | Responsabilidade |
|---|---|
| `PowerEnergyCapability` | Cadência, composição, integração temporal e publicação |
| `PowerEnergyConfig` | Identidade e intervalo de leitura |
| `PowerEnergyMeasurement` | Potência, energia acumulada e estado composto |
| `PowerEnergyMeasurementStatus` | `NOT_READY`, `VALID`, `ESTIMATED` e `INPUT_INVALID` |
| `POWER_ENERGY_TYPE` | Type público da capability |
| `CapabilityStateChanged::energyWh` | Energia formatada opcional no evento |
| `SmartSysApp::addPowerEnergyCapability()` | Registro, ownership da capability e falha atômica |

## 5. Requisitos

### 5.1 Contratos e lifecycle

- **PWR-001:** `PowerEnergyCapability` deve derivar de `ICapability` e receber
  `IVoltageSensor&` e `ICurrentSensor&` como referências não proprietárias.
- **PWR-002:** os dois sensores devem permanecer vivos durante toda a vida da
  capability. `SmartSysApp` possui somente a capability e não assume ownership
  dos sensores.
- **PWR-003:** `PowerEnergyCapability::setup()` não pode chamar `setup()` nos
  sensores, e `PowerEnergyCapability::handle()` não pode chamar `handle()` nos
  sensores.
- **PWR-004:** a capability não deve verificar se os sensores foram
  configurados, inicializados ou acionados. Deve consumir exclusivamente os
  resultados correntes de `voltageMeasurement()` e `currentMeasurement()`.
- **PWR-005:** o type público deve ser exatamente `"Power Energy (W/Wh)"`,
  exposto como `POWER_ENERGY_TYPE`.

### 5.2 Configuração e API

- **PWR-006:** `PowerEnergyConfig` deve conter, no mínimo:

```text
id
readingIntervalMs
```

- **PWR-007:** `readingIntervalMs` possui default de `1000 ms`; `id` não possui
  default válido. Intervalo igual a zero, identidade vazia, inválida ou já
  registrada impedem o registro sem efeito parcial.
- **PWR-008:** a API pública deve ser:

```cpp
PowerEnergyCapability *
SmartSysApp::addPowerEnergyCapability(
    PowerEnergyConfig config,
    IVoltageSensor &voltageSensor,
    ICurrentSensor &currentSensor);
```

- **PWR-009:** o registro deve ocorrer antes de `SmartSysApp::setup()`. O
  ponteiro retornado é não proprietário e estável durante a vida da aplicação;
  `nullptr` indica falha registrada por `ILogger` sem consumir slot, arena ou
  identidade.

### 5.3 Cadência e snapshots

- **PWR-010:** a primeira avaliação deve ocorrer imediatamente no primeiro
  `handle()` posterior ao `setup()` da capability. Avaliações seguintes só
  podem ocorrer depois de transcorrido `readingIntervalMs`.
- **PWR-011:** a cada avaliação elegível, a capability deve obter uma vez a
  referência à última `VoltageMeasurement` e uma vez a referência à última
  `CurrentMeasurement`, sem solicitar nova aquisição.
- **PWR-012:** a medição composta não deve inferir falha de lifecycle nem usar
  `lastStateReadMillis()` como verificação de atualização. O estado resulta
  somente dos valores e estados publicamente expostos pelos sensores.
- **PWR-013:** o controle de intervalo deve usar o provedor de tempo vigente e
  tolerar rollover sem bloquear permanentemente avaliações futuras.

### 5.4 Validade das entradas

- **PWR-014:** tensão `NOT_READY` ou corrente `NOT_READY`/`CALIBRATING` produz
  `NOT_READY`, sem potência, e interrompe a continuidade da integração.
- **PWR-015:** tensão `BELOW_MINIMUM`/`ADC_SATURATION`, corrente
  `ZERO_CALIBRATION_FAILED`/`OUT_OF_CALIBRATED_RANGE`/
  `OVERCURRENT_OR_SATURATION`, alimentação `SUPPLY_OUT_OF_RANGE`, ausência de
  valor exigido ou valor não finito produz `INPUT_INVALID`, sem potência, e
  interrompe a continuidade da integração.
- **PWR-016:** tensão `VALID`, corrente `VALID`, alimentação `IN_RANGE` e ambos
  os valores finitos produzem `VALID`.
- **PWR-017:** tensão `VALID` e corrente numérica `ESTIMATED` produzem
  `ESTIMATED`. Corrente numérica `VALID` com alimentação `NOT_MONITORED` também
  produz `ESTIMATED`. Alimentação `UNKNOWN` produz `NOT_READY`.
- **PWR-018:** quando mais de uma condição se aplicar, `INPUT_INVALID` tem
  precedência sobre `NOT_READY`, que tem precedência sobre `ESTIMATED` e
  `VALID`.

### 5.5 Potência e energia

- **PWR-019:** em `VALID` ou `ESTIMATED`, a potência deve ser:

```text
powerW = abs(voltageV × currentA)
```

  O sinal da corrente não pode produzir potência negativa.
- **PWR-020:** a energia acumulada deve iniciar em `0 Wh` na construção e ser
  reinicializada para `0 Wh` por `setup()` ou `resetEnergy()`. Não deve ser
  persistida.
- **PWR-021:** a primeira avaliação utilizável estabelece potência e instante
  de referência, sem acrescentar energia. Entre duas avaliações utilizáveis e
  consecutivas, a energia deve usar integração trapezoidal pelo tempo real:

```text
deltaEnergyWh = ((previousPowerW + powerW) / 2)
                × elapsedMs / 3600000
energyWh = energyWh + deltaEnergyWh
```

- **PWR-022:** `NOT_READY` ou `INPUT_INVALID` não acrescenta energia e elimina
  a referência anterior. A primeira avaliação utilizável posterior estabelece
  nova referência sem recuperar o intervalo indisponível.
- **PWR-023:** `ESTIMATED` pode acumular energia, mas o estado deve permanecer
  `ESTIMATED`; a energia histórica não deve ser descartada quando o estado
  voltar a `VALID`.
- **PWR-024:** `resetEnergy()` deve zerar a energia e eliminar a referência
  temporal. A chamada não deve alterar nem acionar os sensores.
- **PWR-025:** resultado de potência ou integração não finito deve produzir
  `INPUT_INVALID`, preservar a energia finita já acumulada e eliminar a
  referência anterior.

### 5.6 Métodos e publicação

- **PWR-026:** a capability deve expor:

```cpp
const PowerEnergyMeasurement &powerEnergyMeasurement() const;
void resetEnergy();
```

- **PWR-027:** `PowerEnergyMeasurement` deve conter
  `std::optional<double> powerW`, `double energyWh` e
  `PowerEnergyMeasurementStatus measurementStatus`.
- **PWR-028:** em `VALID` ou `ESTIMATED`, `ICapability::value` deve conter a
  potência em watts com exatamente duas casas decimais, ponto independente de
  locale e zero negativo normalizado. Em `NOT_READY` ou `INPUT_INVALID`, deve
  conter `""`.
- **PWR-029:** `CapabilityStateChanged` deve receber o campo público opcional
  `energyWh`. Eventos desta capability devem preencher `measurementStatus` com
  o token exato do estado e `energyWh` com a energia não negativa usando
  exatamente três casas decimais. `supplyStatus` permanece ausente.
- **PWR-030:** a extensão de evento e sua serialização devem ser aditivas:
  eventos sem `energyWh` permanecem byte a byte inalterados.
- **PWR-031:** a primeira avaliação deve publicar. Depois dela, um evento deve
  ser emitido somente quando mudar a representação publicada de `value`,
  `measurementStatus` ou `energyWh`; o timestamp de avaliação deve avançar
  mesmo sem evento.

## 6. Fluxo e condições de borda

```text
registro antes de SmartSysApp::setup()
→ aplicação inicializa e aciona os sensores por responsabilidade própria
→ setup da PowerEnergyCapability zera energia sem tocar nos sensores
→ primeira avaliação lê os dois snapshots e publica imediatamente
→ entradas utilizáveis: |V × I| e baseline temporal
→ próxima entrada utilizável consecutiva: integração trapezoidal em Wh
→ entrada indisponível ou inválida: sem potência, sem energia nova e sem ponte
→ resetEnergy(): energia zero e nova baseline na próxima entrada utilizável
```

Condições de borda:

- sensores nunca acionados podem permanecer indefinidamente em `NOT_READY`;
- leituras antigas podem continuar sendo usadas e integradas, pois atualização
  ou staleness não é verificada por esta versão;
- corrente negativa produz a mesma magnitude de potência que corrente positiva;
- corrente ou tensão zero válida produz `0.00 W` e não aumenta a energia;
- período inválido não é interpolado nem recuperado;
- reset entre avaliações descarta a baseline anterior;
- arredondamento da publicação não altera o acumulador interno;
- intervalo zero e registro tardio falham atomicamente.

## 7. Critérios de aceite e validações

### PWR-AC-001 — Magnitude da potência

**Cobre:** PWR-014 a PWR-019 e PWR-028.

- snapshots válidos de `24,00 V` e `2,00 A` produzem `48.00` W;
- `24,00 V` e `−2,00 A` também produzem `48.00` W;
- entrada inválida ou indisponível produz valor vazio e o estado contratado;
- **meio:** inspeção e execução instrumentada com sensores controlados, sem
  criar artefato de teste.

### PWR-AC-002 — Acumulação temporal

**Cobre:** PWR-020 a PWR-025.

- uma baseline de `100 W`, seguida após `3600 ms` por `200 W`, acrescenta
  exatamente `0,150 Wh` pelo método trapezoidal;
- a primeira leitura, um intervalo inválido e a primeira leitura após esse
  intervalo não acrescentam energia;
- resultado não finito preserva a energia anterior e rompe a baseline;
- **meio:** execução instrumentada com provider de tempo e snapshots
  controlados, sem criar artefato de teste.

### PWR-AC-003 — Lifecycle externo

**Cobre:** PWR-001 a PWR-004 e PWR-010 a PWR-012.

- contadores instrumentados confirmam que `setup()` e `handle()` da capability
  não chamam os métodos equivalentes dos sensores;
- sensores nunca acionados são apenas consumidos no estado que expuserem, sem
  rejeição, WARN ou inferência de configuração incorreta;
- **meio:** inspeção e execução instrumentada, sem criar artefato de teste.

### PWR-AC-004 — Cadência e reset

**Cobre:** PWR-007, PWR-010, PWR-013, PWR-020 e PWR-024.

- com `readingIntervalMs = 250`, a primeira avaliação é imediata e nenhuma
  avaliação posterior ocorre antes de 250 ms;
- `resetEnergy()` zera a energia, não toca nos sensores e obriga nova baseline;
- intervalo zero é rejeitado sem efeito parcial;
- **meio:** execução instrumentada e inspeção.

### PWR-AC-005 — Publicação aditiva

**Cobre:** PWR-005 e PWR-026 a PWR-031.

- o type é `Power Energy (W/Wh)`, a potência usa duas casas e a energia três;
- repetição da mesma representação é suprimida e mudança em potência, energia
  formatada ou estado produz exatamente um evento;
- evento preexistente sem `energyWh` mantém serialização byte a byte idêntica;
- **meio:** inspeção e captura instrumentada do sink.

### PWR-AC-006 — API, ownership e compatibilidade

**Cobre:** PWR-001, PWR-002 e PWR-006 a PWR-009.

- registro válido antes de `setup()` devolve ponteiro estável e não proprietário;
- identidade inválida, duplicada, slot ou arena indisponível e registro tardio
  retornam `nullptr` sem efeito parcial;
- destruir a aplicação destrói a capability, mas não destrói os sensores;
- `pio run -e esp32_dev` alcança estado terminal com sucesso;
- **meio:** inspeção, execução instrumentada e build canônico.

### 7.1 Testes e permissões

Por decisão explícita do Arquiteto, nenhum artefato de teste automatizado deve
ser criado, ampliado, reestruturado ou corrigido nesta versão. Os meios
instrumentados descritos nos critérios são evidências de execução, não
autorização para registrar harness ou suíte persistente. Execução instrumentada
ou em hardware exige ordem operacional própria; enquanto ausente, permanece
`Not Executed`.

## 8. Conhecimento afetado

- adicionar esta especificação ao índice e à cobertura de capabilities em
  `docs/rfc/KNOWLEDGE-MAP.md`;
- registrar no mapa que `PowerEnergyCapability` somente consome snapshots, não
  controla nem verifica o lifecycle dos sensores e mantém referências não
  proprietárias sob responsabilidade da aplicação;
- registrar a autoria em `docs/rfc/EKOM-CHANGELOG.md`;
- encaminhar a versão 0.1 para análise formal de implementabilidade.

## 9. Relações, decisões e pendências

**Fatos observados:** `CurrentSensorCapability` estabelece o precedente de
cadência, publicação e registro atômico, mas aciona seu sensor em `setup()` e
`handle()`. As interfaces `ICurrentSensor` e `IVoltageSensor` expõem snapshots
estáveis e derivam de `IHardwareAdapter`. O evento vigente comporta campos
opcionais aditivos de medição.

**Decisões confirmadas pelo Arquiteto:** cálculo de potência e energia
acumulada; potência somente por magnitude positiva; sensores podem já estar ou
não associados às capabilities próprias; lifecycle permanece externo e não é
obrigado nem verificado; essa fronteira integra o mapa de conhecimento; nenhum
artefato de teste deve ser implementado.

**Decisões funcionais desta versão:** energia volátil; integração trapezoidal;
intervalos inválidos não são recuperados; potência é o valor escalar; energia é
campo opcional do evento; default de 1000 ms; type `Power Energy (W/Wh)`.

**Autoridades confrontadas:** `AGENTS.md`, `docs/rfc/EKOM-GUIDELINES.md`,
`docs/rfc/KNOWLEDGE-MAP.md`, `IOTSSC-PUBLIC-API`, `IOTSSC-RUNTIME`,
`IOTSSC-CURRENT-SENSOR@0.6` e `IOTSSC-VOLTAGE-SENSOR@0.1`.

**Relação de autoridade:** a versão é uma extensão aditiva [`New`]. Não altera
aquisição, estados ou lifecycle normativos dos sensores existentes; governa
somente sua composição, a nova API e o campo opcional `energyWh`.

**ADRs relacionadas:** nenhuma conhecida. A análise formal deve classificar a
restrição de ownership externo antes de recomendar prontidão.

**Lacunas e débitos:** nenhuma lacuna normativa ou débito técnico foi aceito na
autoria. A implementabilidade permanece pendente.

## 10. Estado da especificação

A versão 0.1 está em Rascunho [`Draft`], com implementação e entrega não
iniciadas. O contrato foi confirmado e registrado por ordem explícita do
Arquiteto e segue para Análise de Implementabilidade. Nenhuma classificação
`Ready`, autorização de implementação ou conclusão é declarada nesta atuação.
