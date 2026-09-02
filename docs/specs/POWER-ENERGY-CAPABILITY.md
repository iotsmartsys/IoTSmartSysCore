# Especificação — PowerEnergyCapability

**ID:** `IOTSSC-POWER-ENERGY-CAPABILITY`

**Classe da fonte:** Normativa

**Versão:** 0.2

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Não pronta [`Not Ready`]

**Revisão de implementabilidade:** Pendente [`Pending Review`]

**Bloqueio arquitetural:** Nenhum conhecido antes da análise formal

**Relações normativas e de dependência:**

- Nova [`New`] — `PowerEnergyCapability`, `PowerEnergyConfig`, tipos de
  medição, API pública e campo opcional de energia no evento;
- Corrige [`Corrects`] a versão 0.1 ao contratar o exemplo executável omitido;
- Depende de `IOTSSC-CURRENT-SENSOR@0.6` e
  `IOTSSC-VOLTAGE-SENSOR@0.1` somente quanto aos contratos públicos de
  `ICurrentSensor`, `IVoltageSensor` e suas últimas medições;
- Preserva `IOTSSC-PUBLIC-API`, `IOTSSC-RUNTIME` e
  `IOTSSC-HW-EXAMPLES` por extensão aditiva e consumo do catálogo vigente.

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
- exemplo executável `power_energy` na MCB R1, com sensores externamente
  possuídos e conduzidos pelo próprio exemplo.

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
- novo Hardware Adapter, factory de sensor ou política geral de arbitragem de
  pinos;
- exemplo em placa diferente da MCB R1 ou demonstração dos sensores associados
  simultaneamente às capabilities próprias;
- alteração dos valores ou estados publicados pelas capabilities de corrente e
  tensão existentes;
- alteração de API pública para facilitar o exemplo;
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

### 5.7 Exemplo executável

O exemplo é consumidor da API pública e demonstra explicitamente o cenário em
que os sensores não estão associados a `CurrentSensorCapability` ou
`VoltageSensorCapability`. Ele herda integralmente
`IOTSSC-HW-EXAMPLES` e não altera o comportamento da capability.

- **PWR-032:** o catálogo executável deve conter o exemplo `power_energy`,
  composto por `examples/executable/power_energy/example.hpp`, seu `README.md`,
  um seletor mutuamente exclusivo em `src/ExecutableExampleRunner.cpp` e o
  environment `example_power_energy_mcb_r1` em
  `configs/executable_examples.ini`. O catálogo em `examples/README.md` deve
  identificá-lo. O build padrão e os exemplos preexistentes permanecem
  inalterados.
- **PWR-033:** o exemplo deve usar `SmartSysApp::addPowerEnergyCapability()`,
  `PowerEnergyConfig`, `PowerEnergyCapability::powerEnergyMeasurement()` e
  `PowerEnergyCapability::resetEnergy()`. Os sensores devem ser instâncias
  externas de `ACS712C30ACurrentSensor` e
  `ResistiveDividerVoltageSensor`, recebidas pela capability como
  `ICurrentSensor&` e `IVoltageSensor&`. Nenhuma API pública deve ser criada ou
  alterada para facilitar o exemplo.
- **PWR-034:** o exemplo deve possuir os dois sensores durante toda a vida da
  capability, chamar uma vez `setup()` em cada sensor antes de
  `SmartSysApp::setup()` e, em toda iteração de `loop()`, chamar uma vez
  `handle()` em cada sensor antes de `SmartSysApp::handle()`. A
  `PowerEnergyCapability` continua proibida de propagar ou verificar essas
  chamadas.
- **PWR-035:** somente a `PowerEnergyCapability` deve ser registrada na
  `SmartSysApp` do exemplo. O exemplo não deve registrar
  `CurrentSensorCapability` nem `VoltageSensorCapability`, reimplementar
  aquisição, calibração, conversão, qualificação, cálculo de potência,
  integração ou cadência interna.
- **PWR-036:** o sinal do ACS712 deve usar
  `ITS_MCB01_J4_EXT_ADC`, que resolve GPIO 34, e o nó ADC do divisor de tensão
  deve usar `ITS_MCB01_J4_EXT_IO33`, autorizado pelo Arquiteto como segunda
  entrada analógica e correspondente a GPIO 33/ADC1. Literais numéricos e
  macros próprias de pino são proibidos no código e no environment; a ausência
  de qualquer símbolo deve causar erro de build compreensível.
