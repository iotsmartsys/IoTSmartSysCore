# Histórico de mudanças EKOM — IoTSmartSysCore

Este arquivo registra transações iniciadas sob EKOM 4.6. O histórico anterior
permanece preservado em `docs/rfc/EKM-CHANGELOG.md`.

## EKOM-CHG-0022 — Especificação da capacidade configurável do runtime 0.2

**Estado:** Aberta [`Open`]

**Especificação relacionada:** `IOTSSC-RUNTIME-CAPABILITY-CAPACITY@0.2`

**Objetivo:** substituir o limite universal de oito por capacidade estática
configurável, permitir doze capabilities no perfil `ESP32_MCB01`, preservar o
default oito, ampliar com migração o snapshot NVS binário e tornar reproduzível
a composição de nove capabilities da aplicação MCB01.

### Decisão do Arquiteto

- capacidade configurável em build, com default oito e máximo suportado doze;
- perfil `ESP32_MCB01` configurado com doze;
- correção completa dos limites auxiliares, arena e persistência;
- migração segura do formato NVS versão 2 em vez de perda dos estados válidos;
- adapter de tensão PV único e compartilhado com `pv-power-1`;
- registro da direção transversal em ADR.

### Estado operacional

A especificação 0.1 e a ADR-0001 foram registradas. A implementação permanece
Não iniciada [`Not Started`] e a entrega Não pronta [`Not Ready`], pendente da
Análise de Implementabilidade formal solicitada pelo Arquiteto.

### Análise de Implementabilidade

A análise formal confrontou 31 requisitos, 9 critérios, as autoridades
emendadas e a baseline técnica. A versão foi classificada Pronta [`Ready`], sem
bloqueador normativo, material, arquitetural ou de evidência prévia. O registro
imutável é
`docs/reports/analysis/2026-09-04T022539Z-0.1-40e5b583-implementability-analysis.md`.

A classificação conclui o estágio técnico da versão 0.1 e não inicia nem
autoriza implementação, validação física, integração ou conclusão.

### Correção normativa 0.2

Após esclarecimento da falha da guarda EKOM, o Arquiteto confirmou que a
validação deve exigir ausência de novos achados no recorte e registrar
separadamente a falha preexistente. A versão 0.1 e seu relatório permanecem
históricos; sua classificação `Ready` não se aplica à versão corrigida.

A versão 0.2 acrescenta `CAP-032` e corrige `CAP-AC-009`, removendo a exigência
insatisfazível de código zero global sem ocultar, reinterpretar ou autorizar a
correção de documentos fora do recorte. Capacidade, persistência, aplicação e
permissões permanecem inalteradas. A versão corrigida segue para nova Análise
de Implementabilidade.

### Análise de Implementabilidade 0.2

A nova análise reconciliou expressamente a classificação inadequada da versão
0.1, confrontou 32 requisitos, 9 critérios e zero débitos relacionados e
classificou a versão 0.2 como Pronta [`Ready`], sem bloqueadores. O relatório
imutável é
`docs/reports/analysis/2026-09-04T023149Z-0.2-6e89018b-implementability-analysis.md`.

A guarda global foi novamente executada e permaneceu não aprovada por achados
preexistentes fora do recorte; nenhum novo documento desta transação apareceu
na saída. Esse fato é preservado e não impede a implementabilidade sob o
contrato corrigido.

