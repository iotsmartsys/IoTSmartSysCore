# Histórico de mudanças EKOM — IoTSmartSysCore

Este arquivo registra transações iniciadas sob EKOM 4.6. O histórico anterior
permanece preservado em `docs/rfc/EKM-CHANGELOG.md`.

## EKOM-CHG-0011 — Autoria da PowerEnergyCapability 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-POWER-ENERGY-CAPABILITY@0.1`

**Objetivo:** registrar o contrato da `PowerEnergyCapability`, que compõe um
`IVoltageSensor` e um `ICurrentSensor` para publicar potência por magnitude e
energia acumulada em Wh, sem controlar o lifecycle dos sensores.

### Decisões relacionadas

- a capability recebe referências não proprietárias aos dois sensores e não
  chama nem verifica seus métodos `setup()` e `handle()`;
- a aplicação consumidora responde pela atualização e pela duração das
  referências, estejam os sensores ou não associados às capabilities próprias;
- potência é `abs(V × I)` e energia volátil usa integração trapezoidal pelo
  tempo real entre avaliações utilizáveis consecutivas;
- o intervalo de leitura é configurável na instanciação e possui default de
  1000 ms;
- potência ocupa o valor escalar e energia ocupa campo opcional próprio no
  evento;
- nenhum artefato de teste integra a versão por decisão explícita do Arquiteto.

### Lacunas

- nenhuma lacuna normativa conhecida foi aberta na autoria; a compatibilidade
  do ownership externo com o builder vigente deve ser confrontada pela análise
  formal de implementabilidade.

### Débitos técnicos relacionados

- nenhum débito técnico foi aceito nesta transação.

### Relatórios e evidências materiais

- investigação dirigida de `CurrentSensorCapability`, `VoltageSensorCapability`,
  interfaces, medições, evento, builder, API pública, lifecycle, mapa e
  especificações relacionadas;
- rascunho conversacional reconciliado e ordem explícita do Arquiteto para o
  registro, incluindo a proibição de implementar testes;
- integridade textual aprovada sobre o delta documental; a guarda estrutural
  foi executada e permaneceu reprovada somente por mapas experimentais,
  relatórios históricos e seções da especificação de console preexistentes e
  fora deste recorte.

### Resultado

`IOTSSC-POWER-ENERGY-CAPABILITY@0.1` foi registrada em `Draft`, com análise de
implementabilidade pendente, implementação não iniciada e sem bloqueio
arquitetural conhecido antes da análise formal. O mapa registra explicitamente
o lifecycle externo dos sensores. Nenhum código, teste, build ou configuração
funcional foi alterado.

## EKOM-CHG-0010 — Validação final da FanCapability 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-FAN-CAPABILITY@0.1`

**Objetivo:** registrar a revisão e a validação em hardware declaradas pelo
Arquiteto, encerrar a especificação e integrar o recorte validado à `main`.

### Evidência humana recebida

O Arquiteto confirmou em ordem direta que a implementação foi revisada e
validada em hardware e determinou o encerramento da especificação, o merge e o
push para `main`. A decisão considera suficiente o conjunto FAN-AC-001 a
FAN-AC-005.

### Confrontação consultiva

O Consultor de Arquitetura, sem alegação de independência por ter participado
das etapas anteriores, confrontou especificação, implementação, relatórios e
composição Git sem identificar conflito arquitetural ou normativo bloqueante.
Os ensaios físicos não foram reexecutados e seus registros brutos não foram
recebidos pelo Consultor.

O build canônico reprovado durante a Implementação permanece registrado com
seu resultado original e não é convertido em sucesso. A alteração preexistente
e aditiva de `ITS_MCB01_LED_PIN` presente na branch não altera requisitos nem
aceite de `IOTSSC-FAN-CAPABILITY@0.1`.

### Promoções

- estado normativo: `Draft` → Vigente [`Active`];
- estado da implementação: `In Progress` → Validada [`Validated`];
- estado da entrega: → Pronta para integração [`Ready for Integration`].

O relatório
`docs/reports/2026-08-30T033210Z-0.1-39da00a3-final-validation-report.md`
preserva a decisão, a confrontação e seus limites. A transação permanece em
andamento até a integração e sincronização efetivas com `main`.

### Integração e encerramento

