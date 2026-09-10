# Especificação — Suporte ADC1 por modelo de ESP32

**ID:** `IOTSSC-ESP32-ADC1-SUPPORT`

**Classe da fonte:** Normativa

**Versão:** 0.1

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Pendente [`Pending`]

**Revisão de implementabilidade:** Consultar o relatório separado
`docs/reports/2026-09-10T005744Z-0.1-1932659e-implementability-analysis.md`

**Branch:** `spec/esp32-adc1-support`

## 1. Objetivo e contexto

Aceitar os GPIOs de ADC1 do modelo de ESP32 selecionado no build nos sensores
NTC, corrente ACS712 e tensão por divisor resistivo. O relato do Arquiteto
mostra rejeição dos GPIOs 0 e 1 no ESP32-C3. A baseline restringe a validação do
NTC e os factories de corrente/tensão ao ESP32 clássico.

Acrescentar seleção tipada dos GPIOs ADC1 nos dois presets públicos de NTC,
sem impor esse tipo aos campos de configuração nem às APIs de criação dos
sensores. O rascunho e esse adicional foram confirmados pelo Arquiteto, que
ordenou o registro seguido de análise de implementabilidade.

## 2. Autoridades e relações

- **New:** enum de GPIOs ADC1 por target e contrato de seleção nos presets.
- **Amends** `NTC-TEMPERATURE-SENSOR.md@0.1`: NTC-004 e NTC-011 quanto ao
  target; NTC-005 quanto às assinaturas dos dois presets; NTC-AC-001,
  NTC-AC-006 e NTC-AC-008 quanto à seleção tipada e migração do consumidor.
- **Amends** `CURRENT-SENSING-CAPABILITY.md@0.6`: exclusividade do target
  clássico, seção 6.0 e CUR-DEC-012 somente para permitir outros targets com
  parâmetros elétricos próprios; preserva a baseline elétrica e as evidências
  do clássico, sem transferir sua qualificação metrológica aos novos chips.
- **Amends** `VOLTAGE-SENSING-CAPABILITY.md@0.1`: restrição de plataforma das
  seções 1–3 e VLT-009 para incluir ADC1 dos demais targets. Preserva conversão,
  estados, limites configuráveis e arbitragem bilateral VLT-028/VLT-029.
- **Amends** `PUBLIC-API-COMPATIBILITY.md`, API-002/API-008, exclusivamente
  pela quebra explícita das duas assinaturas de presets de NTC. A migração
  obrigatória está na seção 6; a exceção não alcança criação de sensores.
- **Preserves:** `CORE-RUNTIME-LIFECYCLE.md`, ownership, separação
  Contracts/Core/Platform/App, capacidade vigente, eventos e interfaces dos
  sensores; `EXECUTABLE-HARDWARE-EXAMPLES.md`, incluindo o símbolo oficial do
  pino do exemplo MCB R1; ADR-0001 de capacidade configurável.

Esta fonte governa somente essas alterações. As versões concluídas continuam
históricas e não são reabertas ou reescritas nesta atuação. A proposta não
introduz camada, serviço, registro geral de GPIOs ou direção de dependência
nova; não exige uma capacidade arquitetural preparatória.

## 3. Escopo e fronteira de suporte

O recorte inclui validação de GPIO/target, enum público junto à configuração
NTC na camada Arduino, presets, integração nos caminhos existentes de criação
e migração de consumidores e documentação relacionados.

“Todos os modelos” significa os targets da família ESP32 para os quais o
framework Arduino selecionado disponibiliza ADC1. A implementação deve
identificar a unidade pelo mapa oficial do SoC; pertencer à família ESP32 ou
possuir algum ADC não basta. Não se presume que o mesmo número de GPIO tenha
a mesma função em dois modelos.

O suporte aqui é dos componentes ADC. Não declara que todos os serviços do
runtime, Wi-Fi, BLE ou todas as placas comerciais funcionem em todos os SoCs.
Targets como H2/P4 não demandam criação de conectividade ausente neste recorte.
C2/C61 dependem da disponibilidade de seu suporte no framework selecionado;
não se contrata reconstrução do framework nem suporte a SoCs futuros por
presunção. Versões de framework efetivamente utilizadas devem ser registradas
na evidência de implementação.

Ficam fora do recorte ADC2, ESP8266, runtime ESP-IDF nativo, atualização
obrigatória de toolchain, calibração persistente, novo algoritmo térmico,
garantia de precisão por modelo, arbitragem geral de GPIOs e mudanças em
`private.ini`, credenciais, firmware privado ou hardware.

## 4. Requisitos

- **ADC1-001:** determinar suporte e GPIOs ADC1 pelo target real e fontes
  oficiais correspondentes. Um target conhecido com ADC1 não pode ser rejeitado
  apenas por não ser `CONFIG_IDF_TARGET_ESP32`. No C3, GPIOs 0 a 4 são ADC1;
  GPIO5 é ADC2 e não pode ser aceito por este contrato.