## EKOM-CHG-0021 — Revisão, validação e integração INA3221 0.2

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-INA3221-SENSORS@0.2`

**Objetivo:** registrar a revisão do quarto estágio, incorporar a validação em
hardware declarada pelo Arquiteto, encerrar a especificação e promover o recorte
validado para `main`.

### Revisão

- os 60 requisitos e os 9 critérios de aceite foram confrontados;
- o resultado foi classificado como Aderente [`Conformant`], sem defeito
  material de implementação ou de especificação identificado;
- a revisão não é apresentada como independente, pois o mesmo agente participou
  das etapas anteriores;
- o relatório é
  `docs/reports/2026-09-03T144552Z-0.2-88d15fcf-review-report.md`.

### Evidência e decisão do Arquiteto

O Arquiteto declarou ter executado as validações em hardware, validou o código,
considerou a evidência suficiente para encerrar a especificação e ordenou a
promoção do código para `main`. Registros brutos e valores de bancada não foram
fornecidos nesta atuação e não são inferidos.

Por essa decisão, a versão passa a `Active`, a implementação a `Validated` e a
entrega a `Ready for Integration`. O relatório consultivo é
`docs/reports/2026-09-03T144552Z-0.2-18759811-final-validation-report.md`.

### Evidências técnicas reconciliadas

- `pio run -e example_ina3221_voltage_current_mcb_r1`: `SUCCESS`, código 0,
  usando `Adafruit INA3221 Library 1.0.1`, RAM 68.908/327.680 bytes e flash
  1.182.157/2.031.616 bytes;
- inspeção do ELF: exatamente um `setup()` e um `loop()`, componentes INA3221 e
  overloads públicos presentes;
- `pio run -e esp32_dev`: `FAILED`, código 1, em referências preexistentes de
  `src/main.cpp` a símbolos MCB R1 e `TemperatureSensorModel::DS18B20`; esse
  arquivo não integra o delta INA3221;
- nenhuma alteração existe em `test/`, coerente com a exclusão normativa.

### Estado operacional

A validação e o encerramento foram determinados pelo Arquiteto. A transação
foi concluída após a integração e a sincronização efetivas com `main`.

### Integração e encerramento

O recorte validado foi integrado à `main` a partir da branch
`spec/ina3221-sensors` por fast-forward, sem conflito. A entrega passa de Pronta
para integração [`Ready for Integration`] para Concluída [`Done`]. A
especificação permanece Vigente [`Active`], sua implementação permanece
Validada [`Validated`] e a transação é encerrada por objetivo cumprido.

## EKOM-CHG-0020 — Implementação da correção INA3221 0.2

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-INA3221-SENSORS@0.2`

**Objetivo:** aplicar aos manifestos a dependência PlatformIO `^1.0.1` decidida
pelo Arquiteto e repetir os builds normativos.

### Entrada

- análise formal `Ready` registrada em
  `docs/reports/analysis/2026-09-03T021853Z-0.2-2f1e37a3-implementability-analysis.md`;
- ordem explícita do Arquiteto para ajustar a dependência para `^1.0.1`;
- branch de especificação e árvore inicialmente limpas;
- nenhum teste ou operação de hardware integra a autorização.

### Estado operacional

A correção foi concluída e a implementação da versão 0.2 está Implementada
[`Implemented`], seguindo para Revisão. A entrega permanece Não pronta
[`Not Ready`].

### Resultado material

- `library.json` e o environment do exemplo passam a declarar
  `adafruit/Adafruit INA3221 Library@^1.0.1`;
- o registro PlatformIO resolveu e instalou `Adafruit INA3221 Library 1.0.1`;
- nenhuma API, implementação dos adapters, exemplo, teste ou configuração
  elétrica foi alterada.

### Evidências

- `pio run -e example_ina3221_voltage_current_mcb_r1`: `SUCCESS`, código 0,
  RAM 68.908/327.680 bytes e flash 1.182.157/2.031.616 bytes;
- `pio run -e esp32_dev`: `SUCCESS`, código 0, RAM 26.504/327.680 bytes e
  flash 375.773/2.031.616 bytes;
- `git diff --check` e validação sintática de `library.json`: aprovados;
- testes, upload, monitor serial e validação em hardware: `Not Executed`.

### Relatório

`docs/reports/2026-09-03T022216Z-0.2-7b98f201-implementation-correction-report.md`.