O recorte validado foi integrado à `main` a partir da branch
`spec/fan-capability`, sem conflito. A entrega passa de Pronta para integração
[`Ready for Integration`] para Concluída [`Done`]. A especificação permanece
Vigente [`Active`], sua implementação permanece Validada [`Validated`] e esta
transação é encerrada por objetivo cumprido.

## EKOM-CHG-0009 — Implementação da FanCapability 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-FAN-CAPABILITY@0.1`

**Objetivo:** implementar a capability binária de ventilador, sua configuração
própria, a API pública e o exemplo executável contratados pela versão 0.1.

### Entrada

- análise formal `Ready` registrada em
  `docs/reports/analysis/2026-08-30T020625Z-0.1-879c8930-implementability-analysis.md`;
- ordem explícita do Arquiteto para implementar a mesma versão;
- branch de especificação e árvore inicialmente limpas.

### Resultado material

- criados `FanCapability`, `FanConfig` e `FAN_ACTUATOR_TYPE`, preservando o
  comportamento comum de `BinaryCommandCapability`;
- adicionados o registro atômico no builder e a API pública
  `SmartSysApp::addFanCapability()`;
- criado o exemplo `fan` para a MCB R1, com environment, runner, catálogo,
  documentação e matriz de build;
- o environment `example_fan_mcb_r1` foi construído com sucesso;
- a implementação permanece mecanicamente `In Progress`, pois o build
  canônico `esp32_dev` não foi aprovado.

### Limitação do gate canônico

O build `pio run -e esp32_dev` falhou em referências preexistentes de
`src/main.cpp`: `ITS_MCB01_K1_PIN`, `TemperatureSensorModel::DS18B20`,
`ITS_MCB01_J4_EXT_ADC` e `ITS_MCB01_J4_EXT_IO33`. Esse arquivo não foi alterado
pela implementação e sua correção não integra o recorte autorizado. A falha
impede promover o estado mecânico para `Implemented`.

### Evidências e limites

- `pio run -e example_fan_mcb_r1`: `SUCCESS`, RAM 81188/327680 bytes e flash
  1829117/2031616 bytes;
- ELF do exemplo: exatamente um `setup()` e um `loop()`, nova API e type
  `Fan Actuator` presentes;
- `git diff --check` aprovado e nenhum artefato em `test/` alterado;
- relatório:
  `docs/reports/2026-08-30T021453Z-0.1-5343b05b-implementation-report.md`;
- testes, upload, monitor, execução instrumentada e validação física não foram
  executados; nenhum artefato de teste foi criado.

### Encerramento

A revisão e a validação em hardware foram posteriormente confirmadas pelo
Arquiteto em `EKOM-CHG-0010`. A limitação do build canônico permanece
preservada como evidência histórica, sem impedir a decisão humana de promover a
implementação para `Validated`.

## EKOM-CHG-0008 — Autoria da FanCapability 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-FAN-CAPABILITY@0.1`

**Objetivo:** registrar o contrato da `FanCapability`, de sua configuração
própria, da API pública e do exemplo executável, preservando o comportamento
binário de `SwitchCapability` e usando o type `Fan Actuator`.

### Decisões relacionadas

- `FanCapability` deriva de `BinaryCommandCapability` e usa os estados
  `off`/`on` e as operações públicas do precedente de switch;
- o type público é exatamente `Fan Actuator`;
- `FanConfig` é distinta de `SwitchConfig` e preserva o contrato vigente de
  `HardwareConfig`;
- a fachada recebe `FanConfig` por `SmartSysApp::addFanCapability()`;
- a persistência binária aplica-se automaticamente pela derivação comum;
- o exemplo `fan` usa `ITS_MCB01_RELAY_PIN` na MCB R1 e segue o catálogo
  executável vigente;
- nenhum artefato de teste integra a versão por decisão explícita do
  Arquiteto.

### Lacunas

- nenhuma lacuna normativa conhecida foi aberta na autoria;
- a classificação de implementabilidade permanece reservada à análise formal.

### Débitos técnicos relacionados

- nenhum débito técnico foi aceito nesta transação.

### Relatórios e evidências materiais

- investigação dirigida de `SwitchCapability`, `SwitchConfig`,
  `BinaryCommandCapability`, builder, fachada, pinout, exemplo de tensão,
  contratos públicos, lifecycle, persistência, mapa e dossiê;
- rascunho conversacional reconciliado e ordem explícita do Arquiteto para
  registro normativo;