- **ADC1-002:** validar inteiros antes de conversões estreitas. Rejeitar
  negativos, números fora do mapa, GPIOs sem ADC e ADC2. Ausência de suporte
  ou mapeamento oficial utilizável deve produzir rejeição determinística,
  nunca usar o mapa do clássico como fallback.
- **ADC1-003:** expor um `enum class` de pinos ADC1, selecionado em compilação
  pelo modelo, com membros identificáveis pelo GPIO. Somente GPIOs ADC1 desse
  target podem ser membros; nenhum membro inválido/sentinel ou conversão
  implícita de inteiro deve permitir contornar a seleção normal. Nome e
  localização exatos seguem as convenções da camada Arduino.
- **ADC1-004:** `NTC_100K_B3950` e `MF52_103_B3950` devem receber esse enum,
  sem overload que continue aceitando `int`. O preset deve materializar o
  número físico correspondente em `adcPin`. Os parâmetros térmicos e elétricos
  existentes continuam configuráveis após a criação.
- **ADC1-005:** `NtcTemperatureSensorConfig::adcPin` permanece `int`. Factory,
  construtor do sensor, `isSupportedAdcPin(int)` e demais APIs de criação de
  sensores conservam suas assinaturas. Configuração manual continua possível
  e sujeita à validação em runtime. Cast explícito que fabrique um valor do
  enum não dispensa essa validação; não se promete impedir casts em C++.
- **ADC1-006:** aplicar suporte ADC1 aos caminhos existentes de NTC, corrente
  e tensão, incluindo `supplyMonitorAdcPin` opcional de corrente. Preservar a
  rejeição atômica, diagnóstico, ownership, reserva bilateral de corrente/tensão
  e proteção do NTC construído diretamente. O enum não se torna parâmetro
  obrigatório de presets de corrente/tensão ou da criação de sensores.
- **ADC1-007:** preservar APIs não emendadas, presets térmicos, defaults do
  ESP32 clássico, cálculo Beta, 16 amostras fracionárias do NTC, sentinels,
  aquisição cooperativa de corrente/tensão e payloads vigentes. Código do Core
  não deve ganhar dependência de headers de SoC para consumir o enum Arduino.
- **ADC1-008:** conferir resolução solicitada, atenuação e faixa elétrica
  nos targets adicionados. `FULL_RANGE` representa a atenuação máxima
  disponibilizada pelo target/framework, sem afirmar que a faixa do clássico
  seja universal. Parâmetros de tensão, referência e limiares permanecem
  explícitos e configuráveis; não devem ser corrigidos silenciosamente.
  Os defaults existentes são preservados e a documentação deve indicar os
  ajustes necessários para cada target, sem apresentar defaults do clássico
  como perfil fisicamente validado nos demais.
- **ADC1-009:** distinguir o coeficiente configurável `adcReferenceVoltageV`
  usado na conversão bruta do NTC da alimentação do divisor e da faixa
  calibrada do ADC. A seleção de atenuação deve seguir o target; ampliar a
  aceitação de pinos não certifica a conversão elétrica nem a temperatura.
  Nenhum novo algoritmo ou garantia metrológica integra esta versão.
- **ADC1-010:** manter a numeração física de GPIO. O enum descreve capacidade
  ADC1 do SoC, não disponibilidade no conector ou segurança de uso da placa.
  Reservas existentes de placa/framework continuam aplicáveis; não criar
  restrição adicional a todo GPIO de strapping por inferência.
- **ADC1-011:** migrar todas as chamadas versionadas dos dois presets e a
  documentação de consumo. No exemplo MCB R1, preservar
  `ITS_MCB01_J4_EXT_ADC` como autoridade do GPIO e verificar sua correspondência
  com ADC1 ao adaptá-lo à API tipada; um cast sem verificação não constitui a
  migração segura do exemplo.
- **ADC1-012:** documentar a matriz de targets, mapas ADC1, versões do
  framework, parâmetros elétricos conferidos e evidências disponíveis.
  Build, inspeção e validação física devem ser identificados separadamente,
  incluindo falha e ausência de execução.

## 5. Critérios de aceite