## EKOM-CHG-0019 — Correção da dependência INA3221 0.2

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-INA3221-SENSORS@0.2`

**Objetivo:** corrigir a versão 0.1, que confundiu a tag Git upstream `1.0.3`
com uma versão publicada do pacote no registro PlatformIO.

### Decisão do Arquiteto

- usar `adafruit/Adafruit INA3221 Library@^1.0.1`, a versão mais recente
  publicada no registro empregado pelo projeto;
- não trocar de biblioteca nem adotar URL Git como origem da dependência.

### Alteração normativa

- versão da especificação promovida de 0.1 para 0.2;
- INA-044 e INA-AC-006 passam a exigir resolução de `^1.0.1`;
- fonte técnica passa a distinguir o pacote PlatformIO da tag upstream;
- requisitos funcionais, API, ownership, exemplo e limites elétricos permanecem
  inalterados.

### Resultado

A versão 0.2 permanece `Draft`/`In Progress`, retorna mecanicamente a
`Pending Review` e segue para nova análise de implementabilidade. A
implementação da versão 0.1 permanece como baseline parcial.

## EKOM-CHG-0018 — Implementação dos sensores INA3221 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-INA3221-SENSORS@0.1`

**Objetivo:** implementar e validar por build o dispositivo compartilhado, os
adapters de tensão e corrente, os overloads públicos e o exemplo executável
combinado contratados pela versão 0.1.

### Entrada

- análise formal `Ready` registrada em
  `docs/reports/analysis/2026-09-03T015101Z-0.1-79dd19a7-implementability-analysis.md`;
- ordem explícita do Arquiteto para implementar a versão corrente;
- branch de especificação e árvore inicialmente limpas;
- nenhum artefato automatizado de teste integra a versão.

### Estado operacional

A implementação está Em andamento [`In Progress`]. Testes, upload, monitor
serial e validação física não estão autorizados nesta atuação.

### Resultado material

- implementados dispositivo compartilhado, adapters INA3221 de tensão e
  corrente e overloads públicos sob ownership externo;
- criado o exemplo combinado, com MCB R1, `Wire`, GPIOs 21/22, endereço `0x40`,
  canal 0, shunt R100, catálogo, environment e documentação elétrica;
- preservados os modelos e APIs existentes; nenhum teste foi criado ou alterado.

### Evidências e limitação

- o build diagnóstico do exemplo com a tag Git oficial `1.0.3` foi aprovado,
  com RAM 68.908/327.680 bytes e flash 1.182.157/2.031.616 bytes;
- o build canônico do exemplo falhou, código 1, porque o registro PlatformIO
  oferece somente a versão `1.0.1` e não resolve o requisito `^1.0.3`;
- `esp32_dev` compilou os novos objetos e falhou depois, código 1, por símbolos
  MCB R1 e enum de temperatura preexistentes em `src/main.cpp`;
- a escolha entre tag Git, versão publicada `1.0.1` ou espera por publicação é
  normativa e permanece pendente do Arquiteto;
- testes, execução instrumentada, upload, monitor e hardware permanecem
  `Not Executed`; a transação continua aberta e `In Progress`.

### Relatório

`docs/reports/2026-09-03T020435Z-0.1-0e936b93-implementation-report.md`.

### Encerramento

A pendência de resolução da dependência foi substituída e resolvida pela versão
0.2 na transação `EKOM-CHG-0020`. O resultado da implementação vigente é
registrado por essa transação posterior.

## EKOM-CHG-0017 — Autoria dos sensores INA3221 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-INA3221-SENSORS@0.1`

**Objetivo:** registrar o contrato dos adapters INA3221 de tensão e corrente,
do dispositivo I²C compartilhado, dos overloads públicos sob ownership externo
e do exemplo executável combinado.

### Decisões relacionadas

- `INA3221Device` possui uma única instância da biblioteca Adafruit e possui
  setup idempotente para compartilhamento pelos dois adapters;
- `INA3221VoltageSensor` implementa `IVoltageSensor` e
  `INA3221CurrentSensor` implementa `ICurrentSensor` sem alterar os modelos
  preexistentes;
- `SmartSysApp` recebe overloads aditivos que registram adapters externos por
  referência e possui somente as respectivas capabilities;
- o exemplo único `ina3221_voltage_current` usa MCB R1, `Wire`, SDA 21, SCL 22,
  endereço `0x40`, canal 0 e shunt R100 de `0,100 Ω`;
- o limite de conversão derivado do shunt não afirma capacidade térmica, e o
  ensaio inicial documentado fica limitado a `0,5 A`;