- integridade textual aprovada; a guarda estrutural EKOM 4.6 foi executada e
  permaneceu reprovada somente por mapas experimentais, relatórios e seções
  históricas preexistentes fora deste recorte, sem apontar o novo documento.

### Resultado

`IOTSSC-FAN-CAPABILITY@0.1` foi registrada em `Draft`, com análise de
implementabilidade pendente, implementação não iniciada e sem bloqueio
arquitetural conhecido antes da análise formal. Nenhum código, teste, build ou
configuração funcional foi alterado.

## EKOM-CHG-0007 — Validação final do sensor de temperatura NTC 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-NTC-TEMPERATURE-SENSOR@0.1`

**Objetivo:** registrar a validação do código em hardware declarada pelo
Arquiteto, encerrar a especificação e integrar o recorte validado à `main`.

### Evidência humana recebida

O Arquiteto confirmou em ordem direta ter executado os testes em hardware,
validado o código e considerado o resultado suficiente. Na mesma ordem,
determinou o encerramento da especificação, o merge e o push para `main`.

### Confrontação consultiva

O Consultor de Arquitetura, sem alegação de independência por ter participado
das etapas anteriores, confrontou especificação, implementação, relatórios e
composição Git sem identificar conflito arquitetural ou normativo bloqueante.
Os ensaios físicos não foram reexecutados e seus registros brutos não foram
recebidos pelo Consultor.

### Promoções

- estado normativo: `Draft` → Vigente [`Active`];
- estado da implementação: `Implemented` → Validada [`Validated`];
- estado da entrega: → Pronta para integração [`Ready for Integration`].

O relatório
`docs/reports/2026-08-28T151931Z-0.1-d95b28d5-final-validation-report.md`
preserva a decisão, a confrontação e seus limites. A transação permanece em
andamento até a integração e sincronização efetivas com `main`.

### Integração e encerramento

O recorte validado foi integrado à `main` a partir da branch
`spec/ntc-temperature-sensor`, sem conflito e sem alteração adicional de
comportamento. A entrega passa de Pronta para integração
[`Ready for Integration`] para Concluída [`Done`]. A especificação permanece
Vigente [`Active`], sua implementação permanece Validada [`Validated`] e esta
transação é encerrada por objetivo cumprido.

## EKOM-CHG-0006 — Implementação do sensor de temperatura NTC 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-NTC-TEMPERATURE-SENSOR@0.1`

**Objetivo:** implementar integralmente o adapter NTC, configuração, perfis,
factory, diagnóstico e exemplo executável autorizados pela versão 0.1.

### Entrada

- análise formal `Ready` registrada em
  `docs/reports/analysis/2026-08-28T025914Z-0.1-0f251e18-implementability-analysis.md`;
- ordem explícita do Arquiteto para implementar a mesma versão;
- branch de especificação e árvore inicialmente limpas.

### Resultado

- criado `NtcTemperatureSensor`, com configuração parametrizável, presets
  100 kΩ B3950 e MF52-103 10 kΩ/B3950, 16 amostras fracionárias, equação Beta,
  diagnóstico e sentinel exato `-1000.0f`;
- estendido somente o `SensorFactory` concreto, preservando
  `ITemperatureSensor`, `TemperatureSensorCapability`, `ISensorFactory` e
  `TemperatureSensorModel`;
- criado `environment_ntc` com o GPIO oficial J4 da MCB R1, catálogo,
  environment, runner exclusivo e matriz de CI reconciliados;
- estado mecânico da implementação promovido a `Implemented`; estado normativo
  permanece `Draft` e entrega permanece `Not Ready`.

### Evidências

- `pio run -e esp32_dev -e example_environment_ntc_mcb_r1 -e
  example_environment_dht_mcb_r1 -e example_voltage_sensor_mcb_r1`: quatro
  environments `SUCCESS`, código 0;
- inspeção do ELF do novo exemplo: exatamente um `setup()` e um `loop()`;
- configuração do PlatformIO resolvida, delta sem artefatos de teste e
  `git diff --check` aprovado;
- relatório:
  `docs/reports/2026-08-28T030901Z-0.1-a28aa929-implementation-report.md`.

### Limites

Upload, monitor serial, validação instrumentada e validação física permanecem
`Not Executed`. A transação não declara validação, conclusão normativa nem
integração.

## EKOM-CHG-0005 — Autoria do sensor de temperatura NTC 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-NTC-TEMPERATURE-SENSOR@0.1`