- **PWR-037:** o sensor de corrente deve usar o perfil público
  `CurrentSensorConfig::ACS712_30A_3V3()`, sem `supplyMonitorAdcPin`. O sensor
  de tensão deve usar divisor de `330 kΩ/10 kΩ`, limiar ADC inferior de
  `144 mV` e os demais defaults públicos de `VoltageSensorConfig`. A capability
  deve usar `readingIntervalMs = 1000 ms`.
- **PWR-038:** o boot deve registrar o identificador do exemplo, a placa, os
  símbolos e GPIOs resolvidos, o perfil elétrico de corrente, os resistores e o
  limiar do divisor, as identidades dos sensores e da capability e o intervalo
  de leitura, sem expor segredos.
- **PWR-039:** em cadência de apresentação não inferior a
  `EXAMPLE_POWER_ENERGY_LOG_INTERVAL_MS`, definido pelo environment, o exemplo
  deve apresentar a última
  `PowerEnergyMeasurement`, mostrando potência com duas casas decimais quando
  presente, energia com três casas decimais e o token de
  `measurementStatus`. A apresentação não pode bloquear o ciclo cooperativo ou
  alterar a cadência da capability.
- **PWR-040:** `nullptr` retornado por `addPowerEnergyCapability()` deve ser
  tratado como falha observável, registrado e nunca desreferenciado.
- **PWR-041:** o comando local `r` ou `R` recebido pelo monitor serial deve
  demonstrar `resetEnergy()`, registrar a operação e não adicionar comando
  remoto de reset.
- **PWR-042:** como a alimentação do ACS712 não é monitorada, uma medição
  numérica de corrente com tensão válida deve resultar em potência e energia
  com estado composto `ESTIMATED`. O exemplo e seu README não podem apresentar
  `VALID` nem afirmar exatidão contratada nessa condição.
- **PWR-043:** o `README.md` deve documentar objetivo, APIs e capability,
  ownership e lifecycle externos dos sensores, MCB R1, periféricos, tabela de
  pinos, esquema de ligação, configurações, comandos de build, upload e
  monitor, sequência manual, resultado esperado, reset local, limitações e
  riscos elétricos.

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

No exemplo `power_energy`, o lifecycle externo é concretizado assim:

```text
setup Arduino
→ setup do sensor de corrente
→ setup do sensor de tensão
→ registro da PowerEnergyCapability
→ SmartSysApp::setup()

cada loop Arduino
→ handle do sensor de corrente
→ handle do sensor de tensão
→ SmartSysApp::handle()
→ apresentação eventual do último snapshot composto
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

### PWR-AC-007 — Build e consumo público do exemplo

**Cobre:** PWR-032 a PWR-037 e PWR-040.

- `pio run -e example_power_energy_mcb_r1` alcança estado terminal com sucesso
  e vincula exatamente um `setup()` e um `loop()`;
- a inspeção confirma exatamente uma capability registrada, sensores externos
  vivos, ordem de lifecycle contratada e consumo da API pública sem nova API;
- código e environment usam `ITS_MCB01_J4_EXT_ADC` e
  `ITS_MCB01_J4_EXT_IO33`, sem literal ou macro própria de GPIO;
- ausência de símbolo obrigatório ou falha de registro é observável conforme o
  contrato;
- **meio:** build canônico do exemplo e inspeção.

### PWR-AC-008 — Documentação e apresentação

**Cobre:** PWR-038 a PWR-043.

- o `README.md` contém todos os elementos de PWR-043 e explicita que os
  adapters são possuídos e acionados pelo exemplo;
- boot e apresentação contêm os campos contratados, sem segredo e sem lógica
  de aquisição ou cálculo duplicada;
- o comando serial `r` registra o reset local e a leitura seguinte mostra
  energia reiniciada a `0.000 Wh` até nova integração elegível;
- **meio:** inspeção do firmware e da documentação e captura de monitor serial.

### PWR-AC-009 — Validação física do exemplo

**Cobre:** PWR-034, PWR-036, PWR-037, PWR-039, PWR-041 e PWR-042.

- gravado na MCB R1 com a montagem documentada, o exemplo permanece
  `NOT_READY` durante aquecimento e calibração do ACS712;
- após tensão e corrente numéricas, apresenta potência não negativa, energia
  não decrescente e estado `ESTIMATED`, pois a alimentação não é monitorada;
- variar a carga altera a potência observada, e `r` reinicia a energia sem
  interromper o lifecycle dos sensores;
- ausência ou invalidade de uma entrada produz o estado e a ausência de
  potência contratados, sem acumular o intervalo;
- **meio:** validação em hardware com instrumento independente.

### 7.1 Testes e permissões

Por decisão explícita do Arquiteto, nenhum artefato de teste automatizado deve
ser criado, ampliado, reestruturado ou corrigido nesta versão. Os meios
instrumentados descritos nos critérios são evidências de execução, não
autorização para registrar harness ou suíte persistente. Execução instrumentada,
captura de monitor, upload ou validação em hardware exige ordem operacional
própria; enquanto ausente, permanece `Not Executed`.

## 8. Conhecimento afetado

- atualizar esta especificação para 0.2 no índice e na cobertura de
  capabilities em `docs/rfc/KNOWLEDGE-MAP.md`;
- registrar no mapa o exemplo `power_energy`, sua pendência de implementação e
  a demonstração do lifecycle externo dos sensores;
- registrar a autoria da versão 0.2 em `docs/rfc/EKOM-CHANGELOG.md`;
- encaminhar a versão 0.2 para nova análise formal de implementabilidade.

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
artefato de teste deve ser implementado. Na versão 0.2, o exemplo usa
`ITS_MCB01_J4_EXT_ADC` para corrente e o símbolo oficial
`ITS_MCB01_J4_EXT_IO33`, expressamente autorizado como segunda entrada
analógica, para tensão.

**Decisões funcionais desta versão:** energia volátil; integração trapezoidal;
intervalos inválidos não são recuperados; potência é o valor escalar; energia é
campo opcional do evento; default de 1000 ms; type `Power Energy (W/Wh)`; o
exemplo demonstra adapters externos e reset local, sem capabilities próprias.

**Autoridades confrontadas:** `AGENTS.md`, `docs/rfc/EKOM-GUIDELINES.md`,
`docs/rfc/KNOWLEDGE-MAP.md`, `IOTSSC-PUBLIC-API`, `IOTSSC-RUNTIME`,
`IOTSSC-CURRENT-SENSOR@0.6`, `IOTSSC-VOLTAGE-SENSOR@0.1` e
`IOTSSC-HW-EXAMPLES@1.1`.

**Relação de autoridade:** a fonte permanece uma extensão aditiva [`New`] em
relação às APIs preexistentes. A versão 0.2 **Corrige** [`Corrects`] a versão
0.1 ao contratar o exemplo executável omitido, sem alterar cálculo, estados,
publicação ou lifecycle da capability. Preserva `IOTSSC-HW-EXAMPLES`: o exemplo
usa seu catálogo e sua exceção explícita para demonstrar os adapters de baixo
nível, sem alterar API pública para facilitá-lo.

**ADRs relacionadas:** nenhuma conhecida. A análise formal da versão 0.1
classificou como plausível o ownership externo; a versão 0.2 deve reconfrontar
sua concretização pelo exemplo e a seleção dos dois ADCs.

**Lacunas e débitos:** nenhuma lacuna normativa ou débito técnico foi aceito na
autoria. A implementação da versão 0.1 permanece como baseline histórica; o
exemplo da versão 0.2 e suas evidências ainda não existem. A implementabilidade
da versão corrente permanece pendente.

## 10. Estado da especificação

A versão 0.2 está em Rascunho [`Draft`], com implementação Não iniciada
[`Not Started`], entrega Não pronta [`Not Ready`] e revisão de
implementabilidade Pendente [`Pending Review`]. O código e o build canônico da
versão 0.1 permanecem como evidência histórica, mas não cobrem o exemplo agora
contratado nem autorizam sua implementação. A versão 0.2 segue para nova
análise formal. Nenhum artefato de teste integra o recorte e nenhum teste foi
criado, alterado ou executado nesta autoria.