- nenhum artefato automatizado de teste integra a versão 0.1.

### Lacunas

- nenhuma lacuna normativa conhecida foi aberta na autoria;
- a biblioteca não expõe falha separada em toda leitura de registrador, de modo
  que a versão não garante detecção de toda desconexão posterior ao setup;
- a classificação de implementabilidade permanece reservada à análise formal.

### Débitos técnicos relacionados

- nenhum débito técnico foi aceito nesta transação.

### Relatórios e evidências materiais

- investigação dirigida das interfaces, capabilities, builders, factories,
  exemplos, dependências, mapa, dossiê e especificações relacionadas;
- confronto com a biblioteca Adafruit INA3221 1.0.3 e com o datasheet TI
  `SBOS576C`;
- rascunho conversacional reconciliado e ordem explícita do Arquiteto para
  registro normativo;
- `git diff --check`: aprovado sobre o delta documental;
- guarda estrutural EKOM executada e reprovada somente por passivos
  preexistentes fora do recorte: mapa do experimento NVS sem as seções do mapa
  principal, relatórios legados sem metadados exigidos pelo validador atual e
  seções históricas de análise incorporadas a `SCREEN-CONSOLE-TOOLING.md`.

### Resultado

`IOTSSC-INA3221-SENSORS@0.1` foi registrada em `Draft`, com análise de
implementabilidade pendente, implementação não iniciada e sem pré-requisito
arquitetural conhecido antes da análise formal. Nenhum código, teste,
dependência, build ou configuração funcional foi alterado.

## EKOM-CHG-0016 — Validação final da PowerEnergyCapability 0.3

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-POWER-ENERGY-CAPABILITY@0.3`

**Objetivo:** registrar a validação declarada pelo Arquiteto, encerrar a
especificação e integrar o recorte validado à `main`.

### Evidência humana recebida

O Arquiteto declarou em ordem direta que validou a implementação e determinou
o encerramento da especificação, sua promoção para `main`, commit e push. A
decisão considera suficiente o conjunto PWR-AC-001 a PWR-AC-009, sem exigir
reexecução pelo Consultor.

### Confrontação consultiva

O Consultor de Arquitetura, sem alegação de independência por ter participado
das etapas anteriores, confrontou especificação, implementação, relatórios e
composição Git sem identificar conflito arquitetural ou normativo bloqueante.

O build próprio do exemplo permanece aprovado. O build canônico reprovado
durante a Implementação permanece registrado com seu resultado original e não
é convertido em sucesso. A ordem não informou registros brutos nem caracterizou
se a validação recebida incluiu execução física; esta atuação não infere essa
evidência.

### Promoções

- estado normativo: `Draft` → Vigente [`Active`];
- estado da implementação: `In Progress` → Validada [`Validated`];
- estado da entrega: → Pronta para integração [`Ready for Integration`].

O relatório
`docs/reports/2026-09-02T203647Z-0.3-46cc009f-final-validation-report.md`
preserva a decisão, a confrontação e seus limites. A transação permanece em
andamento até a integração e sincronização efetivas com `main`.

### Integração e encerramento

O recorte validado foi integrado à `main` a partir da branch
`spec/power-energy-capability` por fast-forward, sem conflito. A entrega passa
de Pronta para integração [`Ready for Integration`] para Concluída [`Done`]. A
especificação permanece Vigente [`Active`], sua implementação permanece
Validada [`Validated`] e esta transação é encerrada por objetivo cumprido.

## EKOM-CHG-0015 — Implementação da PowerEnergyCapability 0.3

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-POWER-ENERGY-CAPABILITY@0.3`

**Objetivo:** implementar e validar por build o exemplo executável
`power_energy`, completando o recorte funcional contratado pela versão 0.3.

### Entrada

- análise formal `Ready` registrada em
  `docs/reports/analysis/2026-09-02T195142Z-0.3-22e4fb59-implementability-analysis.md`;
- ordem explícita do Arquiteto para implementar a versão corrente;
- branch de especificação e árvore inicialmente limpas;
- nenhum artefato de teste integra a versão.

### Estado operacional