**Objetivo:** registrar o contrato do `NtcTemperatureSensor`, sua configuração
parametrizável, os perfis iniciais, o tratamento de leitura inválida e o exemplo
executável, preservando `ITemperatureSensor` e `TemperatureSensorCapability`.

### Decisões relacionadas

- o divisor usa resistor série ao positivo e NTC ao GND;
- cada leitura usa média fracionária de exatamente 16 amostras ADC;
- a conversão usa a equação Beta e parâmetros elétricos configuráveis;
- os perfis iniciais são 100 kΩ B3950 e MF52-103 10 kΩ empiricamente tratado
  como B3950;
- toda leitura inválida retorna exatamente `-1000.0f`;
- a capability e a interface vigentes permanecem inalteradas;
- nenhum teste automatizado integra a versão; o exemplo integra o recorte.

### Lacunas

- nenhuma lacuna normativa conhecida foi aberta na autoria;
- a classificação de implementabilidade permanece reservada à análise formal.

### Débitos técnicos relacionados

- nenhum débito técnico foi aceito nesta transação.

### Relatórios e evidências materiais

- investigação dirigida de `TemperatureSensorCapability`,
  `ITemperatureSensor`, `DHTSensor`, `DS18B20TemperatureSensor`,
  `SensorFactory`, `ResistiveDividerVoltageSensor` e `environment_dht`;
- rascunho conversacional reconciliado e ordem explícita do Arquiteto para
  registro normativo;
- integridade textual aprovada; a guarda estrutural EKOM 4.6 foi executada e
  permaneceu reprovada somente por mapas experimentais, relatórios e seções
  históricas preexistentes fora deste recorte, sem apontar o novo documento.

### Resultado

`IOTSSC-NTC-TEMPERATURE-SENSOR@0.1` foi registrada em `Draft`, com análise de
implementabilidade pendente, implementação não iniciada e sem bloqueio
arquitetural conhecido antes da análise formal. Nenhum código, teste, build ou
configuração funcional foi alterado.

## EKOM-CHG-0004 — Validação final da medição de tensão 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-VOLTAGE-SENSOR@0.1`

**Objetivo:** registrar a revisão e a validação em hardware declaradas pelo
Arquiteto, encerrar a especificação e integrar o recorte validado à `main`.

### Evidência humana recebida

O Arquiteto confirmou em ordem direta ter revisado o código, executado a
validação em hardware e considerado o resultado suficiente. Na mesma ordem,
determinou o encerramento da especificação, o merge e o push para `main`.

### Confrontação consultiva

O Consultor de Arquitetura, sem alegação de independência por ter participado
das etapas anteriores, confrontou especificação, implementação, relatórios e
composição Git sem identificar conflito arquitetural ou normativo bloqueante.
Os ensaios físicos não foram reexecutados e seus registros brutos não foram
recebidos pelo Consultor.

### Promoções

- estado normativo: `Draft` → Vigente [`Active`];
- estado da implementação: `Implemented` → Validada [`Validated`];
- estado da entrega: → Pronta para integração [`Ready for Integration`].

O relatório
`docs/reports/2026-08-28T005022Z-0.1-776c305a-final-validation-report.md`
preserva a decisão, a confrontação e seus limites. A transação permanece em
andamento até a integração e sincronização efetivas com `main`.

### Integração e encerramento

O recorte validado foi integrado à `main` a partir da branch
`spec/voltage-sensing-capability`, sem conflito e sem alteração adicional de
comportamento. A entrega passa de Pronta para integração
[`Ready for Integration`] para Concluída [`Done`]. A especificação permanece
Vigente [`Active`], sua implementação permanece Validada [`Validated`] e esta
transação é encerrada por objetivo cumprido.

## EKOM-CHG-0003 — Implementação da medição de tensão 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-VOLTAGE-SENSOR@0.1`

**Objetivo:** implementar a capability, o Hardware Adapter de divisor
resistivo, a API pública, a arbitragem bilateral de ADC e o exemplo executável
contratados pela versão 0.1.

### Decisões locais

- contratos e implementação seguem a mesma separação do sensor de corrente;
- a razão do divisor é calculada somente no adapter a partir de R1/R2;
- a reserva privada de ADC do builder passou a ser comum a corrente e tensão;
- o exemplo consulta somente a medição estável da capability; a razão exibida
  no boot vem do diagnóstico do adapter.

