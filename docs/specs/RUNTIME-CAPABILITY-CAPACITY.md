# Especificação — Capacidade configurável de capabilities do runtime

**ID:** `IOTSSC-RUNTIME-CAPABILITY-CAPACITY`

**Classe da fonte:** Normativa

**Versão:** 0.2

**Estado normativo:** Vigente [`Active`]

**Estado da implementação:** Validada [`Validated`]

**Estado da entrega:** Pronta para integração [`Ready for Integration`]

**Revisão de implementabilidade:** Pronta [`Ready`]

**Relação normativa:** Corrige [`Corrects`]
`IOTSSC-RUNTIME-CAPABILITY-CAPACITY@0.1`; permanece Nova [`New`] e altera [`Amends`] `IOTSSC-RUNTIME`,
`IOTSSC-PUBLIC-API`, `IOTSSC-BINARY-COMMAND-STATE@0.6` e
`IOTSSC-HW-EXAMPLES@1.1`; preserva `IOTSSC-POWER-ENERGY-CAPABILITY@0.3`,
`IOTSSC-INA3221-SENSORS@0.2`, `IOTSSC-CURRENT-SENSOR@0.6` e
`IOTSSC-VOLTAGE-SENSOR@0.1`

## 1. Objetivo e contexto

Substituir o limite universal e duplicado de oito capabilities por uma
capacidade estática configurável em build, mantendo oito como default e
permitindo doze no environment `ESP32_MCB01`.

A mudança deve permitir que a aplicação MCB01 registre e processe as nove
capabilities de controle, temperatura, tensão, corrente, potência e energia do
seu cenário fotovoltaico. Deve também manter slots, bookkeeping, arena e
persistência binária coerentes, preservar estados NVS gravados pelo formato
vigente e corrigir a composição PV para que `pv-power-1` consuma o mesmo
adapter de tensão conduzido por sua capability pública.

O limite continua determinado em compilação. Não há registro tardio, realocação
dinâmica ou crescimento durante a execução.

## 2. Relações de autoridade

- **Altera [`Amends`] `IOTSSC-RUNTIME`:** substitui somente `RUN-012` quanto
  ao número fixo. Configuração anterior a `setup()` e processamento
  cooperativo permanecem invariantes.
- **Altera [`Amends`] `IOTSSC-PUBLIC-API`:** substitui `API-005` pelo default
  oito, máximo suportado doze e seleção explícita por build; preserva
  assinaturas, ownership e falha observável de `API-006`.
- **Altera [`Amends`] `IOTSSC-BINARY-COMMAND-STATE@0.6`:** amplia de oito para
  doze o formato persistente suportado e acrescenta migração do snapshot
  versão 2, sem mudar identidade, semântica binária, escrita assíncrona ou
  isolamento do namespace NVS.
- **Altera [`Amends`] `IOTSSC-HW-EXAMPLES@1.1`:** permite que a aplicação
  oficial do `ESP32_MCB01` exceda oito somente pelo perfil autorizado e exige
  que o cenário seja versionado e construível.
- **Preserva `IOTSSC-POWER-ENERGY-CAPABILITY@0.3`:** a aplicação continua
  responsável pelo lifecycle dos sensores; a capability apenas consome seus
  snapshots.
- **Preserva `IOTSSC-INA3221-SENSORS@0.2`:** dispositivo e adapters continuam
  externamente possuídos, compartilhados por referência e conduzidos pelas
  respectivas capabilities.
- **Preserva `IOTSSC-CURRENT-SENSOR@0.6` e
  `IOTSSC-VOLTAGE-SENSOR@0.1`:** medição, estados, cadência, formatação e
  publicação não mudam.

A direção arquitetural e seus trade-offs estão registrados em
`docs/adr/ADR-0001-CONFIGURABLE-RUNTIME-CAPACITY.md`.

## 3. Escopo

- configuração em build da quantidade de capabilities;
- default de oito e perfil `ESP32_MCB01` com doze;
- limite superior suportado de doze nesta versão;
- fonte única para slots, destrutores e bookkeeping dependente da capacidade;
- coerência da capacidade de adapters e segurança da arena estática;
- rejeição atômica e diagnosticada acima do limite ou sem memória;
- formato NVS com doze registros binários e migração do formato anterior;
- aplicação MCB01 versionada com nove capabilities registradas antes de
  `setup()`;
- compartilhamento da instância de tensão PV entre sua capability própria e
  `pv-power-1`;