A implementação permanece Em andamento [`In Progress`]. Testes, upload,
monitor serial e validação física não estão autorizados nesta atuação.

### Resultado material

- criado o exemplo `power_energy` com ownership externo dos adapters públicos,
  lifecycle explícito e registro exclusivo da `PowerEnergyCapability`;
- configurados ACS712-30A no símbolo `ITS_MCB01_J4_EXT_ADC` e divisor
  330 kΩ/10 kΩ no símbolo `ITS_MCB01_J4_EXT_IO33`;
- adicionados apresentação de potência, energia e estado, reset serial local,
  environment, seletor exclusivo, catálogo e documentação de bancada;
- nenhuma API pública ou implementação interna da capability foi alterada.

### Evidências e limitação

- `pio run -e example_power_energy_mcb_r1`: `SUCCESS`, código 0, 17,743 s,
  seguido de verificação final incremental em 7,661 s, com RAM
  68.916/327.680 bytes e flash 1.156.681/2.031.616 bytes;
- a primeira execução desse build falhou por uma qualificação local de
  namespace; a correção foi aplicada e a repetição alcançou sucesso;
- o ELF contém exatamente um `setup()` e um `loop()`, os dois adapters, a API
  pública, `powerEnergyMeasurement()` e `resetEnergy()`;
- `pio run -e esp32_dev`: `FAILED`, código 1, 13,514 s, por símbolos MCB R1 e
  enum de temperatura preexistentes e incompatíveis em `src/main.cpp`, arquivo
  fora do delta desta implementação;
- como o build canônico obrigatório não foi aprovado, a versão não avança para
  `Implemented` e a transação permanece aberta;
- nenhum teste foi criado, alterado ou executado.

### Encerramento

A validação e a suficiência das evidências foram posteriormente confirmadas
pelo Arquiteto em `EKOM-CHG-0016`. A limitação do build canônico permanece
preservada como evidência histórica, sem impedir a decisão humana de promover a
implementação para `Validated`.

## EKOM-CHG-0014 — Correção da PowerEnergyCapability 0.3

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-POWER-ENERGY-CAPABILITY@0.3`

**Objetivo:** incorporar o bloqueador da análise formal 0.2 e eliminar a
contradição entre o fora de escopo e a configuração dos adapters pelo exemplo
`power_energy`.

### Decisão do Arquiteto

- a exclusão de aquisição, configuração, calibração e validação física de
  tensão ou corrente aplica-se somente à implementação interna da
  `PowerEnergyCapability`;
- o exemplo permanece autorizado a instanciar, configurar e conduzir os
  adapters preexistentes estritamente para demonstrar a composição contratada;
- os demais requisitos, critérios, GPIOs, lifecycle externo e a proibição de
  artefatos de teste permanecem inalterados.

### Reconciliação

O bloqueador “Configuração dos sensores simultaneamente exigida e excluída”,
registrado em
`docs/reports/analysis/2026-09-02T193655Z-0.2-2fd4ef7f-implementability-analysis.md`,
foi incorporado mediante delimitação explícita do fora de escopo. A relação é
`Corrects` sobre a versão 0.2 e não cria capacidade arquitetural, API ou
comportamento adicional.

### Resultado

`IOTSSC-POWER-ENERGY-CAPABILITY@0.3` permanece em `Draft`, com implementação
`Not Started`, entrega `Not Ready` e análise `Pending Review`. Nenhum código,
teste, build ou configuração funcional foi alterado; a versão segue para nova
análise formal.

## EKOM-CHG-0013 — Autoria da PowerEnergyCapability 0.2

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-POWER-ENERGY-CAPABILITY@0.2`

**Objetivo:** corrigir a versão 0.1 ao contratar o exemplo executável
`power_energy`, omitido no recorte anterior, preservando o contrato funcional
da capability e tornando observável o lifecycle externo dos sensores.

### Decisões relacionadas

- o exemplo é destinado à MCB R1 e registra somente a
  `PowerEnergyCapability`;