### Lacunas

- nenhuma lacuna normativa ou pré-requisito arquitetural foi identificado;
- validações instrumentadas, upload, monitor e hardware permanecem
  `Not Executed`.

### Débitos técnicos relacionados

- nenhum débito técnico foi aceito nesta transação.

### Relatórios e evidências materiais

- `docs/reports/2026-08-27T221102Z-0.1-323fe0bf-implementation-report.md`;
- `pio run -e esp32_dev`: `SUCCESS`, código 0;
- `pio run -e example_voltage_sensor_mcb_r1`: `SUCCESS`, código 0;
- `pio run -e example_current_sensor_mcb_r1`: `SUCCESS`, código 0;
- ELF do novo exemplo contém exatamente um `setup()` e um `loop()`;
- nenhum teste foi criado, alterado ou executado.

### Resultado

`IOTSSC-VOLTAGE-SENSOR@0.1` possui implementação integral no recorte e estado
mecânico Implementada [`Implemented`]. A entrega segue para revisão técnica;
validação física, conclusão e integração não são declaradas.

## EKOM-CHG-0001 — Migração da governança para EKOM 4.6

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** Não se aplica [`Not Applicable`]

**Objetivo:** atualizar a fundação documental do repositório da EKM 1.19 para
o EKOM 4.6 sem alterar código, testes, dependências, build, automações ou
configuração funcional.

### Decisões relacionadas

- adoção do método, regras comuns, perfis e templates vigentes do EKOM 4.6;
- preservação não retroativa do histórico e dos identificadores EKM 1.x;
- novas transações, lacunas e débitos usam o namespace `EKOM`;
- a especificação relacionada é `Not Applicable` por se tratar de governança.

### Lacunas

- nenhuma lacuna nova foi aberta.

### Débitos técnicos relacionados

- nenhum débito técnico foi aceito nesta transação.

### Relatórios e evidências materiais

- confronto documental com `EKOM-METHOD.md`, `GOVERNANCE.md`,
  `DESIGN-DECISIONS.md`, perfis e templates oficiais do EKOM 4.6;
- guarda estrutural EKOM 4.6 e `git diff --check` executados sobre o delta
  documental.
- o Consultor participou da migração e não alega revisão independente deste
  mesmo recorte.

### Resultado

O roteamento de agentes, as diretrizes locais, o mapa de conhecimento e o
adaptador do Claude Code passam a referenciar o EKOM 4.6. As fontes EKM 1.x
permanecem históricas; código, testes, dependências, build, automações e
configuração funcional não foram alterados.

## EKOM-CHG-0002 — Autoria da medição de tensão 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-VOLTAGE-SENSOR@0.1`

**Objetivo:** registrar o contrato da `VoltageSensorCapability`, do
`IVoltageSensor`, do adapter de divisor resistivo, da API pública e do exemplo
executável, preservando a separação de responsabilidades do sensor de corrente.

### Decisões relacionadas

- capability e type são `VoltageSensorCapability` e `Voltage Sensor (V)`;
- R1/R2 determinam dinamicamente a razão `(R1 + R2) / R2`;
- não existe offset; `adcMinimumMv` possui default de 144 mV;
- leitura abaixo do mínimo publica `-1000.00` e `BELOW_MINIMUM`;
- valores usam duas casas; corrente e tensão arbitram bilateralmente o ADC;
- nenhum teste automatizado integra a versão; o exemplo integra o recorte.

### Lacunas

- nenhuma lacuna normativa conhecida foi aberta na autoria; a classificação de
  implementabilidade permanece reservada à análise formal.

### Débitos técnicos relacionados

- nenhum débito técnico foi aceito nesta transação.

### Relatórios e evidências materiais

- investigação dirigida do precedente `IOTSSC-CURRENT-SENSOR@0.6`, contratos
  públicos, lifecycle, builder, factory, evento, exemplo, mapa e dossiê;
- rascunho conversacional reconciliado e ordem explícita do Arquiteto para
  registro normativo;
- guarda estrutural EKOM 4.6 e integridade textual sobre o delta documental.

### Resultado

`IOTSSC-VOLTAGE-SENSOR@0.1` foi registrada em `Draft`, com análise de
implementabilidade pendente, implementação não iniciada e sem bloqueio
arquitetural conhecido antes da análise formal. Nenhum código, teste, build ou
configuração funcional foi alterado.