- testes automatizados diretamente vinculados aos critérios desta versão;
- builds `ESP32_MCB01` e `esp32_dev`;
- procedimento posterior de validação física da aplicação MCB01.

## 4. Fora de escopo

- registro, remoção ou redimensionamento de capabilities depois de `setup()`;
- alocação dinâmica de capabilities, adapters ou arena;
- capacidade maior que doze;
- mudança de API, payload, type, estado, cálculo ou cadência de capability;
- mudança elétrica, calibração ou faixa dos sensores;
- novo modelo de ownership ou lifecycle de sensor;
- mudança de MQTT, provisioning, OTA, factory reset ou settings;
- persistência de capabilities não binárias;
- migração ou alteração do domínio NVS de settings;
- suporte a ESP8266 ou promoção de ESP-IDF nativo;
- upload, deploy, release ou operação externa durante a implementação.

## 5. Contrato de configuração

### 5.1 Capacidade do runtime

- **CAP-001:** deve existir uma única configuração de build para a capacidade
  máxima de capabilities de `SmartSysApp`; seu nome público recomendado é
  `IOTSMARTSYS_MAX_CAPABILITIES`.
- **CAP-002:** na ausência de configuração explícita, a capacidade deve ser
  exatamente oito.
- **CAP-003:** o environment `ESP32_MCB01` deve configurar a capacidade como
  exatamente doze.
- **CAP-004:** valores menores que um ou maiores que doze devem falhar em
  compilação com diagnóstico compreensível; não pode haver clamp ou fallback
  silencioso.
- **CAP-005:** o valor configurado deve dimensionar os slots de capability e
  seus destrutores e deve ser o valor entregue ao `CapabilitiesBuilder`, sem
  literal divergente.
- **CAP-006:** estruturas auxiliares indexadas pela quantidade de capabilities,
  inclusive o bookkeeping de identidades de sensores, não podem possuir
  capacidade menor nem escrever além de seus limites.
- **CAP-007:** a capacidade de adapters deve ser explícita, estática e
  suficiente para doze capabilities que legitimamente exijam um adapter cada,
  sem reduzir o default de consumers existentes.

### 5.2 Lifecycle, memória e falhas

- **CAP-008:** toda capability continua sendo registrada antes de
  `SmartSysApp::setup()`; tentativa posterior deve retornar `nullptr`, produzir
  diagnóstico e não alterar o runtime.
- **CAP-009:** o registro até a capacidade configurada ainda pode falhar por
  configuração inválida, identidade, conflito de recurso, adapter ou arena;
  essas falhas permanecem distintas de esgotamento de slots e observáveis.
- **CAP-010:** exceder a capacidade configurada deve retornar `nullptr`, emitir
  diagnóstico de slots esgotados e não consumir slot, destrutor, adapter,
  arena, GPIO ou identidade.
- **CAP-011:** construção parcialmente falha deve restaurar contadores e offset
  da arena ao estado anterior e destruir somente objetos já construídos pela
  tentativa.
- **CAP-012:** a arena permanece estática. O perfil MCB01 deve possuir espaço
  comprovado para sua composição normativa de nove capabilities e adapters,
  sem heap como fallback.
- **CAP-013:** `CapabilityManager` deve receber a contagem efetivamente
  registrada e executar exatamente uma chamada de `setup()` por capability;
  cada ciclo elegível de `handle()` deve percorrer as nove na ordem registrada.
- **CAP-014:** o aumento da travessia não pode introduzir espera ativa, `delay()`
  ou task exclusiva por capability.

## 6. Persistência NVS e compatibilidade

- **CAP-015:** o formato persistente novo deve possuir versão distinta da
  versão 2 e exatamente doze posições para estados de
  `BinaryCommandCapability`.
- **CAP-016:** o formato persistente deve permanecer fixo em doze registros em
  todos os environments suportados, ainda que a capacidade do runtime seja
  oito, evitando layout NVS dependente do environment.
- **CAP-017:** ao encontrar um blob versão 2 com oito registros, o provider deve
  validar tamanho, versão, flags, terminação, identidades e checksum conforme o
  contrato anterior antes de aceitar qualquer registro.
- **CAP-018:** snapshot legado válido deve ser convertido integralmente em
  memória: os até oito registros ativos preservam identidade e estado, e as
  quatro posições restantes iniciam vazias.
- **CAP-019:** snapshot legado inválido, truncado, corrompido ou semanticamente
  inválido deve ser rejeitado integralmente; nenhuma entrada parcial pode ser
  aplicada ou copiada.
- **CAP-020:** a leitura/migração não pode apagar a partição NVS, modificar o
  namespace de settings, abortar ou reiniciar o dispositivo.