- o ACS712-30A usa `ITS_MCB01_J4_EXT_ADC`/GPIO 34, e o divisor de tensão usa
  `ITS_MCB01_J4_EXT_IO33`/GPIO 33, expressamente autorizado pelo Arquiteto como
  segunda entrada analógica ADC1 do exemplo;
- os adapters são externamente possuídos pelo exemplo, recebem `setup()` antes
  de `SmartSysApp::setup()` e recebem `handle()` antes de
  `SmartSysApp::handle()` em cada ciclo;
- o ACS712 usa o perfil público de 3,3 V sem monitor de alimentação; por isso,
  potência e energia numéricas são apresentadas como `ESTIMATED`, sem afirmação
  de exatidão contratada;
- `resetEnergy()` é demonstrado somente pelo comando serial local `r`;
- nenhuma API pública será alterada para facilitar o exemplo;
- nenhum artefato de teste integra a versão e nenhum teste foi criado,
  alterado ou executado nesta autoria.

### Relações e evidências

- relação `Corrects` sobre `IOTSSC-POWER-ENERGY-CAPABILITY@0.1`;
- preservação de `IOTSSC-HW-EXAMPLES`, inclusive seleção por environment,
  símbolos oficiais de pinout, aplicação Arduino única e documentação de
  validação manual;
- investigação dirigida do precedente `current_sensor`, do exemplo
  `voltage_sensor`, dos adapters públicos, do runner, dos environments e do
  pinout da MCB R1;
- rascunho conversacional confirmado e ordem explícita do Arquiteto para
  registrar a versão 0.2.

### Resultado

`IOTSSC-POWER-ENERGY-CAPABILITY@0.2` foi registrada em `Draft`, com análise de
implementabilidade `Pending Review`, implementação `Not Started` e entrega
`Not Ready`. O código implementado da versão 0.1 permanece como baseline
histórica, mas seu relatório `Ready` não autoriza o novo recorte. Nenhum código,
teste, build ou configuração funcional foi alterado; a versão 0.2 segue para
nova análise formal.

## EKOM-CHG-0012 — Implementação da PowerEnergyCapability 0.1

**Estado:** Fechada [`Closed`]

**Especificação relacionada:** `IOTSSC-POWER-ENERGY-CAPABILITY@0.1`

**Objetivo:** implementar a composição de potência e energia, sua API pública,
registro, publicação aditiva e ownership externo dos sensores contratados pela
versão 0.1.

### Entrada

- análise formal `Ready` registrada em
  `docs/reports/analysis/2026-09-02T022248Z-0.1-ab1fccfc-implementability-analysis.md`;
- ordem explícita do Arquiteto para implementar a mesma versão;
- branch de especificação e árvore inicialmente limpas;
- nenhum artefato de teste integra a versão.

### Resultado material

- criados contratos, estados, configuração e implementação da
  `PowerEnergyCapability`;
- implementados magnitude da potência, integração trapezoidal, energia volátil,
  reset, precedência de estados, finitude, cadência e publicação por mudança;
- adicionado o campo opcional `energyWh` nas serializações de evento e MQTT,
  preservando payloads preexistentes quando ausente;
- adicionados registro atômico no builder e fachada pública com ownership
  externo dos sensores;
- estado mecânico promovido a Implementada [`Implemented`].

### Evidências e limites

- relatório:
  `docs/reports/2026-09-02T023107Z-0.1-e3e419d7-implementation-report.md`;
- `pio run -e esp32_dev`: `SUCCESS`, código 0, 22,593 s, RAM 26.504/327.680
  bytes e flash 375.773/2.031.616 bytes;
- inspeção dos objetos confirma classe, métodos próprios, builder e fachada;
- `git diff --check` aprovado;
- guarda estrutural do delta aprovada; a varredura integral permanece reprovada
  somente por documentos preexistentes fora do recorte;
- nenhum teste foi criado, alterado ou executado;
- execuções instrumentadas, upload, monitor e hardware permanecem
  `Not Executed`.

### Resultado

A implementação integral e o build canônico estão concluídos. A especificação
permanece `Draft`, a entrega permanece `Not Ready` e o recorte segue para
Revisão; validação, conclusão e integração não são declaradas.

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
