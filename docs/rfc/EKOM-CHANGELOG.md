# Histórico de mudanças EKOM — IoTSmartSysCore

Este arquivo registra transações iniciadas sob EKOM 4.6. O histórico anterior
permanece preservado em `docs/rfc/EKM-CHANGELOG.md`.

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