- **CAP-021:** a persistência do formato novo deve ocorrer somente pelo escritor
  assíncrono vigente. A leitura de um legado válido não autoriza write ou commit
  síncrono no caminho de boot.
- **CAP-022:** ausência de blob continua significando ausência de estado e não
  falha; erro real de NVS continua observável pelo contrato vigente.
- **CAP-023:** até doze identidades binárias distintas devem poder ser
  preservadas no novo snapshot; a 13ª deve retornar overflow sem modificar o
  último snapshot válido nem substituir outra identidade.
- **CAP-024:** checksum e validação do formato novo devem cobrir todo o cabeçalho
  e as doze posições, inclusive as não utilizadas.

## 7. Aplicação `ESP32_MCB01`

- **CAP-025:** a fonte da aplicação MCB01 usada como evidência deste contrato
  deve ser versionada e selecionada de forma determinística pelo environment;
  arquivo local ignorado não constitui entrega nem evidência reproduzível.
- **CAP-026:** a aplicação deve registrar, antes de `setup()`, as nove
  capabilities correntes: corrente PV, tensão PV, potência/energia PV, switch
  do controlador, fan do controlador, temperatura, tensão de carga da bateria,
  corrente de carga e corrente de descarga.
- **CAP-027:** cada registro deve verificar ou preservar de modo observável o
  retorno de falha; a aplicação não pode anunciar setup completo após rejeição
  silenciosa de capability obrigatória.
- **CAP-028:** `pv-voltage-1` deve registrar por referência exatamente o mesmo
  `pvVoltageSensorAdapter` entregue a `pv-power-1`; nenhum segundo adapter pode
  adquirir o mesmo canal ADC para essa composição.
- **CAP-029:** a capability de tensão deve conduzir `setup()` e `handle()` do
  adapter PV compartilhado antes de `pv-power-1` consumir seu snapshot na ordem
  cooperativa de registro.
- **CAP-030:** os adapters INA3221 de carga e descarga continuam compartilhando
  um único `INA3221Device`; cada capability conduz somente seu adapter recebido
  por referência.
- **CAP-031:** com snapshots válidos de tensão e corrente, `pv-power-1` deve
  deixar `NOT_READY` e publicar potência conforme seu contrato vigente;
  `battery-discharging-current` deve deixar `NOT_READY` após sua primeira
  leitura finita elegível.

## 8. Guarda documental

- **CAP-032:** a guarda estrutural EKOM deve ser executada sobre o repositório
  inteiro. Todo achado introduzido ou materialmente alterado pelos arquivos do
  recorte reprova a entrega. Achados preexistentes, reproduzíveis na baseline e
  sem relação material com esta especificação não bloqueiam o recorte, mas o
  comando, seu código de saída e esses achados devem ser registrados
  separadamente como falha preexistente; não podem ser omitidos, corrigidos sem
  autorização nem apresentados como validação global aprovada.

## 9. Condições de borda

- capacidade default sem macro: oito aceita, nove rejeita;
- capacidade MCB01: doze aceita, treze rejeita;
- nona capability válida com arena insuficiente: falha de arena observável, não
  sucesso parcial;
- nove capabilities registradas e uma inválida: setup completo não pode ser
  anunciado como se todas estivessem presentes;
- snapshot NVS v2 vazio, completo com oito, parcialmente ocupado ou corrompido;
- snapshot novo vazio, completo com doze, 13ª identidade e corrupção em cada
  região coberta;
- reboot depois de migração lida e antes de qualquer novo commit: o legado
  válido continua restaurável;
- ausência ou falha de um sensor composto: somente os estados já contratados
  pelas capabilities afetadas se aplicam, sem bloquear o loop.

## 10. Critérios de aceite