| Critério | Requisitos | Cenário, resultado e meio |
|---|---|---|
| ADC1-AC-001 | 001, 002, 010 | Confrontar enum e validação com o mapa oficial de cada target abrangido: todos os GPIOs ADC1 são reconhecidos, sujeitos às reservas existentes; ADC2, números negativos e fora do mapa são rejeitados. Inspeção dos mapas e caminhos de validação, com evidência identificada por target. |
| ADC1-AC-002 | 003, 004, 005 | No build C3, os dois presets oferecem GPIO0/GPIO1 e nenhum membro GPIO5; uma chamada normal com inteiro não é aceita pela assinatura. Inspeção de tipos e compilação de consumidores válidos. Configuração manual por inteiro continua compilável. |
| ADC1-AC-003 | 005, 006 | Configuração manual ou enum forjado com GPIO inválido chega à validação em runtime e é rejeitada pelo caminho correspondente; configuração restante válida com GPIO0/1 no C3 não sofre rejeição por target. Inspeção de factory, construtor e builder; confirmação operacional posterior por logs. |
| ADC1-AC-004 | 006, 007 | Corrente/tensão aceitam ADC1 do target, verificam o monitor de alimentação e preservam conflitos de GPIO nas duas ordens de registro, sem recurso parcial. Inspeção das validações e do delta, builds dos consumidores materiais. |
| ADC1-AC-005 | 007, 008, 009 | Confrontar resolução, atenuação e parametrização com o framework/SoC; documentar limites distintos do clássico e uso explícito dos campos. Presets e defaults clássicos permanecem iguais, salvo o tipo do argumento. Inspeção de código, fontes oficiais e instruções de configuração. |
| ADC1-AC-006 | 004, 007, 011 | Todas as chamadas versionadas usam o enum ou adaptação verificada do símbolo de placa; exemplos NTC/corrente/tensão e build canônico continuam construíveis. Busca de consumidores, inspeção e builds. |
| ADC1-AC-007 | 001, 007, 012 | Componentes ADC constroem nas versões Arduino 2.x/3.x efetivamente usadas e nos targets disponibilizados por elas, sem exigir novos serviços do runtime. Matriz registra sucesso, falha ou não execução por alvo; falha/ausência não equivale a suporte validado. |

## 6. Compatibilidade e migração

A quebra de fonte é deliberada e limitada aos dois presets NTC. Chamadas como
`NTC_100K_B3950(0)` precisam usar o membro correspondente ao GPIO0 do enum do
target. O identificador concreto será documentado junto ao header público.
Não se mantém overload inteiro, pois isso anularia a restrição aprovada.

Consumidores que obtêm GPIO dinamicamente podem continuar preenchendo uma
configuração por campos e passá-la ao factory. Isso não altera as assinaturas
de criação nem elimina sua validação. Consumidores externos devem receber
instrução de migração; seus repositórios não são alterados nesta transação.

A entrega da biblioteca deve identificar esta mudança como incompatibilidade
de fonte dos presets. Número de release, tag e publicação permanecem decisões
separadas; não são operações desta especificação.

## 7. Validações e permissões

Nenhum artefato de teste automatizado integra esta versão. Os critérios de
tipo e rejeição podem ser confrontados por inspeção; não autorizam criação
implícita de suítes ou fixtures. Testes existentes permanecem no recorte e nas
permissões que já os governam.

Durante a implementação autorizada, executar `pio run -e esp32_dev`,
`pio run -e example_environment_ntc_mcb_r1`,
`pio run -e example_current_sensor_mcb_r1` e
`pio run -e example_voltage_sensor_mcb_r1`. Acrescentar builds proporcionais
dos componentes alterados para C3 e demais targets disponibilizados pelo
framework, registrando comandos e versões. Configurações de build necessárias
à matriz podem ser acrescentadas no recorte, sem configurações privadas ou
novas funcionalidades de aplicação. Compilação de componentes não pode ser
relatada como build completo do runtime.

Nesta atuação de Autoria/Análise, executar somente inspeção, integridade
textual e guarda documental. Testes, instrumentação, upload, monitor, hardware,
deploy e publicação exigem ordem operacional própria. A evidência física
posterior deve distinguir aceitação dos GPIOs 0/1 no C3 com Wi-Fi ativo da
exatidão das leituras do circuito; nenhuma delas decorre apenas de build.

## 8. Fontes técnicas e governança

- `src/Platform/Arduino/Sensors/NtcTemperatureSensor.*`;
- `src/Infra/Factories/SensorFactory.cpp`;
- `src/Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.cpp`;
- `src/App/Builders/Builders/CapabilitiesBuilder.cpp`;
- `src/Contracts/Sensors/CurrentSensorTypes.h` e `VoltageSensorTypes.h`;
- `examples/executable/environment_ntc/` e ambientes dos três sensores;
- headers oficiais `soc/adc_channel.h` e implementação Arduino de ADC da
  versão selecionada;
- [Arduino-ESP32: ADC](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html);
- [Arduino-ESP32: targets](https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html#supported-socs);
- [ESP32-C3: datasheet](https://documentation.espressif.com/esp32-c3_datasheet_en.html).

O enquadramento local informado pelo Arquiteto é EKOM 4.7; diretrizes e parte
do mapa ainda referenciam 4.6. Os perfis centrais consultados anunciam 5.0.
Não se realiza migração de governança nesta especificação. Não foi localizada
qualificação formal Repository Readiness nem contrato de engenharia aprovado
com versão própria; essa situação deve ser distinguida da prontidão funcional
da tarefa e não é sanada por seu relatório. A entrada de implementação deve
observar a governança aplicável e a ordem explícita do Arquiteto.

A Autoria e a Análise são realizadas pelo mesmo agente, por ordem combinada;
não se afirma revisão independente. Nenhuma implementação ou conclusão da
entrega é autorizada por este registro.