| Critério | Requisitos | Cenário e ação | Resultado observável | Evidência |
|---|---|---|---|---|
| CAP-AC-001 | CAP-001 a CAP-006 | Construir o runtime sem override, registrar oito capabilities válidas e tentar a nona. | Oito registros têm sucesso; o nono retorna `nullptr`, registra slots esgotados e não altera contadores ou arena. | Teste automatizado do builder e inspeção da configuração. |
| CAP-AC-002 | CAP-003 a CAP-007, CAP-010 | Construir o perfil de doze, registrar doze capabilities/adapters válidos e tentar a 13ª. | Doze registros têm sucesso sem overflow auxiliar; a 13ª falha atomicamente. | Teste automatizado com capacidade 12. |
| CAP-AC-003 | CAP-008, CAP-013, CAP-014 | Registrar nove capabilities antes de `setup()`, instrumentar lifecycle e tentar outro registro depois de `setup()`. | As nove recebem setup e handle cooperativo; o registro tardio é rejeitado sem alterar o manager. | Teste de integração do lifecycle. |
| CAP-AC-004 | CAP-009 a CAP-012 | Forçar separadamente esgotamento de slot, adapter e arena durante registro. | Cada causa é diagnosticada, não deixa objeto parcial e preserva contadores, identidade e offset anteriores. | Teste automatizado com arena/adapters controlados. |
| CAP-AC-005 | CAP-015 a CAP-022, CAP-024 | Carregar snapshots v2 válidos com zero/oito registros e variantes inválidas; reinicializar antes e depois do primeiro commit novo. | Legados válidos restauram todos os estados sem write síncrono; inválidos não aplicam nada; settings e NVS global não são apagados. | Teste de migração com NVS fiel ou emulada e contadores por operação. |
| CAP-AC-006 | CAP-015, CAP-016, CAP-023, CAP-024 | Persistir doze identidades no formato novo, tentar a 13ª, corromper bytes do cabeçalho e das posições. | Doze sobrevivem ao reboot; a 13ª não substitui dados; toda corrupção coberta é rejeitada integralmente. | Teste de round-trip, overflow e corrupção. |
| CAP-AC-007 | CAP-025 a CAP-030 | Construir a aplicação versionada no environment `ESP32_MCB01` e inspecionar registros e referências. | As nove capabilities são aceitas; existe um único adapter de tensão PV compartilhado e um único dispositivo INA3221. | Build, inspeção do ELF/delta e instrumentação de registro. |
| CAP-AC-008 | CAP-027, CAP-029, CAP-031 | Executar em hardware com entradas finitas e conectividade operacional. | Não há rejeição de capability; descarga e potência deixam `NOT_READY` e publicam valores coerentes; setup completo só é anunciado após todos os registros obrigatórios. | Log serial e observação MQTT produzidos pelo Arquiteto. |
| CAP-AC-009 | CAP-001 a CAP-032 | Executar os builds canônicos, `git diff --check` e a guarda EKOM global; comparar os achados da guarda com a baseline anterior ao recorte. | Os builds e `git diff --check` terminam com código zero. A guarda não apresenta achado novo ou materialmente alterado atribuível ao recorte. Se continuar não zero apenas por achados preexistentes reproduzidos, a entrega registra separadamente comando, código e achados, sem declarar a guarda global aprovada. | Saídas terminais completas, delta inspecionado e reconciliação identificável da falha preexistente. |

## 11. Testes e permissões

Esta versão exige criação ou alteração de testes automatizados somente para
`CAP-AC-001` a `CAP-AC-006`. Esses testes integram a futura implementação
porque exercitam limites, atomicidade, lifecycle e migração introduzidos por
`CAP-001` a `CAP-024`.

`CAP-AC-007` usa build e inspeção. `CAP-AC-008` exige upload, monitor serial,
hardware energizado e MQTT; essas operações não são autorizadas pela ordem de
implementação e dependem de ordem operacional específica do Arquiteto.

Criar ou alterar testes não autoriza sua execução. Build é intrínseco à futura
implementação; execução de testes e hardware permanece separada.

## 12. Correção da versão 0.2

A versão 0.1 exigia código zero da guarda EKOM global em `CAP-AC-009`, embora
a baseline já contivesse achados documentais fora do recorte. Isso tornava o
critério insatisfazível sem ampliar a implementação para reescrever fontes
históricas e experimentais não relacionadas.

Por decisão do Arquiteto, a versão 0.2 acrescenta `CAP-032` e corrige
`CAP-AC-009`: a guarda continua obrigatória e sua falha continua preservada,
mas a conformidade do recorte depende de não introduzir ou alterar achados nos
arquivos sob sua autoridade. A correção não muda capacidade, lifecycle, arena,
persistência, migração, aplicação MCB01 nem permissões operacionais.

## 13. Estado e encaminhamento

A versão 0.2 está Vigente [`Active`], com implementação Validada [`Validated`],
Análise de Implementabilidade Pronta [`Ready`] e entrega Pronta para integração
[`Ready for Integration`]. A revisão classificou o recorte como aderente com
limitações de evidência. O Arquiteto considerou as evidências suficientes,
aceitou o risco residual dos testes e da validação física não executados,
determinou o encerramento da especificação e ordenou sua promoção para `main`.
