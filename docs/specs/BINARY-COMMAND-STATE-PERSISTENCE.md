# Especificação — Persistência do estado de comandos binários

**ID:** IOTSSC-BINARY-COMMAND-STATE

**Classe da fonte:** Normativa

**Versão:** 0.4

**Estado normativo:** Proposta [`Proposed`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Não pronta [`Not Ready`]

**Revisão de implementabilidade:** Pendente de revisão [`Pending Review`]

**Relação normativa:** Corrige [`Corrects`]
`IOTSSC-BINARY-COMMAND-STATE@0.3`

## 1. Objetivo e contexto

Fazer com que toda capability baseada em `BinaryCommandCapability` restaure no
boot o último estado binário registrado e o aplique ao hardware, em vez de
permanecer no valor default após cada reinicialização.

Hoje, `BinaryCommandCapability` começa com o valor de desligado definido pelo
tipo concreto. Durante o `setup()`, o adapter de comando é inicializado e seu
estado corrente passa a ser o estado lógico da capability. Como não existe
persistência própria estável e segura para esse domínio, uma reinicialização
pode perder a última intenção efetivamente aplicada e, nos artefatos
experimentais já presentes na branch, introduzir regressões de provisionamento,
integridade NVS e vocabulário da valve.

A intenção confirmada pelo Arquiteto para o comportamento funcional é:

- registrar o estado em NVS quando o estado binário confirmado mudar, segundo a
  política de origem ainda sujeita às decisões pendentes desta versão;
- manter a leitura de boot leve;
- aplicar, durante a inicialização, o último estado validamente registrado para
  cada capability abrangida;
- preservar o grafo único de serviços, o provisionamento BLE e os settings
  existentes.

A versão 0.4 incorpora a avaliação consultiva registrada em `EKM-CHG-0018`. Ela
corrige o contrato 0.3 sem reutilizar o estado `Implementable` daquela versão.
As seções históricas 13 a 15 permanecem apenas como evidência contestada.

## 2. Fatos observados

- `BinaryCommandCapability` concentra os comandos `toggle`, `turnOn`,
  `turnOff` e `power`, além da sincronização do estado lógico com o adapter.
- `SwitchCapability`, `SwitchPlugCapability`, `LightCapability`,
  `LEDCapability` e `ValveCapability` derivam de
  `BinaryCommandCapability`.
- os valores binários não são textualmente iguais em todos os tipos:
  `ValveCapability` usa `closed` e `open`, enquanto os demais tipos observados
  usam `off` e `on`;
- o nome definitivo da capability é estabelecido pelo builder antes de
  `CapabilityManager::setup()`;
- o runtime admite no máximo oito capabilities;
- o projeto já separa contratos de Core, provedores de plataforma e registro de
  serviços Espressif;
- a implementação Espressif vigente já usa NVS para settings e, na branch
  atual, também há artefatos experimentais de estado binário; esses artefatos
  não constituem implementação aprovada desta versão;
- a API pública de configuração expõe `capability_name` como ponteiro/string
  sem limite documental único e inequívoco; o builder possui buffer local de
  geração automática, mas nomes fornecidos externamente não compartilham
  automaticamente o mesmo teto do storage experimental;
- o build base e environments ESP32 aplicáveis usam
  `-fno-threadsafe-statics`; portanto a garantia de "magic statics" thread-safe
  do C++11 não está disponível nesta toolchain;
- `ServiceManager::init()` e `ServiceManager::instance()` declaram, cada um,
  um `static ServiceManager` local distinto;
- o callback de conclusão do provisionamento BLE chama
  `ServiceManager::instance()`, ignora o retorno de
  `SettingsManager::save()` e agenda restart com log de sucesso de forma
  incondicional;
- o storage binário experimental, em falha de `nvs_flash_init`, executa
  `nvs_flash_erase()` sob `ESP_ERROR_CHECK`, o que pode apagar a partição NVS
  inteira e abortar o runtime;
- o snapshot experimental valida tamanho, versão e checksum, mas não valida de
  forma completa domínio e terminação de `used`, `isOn`, `capability_name` e
  `type` antes de usar as strings;
- o fallback de `restoreFromStorage()` ainda pode atualizar estado a partir de
  `getStateValue()` sem interpreter quando o restore interpretado não se
  completa, expondo `off`/`on` onde a valve exige `closed`/`open`;
- a execução experimental das versões 0.1/0.2 demonstrou que testes
  estruturalmente presentes ainda podiam aceitar valve sem interpreter, LED
  fora do protocolo comum, snapshot sem integridade suficiente, falhas NVS
  indistintas de ausência, identidades truncadas ou rejeitadas por limite
  interno menor, gravação síncrona de alta frequência em `blink` e compilação
  com zero casos executados;
- a avaliação consultiva `EKM-CHG-0018` contestou o fundamento da revisão
  `Implementable` da versão 0.3, em especial a premissa de statics thread-safe,
  a suficiência causal da correção do singleton para o provisioning, o gate
  `esp32_dev` sem caminho de aprovação e a cópia desnecessária de metadados
  Git.

### 2.1 Defeito encontrado na validação da versão 0.2

Durante a validação em ESP32-S3 do firmware 1.0.54, um provisionamento BLE
sem settings previamente armazenados recebeu uma configuração fragmentada em
escritas GATT de 9, 96, 96, 48 e 3 bytes. A escrita final de 3 bytes continha o
encerramento da transmissão. Antes de registrar conclusão ou persistência, o
dispositivo abortou com `Stack canary watchpoint triggered (BTC_TASK)` e
reiniciou. No boot seguinte, `SettingsManager` retornou `NotFound(6)` e o
dispositivo entrou novamente em provisionamento.

O backtrace foi decodificado contra o ELF local do environment `esp32s3_ia`,
com a mesma versão 1.0.54, e resolveu integralmente os frames do projeto. A
cadeia observada foi:

```text
vsnprintf
→ ArduinoSerialLogger::logf
→ EspNvsBinaryCapabilityStateProvider::loadSnapshot
→ EspressifPlatformServiceRegistrar::registerPlatformServices
→ ServiceManager::ServiceManager
→ ServiceManager::instance
→ callback de ProvisioningController
→ ProvisioningManager::handleNewConfig
→ BleProvisioningChannel::onConfigWrite
→ BleProvisioningChannel::gattsEventHandler
→ btc_gatts_cb_to_app
→ btc_thread_handler
```

As fontes mostram a seguinte causa parcial e necessária:

- `SmartSysApp` obtém seu grafo de serviços por `ServiceManager::init()`;
- `ServiceManager::init()` e `ServiceManager::instance()` declaram, cada um,
  um `static ServiceManager` local, formando duas instâncias distintas;
- o callback de conclusão do provisionamento chama
  `ServiceManager::instance()` no caminho síncrono iniciado pelo callback GATT;
- o primeiro uso desse segundo accessor constrói outro `ServiceManager`,
  registra novamente os serviços de plataforma e chama novamente
  `loadSnapshot()`;
- o ESP32-S3 usa pilha de 3072 bytes para a `BTC_TASK`; o encadeamento adicional
  alcança `ArduinoSerialLogger::logf()` e `vsnprintf()`, onde o canário acusa a
  pilha já excedida;
- o backtrace não alcança `SettingsManager::save()`. O reboot ocorre antes da
  gravação e do commit dos settings recebidos.

A duplicação do grafo explica o panic observado. Ela não prova, por si só, que
seja a única correção necessária para o fluxo de provisioning: o callback
ainda ignora falha de `save()` e registra sucesso/restart de forma
incondicional. Os erros posteriores sobre configuração ou partição de core dump
não integram a cadeia causal do abort.

Esses fatos descrevem a implementação atual e sustentam a proposta; não
constituem por si mesmos requisitos legados aprovados.

## 3. Escopo

- todas as instâncias cujo comportamento concreto deriva de
  `BinaryCommandCapability`;
- identificação estável do registro por capability, reconciliada com a API
  pública de nomeação;
- representação persistida dos dois estados lógicos suportados;
- leitura, validação estrutural/semântica e cache do snapshot durante o boot;
- aplicação do estado restaurado ao hardware durante `setup()`;
- gravação e commit em NVS após mudança lógica confirmada, segundo a política
  de origem autorizada;
- tratamento de ausência, corrupção, incompatibilidade e falha de storage sem
  erase global da NVS e sem abort por checagem fatal no fluxo binário;
- logs diagnósticos sem dados privados;
- preservação da identidade única do grafo de serviços e da ordem de
  inicialização única antes de acessos concorrentes, considerando
  `-fno-threadsafe-statics`;
- preservação do provisionamento BLE e da persistência de settings, com restart
  e status de sucesso condicionados ao sucesso de `SettingsManager::save()`;
- oráculo de cooperatividade observável para write/commit;
- testes automatizados do Core, do provedor NVS, da valve e do provisioning
  após reboot;
- build do runtime Arduino sobre ESP32 no environment de gate que o Arquiteto
  autorizar.

## 4. Fora de escopo

- persistir estados de capabilities que não derivem de
  `BinaryCommandCapability`;
- persistir comandos transitórios como o texto `toggle`;
- alterar nomes públicos de capabilities, comandos, tipos ou estados, salvo o
  limite explícito de identidade se o Arquiteto o autorizar para reconciliar
  storage e API;
- alterar defaults públicos das capabilities;
- alterar o limite de oito capabilities;
- sincronizar o estado persistido com API remota, MQTT ou settings do
  dispositivo;
- criar histórico de estados, contador de acionamentos ou telemetria;
- decidir unilateralmente debounce, atraso, batching ou exclusão de `blink`
  enquanto `BCS-DEC-002` permanecer pendente;
- aumentar a pilha da `BTC_TASK` como substituto para corrigir a duplicação do
  grafo de serviços;
- reorganizar genericamente o threading, protocolo ou arquitetura do
  provisionamento BLE além do necessário para satisfazer os requisitos de
  identidade do grafo e de persistência condicional dos settings;
- alterar o fluxo de factory reset enquanto `BCS-DEC-001` permanecer pendente;
- suportar ESP8266;
- promover ESP-IDF a runtime suportado;
- upload, release ou deploy;
- copiar para a fonte normativa metadados Git que o repositório já preserve e
  que não expliquem desvio material.

## 5. Solução arquitetural proposta

### 5.1 Fronteira de persistência

Deve existir um contrato de storage específico para estados binários no Core e
uma implementação Espressif baseada em NVS. A implementação de
`BinaryCommandCapability` não pode incluir nem chamar diretamente APIs
`Preferences`, `nvs_*` ou headers específicos de plataforma.

O serviço deve ser registrado no bootstrap de plataforma e disponibilizado às
capabilities pelo mecanismo de composição já usado pelo projeto. Construtores
e formas públicas de registro existentes devem permanecer compatíveis.

Essa proposta adiciona uma fronteira de persistência porque não há precedente
equivalente aprovado para estado de capabilities. Ela preserva o padrão vigente
de contrato no Core, implementação em `Platform/Espressif` e composição no
registro de serviços; não autoriza camada estrutural adicional fora dessas
responsabilidades.

### 5.2 Modelo de armazenamento

O provedor Espressif deve usar namespace NVS próprio, sem compartilhar nem
reescrever o blob de settings. O conteúdo deve ser um snapshot compacto e
versionado, com no máximo oito registros ativos.

Cada registro deve conter:

- identidade composta pelo `capability_name` definitivo e pelo `type`;
- estado semântico binário `off` ou `on`;
- informação suficiente para rejeitar formato truncado, incompatível,
  semanticamente inválido ou corrompido.

Antes de qualquer `strcmp`, cópia para string ou aplicação de estado, o
provedor deve validar estrutural e semanticamente o snapshot carregado,
incluindo no mínimo:

- versão suportada;
- quantidade de registros ativos compatível com o limite de oito;
- domínio de flags como `used` e `isOn`;
- terminação nula integral de `capability_name` e `type` dentro do próprio
  campo;
- ausência de registros ativos com identidade vazia;
- verificação de integridade que cubra o cabeçalho e todos os registros
  ativos.

Tamanho e versão compatíveis, isoladamente, não comprovam integridade nem
validade semântica. Registro que falhe qualquer validação torna o snapshot
inválido para uso; nenhum de seus valores pode ser aplicado.

#### Identidade e API pública

A identidade deve ser preservada integralmente para todo nome e tipo aceitos
pela configuração pública vigente. São reprovações:

- truncamento silencioso;
- colisão por prefixo;
- gravação parcial;
- rejeição causada apenas por limite interno de storage menor do que o limite
  efetivamente aceito pela API/configuração pública.

Se a implementação precisar de capacidade máxima de campo, esse limite deve ser
o mesmo contrato observável da API pública de nomeação/configuração. Não é
aceitável um storage "interno" mais restritivo do que o caminho público que
registra a capability.

O estado semântico deve ser convertido para `_offValue` ou `_onValue` pela
própria `BinaryCommandCapability`. Assim, o mesmo storage atende vocabulários
como `off`/`on` e `closed`/`open` sem persistir comandos transitórios.

O snapshot deve ser obtido por no máximo uma chamada que copie o blob da NVS
para memória durante a inicialização do serviço. Uma consulta de metadado para
obter o tamanho do blob não conta como leitura de dados; qualquer chamada
adicional que copie conteúdo conta. Consultas posteriores no mesmo boot devem
ocorrer somente no cache em memória. O ciclo cooperativo de `handle()` não pode
fazer leituras NVS.

Uma gravação deve atualizar o snapshot em memória, persistir o blob e executar
o commit NVS. Gravação e leitura não podem tocar o namespace ou a chave de
settings.

#### Recuperação NVS do storage binário

No fluxo do storage binário é proibido:

- apagar a partição NVS global ou qualquer namespace alheio como recuperação;
- usar `ESP_ERROR_CHECK`, abort, `abort()`, restart ou equivalente para tratar
  falha de init/open/read/write/commit desse provedor.

Falha de storage binário deve devolver resultado observável, registrar
diagnóstico e preservar o runtime. A recuperação legítima limita-se a tratar o
snapshot como ausente/inválido para o domínio binário, sem destruir settings ou
outros dados persistidos do dispositivo.

### 5.3 Identidade e ciclo de vida

A identidade somente pode ser consultada depois que o builder tiver definido o
nome definitivo da capability. Alterar `capability_name` ou `type` entre boots
cria outra identidade e não pode aplicar automaticamente o registro da
identidade anterior.

O fluxo de inicialização proposto é:

```text
bootstrap da plataforma
→ inicialização única e completa de ServiceManager antes de tasks/callbacks concorrentes
→ inicialização do storage e leitura única do snapshot NVS
→ validação estrutural e semântica do snapshot
→ construção e nomeação das capabilities
→ setup do adapter de comando
→ consulta em cache por (capability_name, type)
→ registro válido encontrado
  → conversão para o valor binário concreto
  → aplicação pelo mesmo caminho de interpretação usado por um comando normal
  → leitura de confirmação pelo mesmo caminho de interpretação do estado
  → atualização/publicação do estado confirmado
→ registro ausente ou inválido
  → preservação do fluxo e do default vigentes, sempre via interpreter quando a capability o exige
```

O estado persistido não pode ser anunciado como aplicado antes de o adapter
aceitar o comando e sua leitura confirmar o valor correspondente.

Para `ValveCapability`, restaurar `on` significa solicitar `open` à capability,
o interpreter deve entregar `on` ao adapter, o adapter deve reportar `on` e o
interpreter deve converter a leitura confirmada em `open`. O fluxo equivalente
de `off` usa `closed` na capability e `off` no adapter. Enviar diretamente
`open` ou `closed` ao adapter não satisfaz esta especificação. Qualquer
fallback de restore, sync ou default da valve também deve percorrer o
interpreter antes de atualizar, publicar ou persistir estado lógico; ler
`getStateValue()` e promover `off`/`on` ao estado lógico da valve é reprovação.

### 5.4 Registro de mudanças

Toda transição lógica confirmada entre os dois valores binários, cuja origem
esteja autorizada pela política vigente, deve solicitar a atualização e o
commit do snapshot. Isso inclui, no protocolo comum:

- comandos remotos;
- chamadas públicas como `turnOn`, `turnOff`, `power` e `toggle`;
- sincronização com mudança observada no adapter;
- comportamento automático de classe derivada, ressalvado `BCS-DEC-002` para
  `blink`.

Repetir o mesmo valor não constitui mudança e não pode gerar nova gravação. O
texto `toggle` nunca deve ser persistido; deve ser registrado apenas o estado
resultante confirmado.

O caminho comum de `BinaryCommandCapability` deve centralizar a detecção da
transição e a solicitação de persistência. Um override de classe derivada,
inclusive `LEDCapability::handle()`, não pode contornar aplicação, read-back,
atualização do estado lógico, publicação e o protocolo de persistência
aplicável. Uma falha de gravação deve ser registrada em log, mas não pode
reverter um comando já aplicado nem impedir o processamento cooperativo.

Enquanto `BCS-DEC-004` permanecer pendente, a especificação não autoriza nem
proíbe de forma final a execução síncrona ou assíncrona de write/commit. Em
qualquer escolha futura autorizada pelo Arquiteto:

- o ciclo cooperativo deve permanecer contínuo e observável;
- write/commit não podem abortar, reiniciar ou bloquear indefinidamente o
  dispositivo;
- a semântica de "último commit bem-sucedido" após reboot deve permanecer
  testável.

### 5.5 Identidade do grafo de serviços e compatibilidade do provisioning

O precedente arquitetural vigente é o acesso singleton ao grafo composto por
`ServiceManager` e `ServiceProvider`. Os accessors públicos existentes
`ServiceManager::init()` e `ServiceManager::instance()` devem resolver o mesmo
objeto durante todo o boot.

Como a toolchain usa `-fno-threadsafe-statics`, a correção não pode depender de
inicialização concorrente thread-safe de estáticas locais. A solução proposta
exige:

1. inicialização única e completa do `ServiceManager` no bootstrap, antes de
   habilitar ou executar callbacks/tasks que possam chamá-lo concorrentemente;
2. convergência observável dos dois accessors para a mesma instância;
3. registro único dos serviços de plataforma e leitura inicial única do
   snapshot binário por boot.

A forma interna de compartilhar a instância permanece escolha de
implementação, desde que a ordem e a identidade sejam observáveis. Apenas
aumentar a pilha, suprimir o log no qual o canário foi detectado, postergar a
segunda construção ou invocar "magic statics" sob `-fno-threadsafe-statics`
não satisfaz esta especificação.

#### Provisioning e `SettingsManager::save()`

Após a recepção de uma configuração BLE válida:

- o fluxo deve chamar `SettingsManager::save()`;
- restart controlado e status/log de sucesso do provisioning somente podem
  ocorrer se `save()` concluir com sucesso;
- falha de `save()` deve permanecer observável, não pode ser registrada como
  sucesso e não pode descartar a possibilidade de nova tentativa;
- no boot seguinte a um save bem-sucedido, os settings devem ser carregados do
  cache e o bootstrap não pode retornar ao provisionamento por ausência da
  configuração aceita;
- a inclusão do provedor binário não pode introduzir panic, abort, stack
  overflow ou reboot não controlado nesse caminho.

## 6. Requisitos

- **BCS-001:** toda capability derivada de `BinaryCommandCapability` deve
  participar automaticamente da restauração e persistência, sem configuração
  opt-in por tipo concreto.
- **BCS-002:** a identidade persistente deve combinar o `capability_name`
  definitivo e o `type`, sem truncamento silencioso, colisão por prefixo ou
  rejeição causada por limite interno menor que o contrato público de
  nomeação/configuração.
- **BCS-003:** o storage deve representar somente os estados semânticos
  binários `off` e `on`.
- **BCS-004:** cada estado semântico restaurado deve ser convertido para o
  `_offValue` ou `_onValue` do tipo concreto e percorrer o interpreter
  configurado antes de alcançar o adapter.
- **BCS-005:** o provedor deve manter os estados em namespace NVS exclusivo e
  não pode alterar o blob de settings.
- **BCS-006:** o formato persistido deve ser versionado, limitado a oito
  registros ativos e capaz de rejeitar conteúdo incompleto, incompatível,
  semanticamente inválido ou corrompido por validação estrutural, semântica e
  de integridade do cabeçalho e dos registros ativos.
- **BCS-007:** o snapshot deve exigir no máximo uma leitura de dados da NVS por
  boot; buscas por capability devem usar o cache em memória.
- **BCS-008:** `handle()` não pode realizar leitura NVS.
- **BCS-009:** no `setup()`, um registro válido deve ser aplicado ao adapter
  depois da inicialização do hardware e antes de a capability ser considerada
  inicializada.
- **BCS-010:** o valor restaurado somente pode se tornar o estado lógico da
  capability após sucesso do comando e confirmação pela leitura do adapter.
- **BCS-011:** na ausência de registro, o comportamento default vigente deve
  ser preservado e a inicialização deve continuar.
- **BCS-012:** versão desconhecida, conteúdo inválido, corrupção, campo sem
  terminador, flag fora de domínio ou identidade não correspondente devem ser
  tratados como ausência de registro utilizável para a capability afetada, sem
  aplicar valores parciais.
- **BCS-013:** toda transição confirmada autorizada de `off` para `on` ou de
  `on` para `off` deve atualizar o cache, gravar o snapshot e executar commit
  NVS, observada a política pendente de `blink` em `BCS-DEC-002`.
- **BCS-014:** uma tentativa de atribuir novamente o estado lógico corrente não
  pode gerar gravação nem commit.
- **BCS-015:** `toggle` deve persistir somente o valor final confirmado, nunca
  o comando transitório.
- **BCS-016:** mudanças originadas por comandos, API pública, leitura do
  adapter ou automação de classe derivada devem seguir o mesmo protocolo de
  aplicação/read-back/publicação e o protocolo de persistência aplicável, mesmo
  quando a classe derivada sobrescrever `handle()` ou outro ponto do ciclo.
- **BCS-017:** falha ao inicializar, abrir, ler, gravar ou executar commit na
  NVS do storage binário não pode bloquear o loop, abortar ou reiniciar o
  dispositivo nem desfazer o estado de hardware já confirmado.
- **BCS-018:** após falha de persistência, o estado lógico e sua publicação
  devem continuar refletindo o hardware; o último estado registrado continua
  sendo o último commit concluído com sucesso.
- **BCS-019:** a restauração de uma capability não pode alterar outra
  capability, mesmo quando ambas possuem o mesmo estado semântico.
- **BCS-020:** mudança de nome ou tipo entre boots não pode reutilizar
  silenciosamente o registro da identidade anterior.
- **BCS-021:** logs devem permitir distinguir ausência de registro, registro
  inválido, falha de aplicação e cada falha de storage observada, sem converter
  falha de storage em ausência e sem expor settings, credenciais ou conteúdo
  privado.
- **BCS-022:** APIs públicas, defaults, ordem de configuração antes de
  `SmartSysApp::setup()`, processamento cooperativo e limite de oito
  capabilities devem ser preservados, ressalvada eventual publicação explícita
  de limite de identidade autorizada para satisfazer BCS-002.
- **BCS-023:** a implementação Espressif deve operar no runtime Arduino sobre
  ESP32 e não pode degradar o código preparatório para ESP-IDF.
- **BCS-024:** `ServiceManager::init()` e `ServiceManager::instance()` devem
  resolver a mesma instância; a inicialização única deve completar-se antes de
  acessos concorrentes de tasks/callbacks; o registro dos serviços de
  plataforma e a leitura inicial do snapshot binário devem ocorrer uma única
  vez por boot. A correção deve considerar `-fno-threadsafe-statics` e não
  pode depender de inicialização concorrente thread-safe de estáticas locais.
- **BCS-025:** o registro e o uso do provedor de estado binário não podem
  degradar o provisionamento BLE: uma configuração válida deve ser persistida
  com sucesso antes do restart controlado, carregada do cache no boot seguinte
  e não pode provocar stack overflow, panic, abort ou reboot não controlado.
- **BCS-026:** restart controlado e status/log de sucesso do provisioning
  somente podem ocorrer após `SettingsManager::save()` retornar sucesso; falha
  de `save()` permanece observável e não autoriza restart de sucesso nem
  descarta nova tentativa.
- **BCS-027:** o storage binário não pode recuperar-se por erase global da NVS
  nem por `ESP_ERROR_CHECK`/abort no seu fluxo de init/open/read/write/commit.
- **BCS-028:** todo fallback de `ValveCapability` — restore, sync, default pós
  falha de aplicação/read-back — deve percorrer o interpreter antes de
  atualizar, publicar ou persistir o estado lógico.
- **BCS-029:** write e commit do snapshot devem preservar cooperatividade
  observável do ciclo: retorno de `handle()`/caminho cooperativo, continuidade
  de ciclos posteriores e ausência de reset por watchdog atribuível a essa
  operação, conforme o oráculo de aceite. A escolha final entre execução
  síncrona e assíncrona permanece em `BCS-DEC-004`.

## 7. Falhas e condições de borda

- Primeiro boot ou NVS apagada por operação externa legítima: cada capability
  usa o default vigente; a mera inicialização no default não cria uma mudança a
  persistir.
- Snapshot parcialmente escrito, corrompido, de versão desconhecida ou
  semanticamente inválido: nenhum valor não validado é aplicado; o dispositivo
  continua com os defaults do domínio binário e não apaga settings.
- Registro de outra identidade: o registro é ignorado para a capability atual.
- Comando rejeitado pelo adapter: o estado solicitado não é persistido.
- Aplicação aceita, mas leitura não confirma o valor: o valor solicitado não é
  anunciado nem persistido como estado confirmado.
- Falha de commit: o runtime mantém o estado confirmado em memória e hardware,
  registra a falha e tenta persistir novamente apenas quando ocorrer outra
  transição autorizada; esta especificação não cria fila de retry.
- Reinicialização após commit bem-sucedido: o valor confirmado é restaurado.
- Reinicialização após falha de commit: restaura-se o último snapshot cujo
  commit foi concluído com sucesso.
- Duas capabilities no mesmo estado: cada registro permanece isolado pela
  identidade composta.
- `LEDCapability` em `blink`: o comportamento de persistência por alternância
  permanece subordinado a `BCS-DEC-002`; até essa decisão, a implementação não
  está autorizada a assumir nem "persistir cada alternância" nem "ignorar
  blink" como contrato final.
- Primeiro acesso aos serviços durante o provisionamento: deve reutilizar o
  grafo inicializado no bootstrap, sem reconstrução nem nova leitura do
  snapshot binário.
- Provisionamento BLE fragmentado concluído com `save()` bem-sucedido: deve
  reiniciar de forma controlada e, no boot seguinte, carregar os settings sem
  reentrar em provisioning por `NotFound`.
- Provisionamento com `save()` falho: não agenda restart de sucesso, permanece
  observável como falha e admite nova tentativa.
- Falha de init NVS no storage binário: não executa erase global, não aborta e
  trata o domínio binário como ausente/indisponível.

## 8. Critérios de aceite e validações

Um critério comportamental só é aprovado quando o teste indicado executa ao
menos um caso, termina sem falha nem erro e todas as asserções descritas são
satisfeitas. Compilação, descoberta sem execução, zero casos, erro de upload,
erro de infraestrutura ou resultado desconhecido classificam o critério como
**não verificado**, nunca como aprovado.

### 8.1 Matriz assertável

| Critério | Requisito | Cenário e ação | Resultado observável | Evidência terminal |
|---|---|---|---|---|
| BCS-AC-001 | BCS-001 | Para cada tipo concreto `SwitchCapability`, `SwitchPlugCapability`, `LightCapability`, `LEDCapability` e `ValveCapability`, partir de snapshot válido com estado oposto ao default, executar `setup()` e depois uma transição confirmada autorizada. | Cada um dos cinco tipos restaura o estado correto e persiste exatamente uma vez a transição posterior, sem opt-in específico do tipo. Se qualquer tipo não restaurar ou não persistir, o critério reprova. | Suíte parametrizada ou cinco casos nomeados, todos executados e aprovados, com adapter e storage observáveis. |
| BCS-AC-002 | BCS-002, BCS-019 | Salvar duas capabilities cujas identidades diferem apenas pelo nome, duas que compartilham o nome mas diferem pelo tipo e duas identidades válidas no contrato público com prefixo longo comum e sufixos distintos, inclusive no maior comprimento público aceito; reinicializar o provedor e consultar cada identidade completa. | Cada consulta retorna somente o próprio estado. Nenhuma identidade válida na API pública é truncada, rejeitada por limite interno menor, colide, consome o registro de outra ou altera outra capability. | Teste de round-trip e isolamento que compara nome e tipo integrais antes e depois do reboot simulado, incluindo o comprimento máximo público. |
| BCS-AC-003 | BCS-003, BCS-015 | Partir de `off`, executar `toggle`, confirmar `on`, persistir e decodificar o snapshot; repetir partindo de `on`. | O snapshot contém somente o estado semântico final `on` ou `off`; nunca contém `toggle`, `open`, `closed` ou outro comando transitório/concreto. | Teste executado que observa a entrada do contrato de storage e decodifica o blob persistido nos dois sentidos. |
| BCS-AC-004 | BCS-004, BCS-009, BCS-010, BCS-028 | Configurar uma `ValveCapability` com o `ValveHardwareCommandInterpreter` e um double fiel ao `OutputHardwareAdapter`, que aceita somente `on`, `off` e `toggle`. Restaurar semanticamente `on` e depois `off`. Em seguida, forçar o caminho de fallback pós rejeição/read-back não confirmado e o sync inicial sem registro. | Para `on`, a capability solicita `open`, o interpreter envia `on` ao adapter, o read-back `on` é interpretado como `open` e somente então o estado lógico/publicado se torna `open`. Para `off`, ocorre `closed → off → off → closed`. Em todos os fallbacks, nenhum `off`/`on` cruza para estado lógico/publicação/persistência da valve sem interpreter. Envio direto de `open`/`closed` ao adapter reprova. | Casos executados de restore e fallback com sequência de chamadas e valores observáveis; o double rejeita o vocabulário da valve quando recebido diretamente. |
| BCS-AC-005 | BCS-004, BCS-009, BCS-010 | Para switch, switch plug, light e LED, restaurar `on` e `off` após o `setup()` do adapter, registrando ordem, aceitação, read-back e eventos. | A ordem é `adapter setup → comando interpretado/aplicado → read-back → atualização/publicação`. Estado rejeitado ou não confirmado não é assumido nem publicado. | Casos executados para os quatro tipos, com spy de ordem e asserções de ausência de atualização antes do read-back. |
| BCS-AC-006 | BCS-005 | Gravar e restaurar um snapshot enquanto existe um blob sentinela no namespace/chave de settings; enumerar aberturas e escritas por namespace/chave. | Somente o namespace e a chave exclusivos da funcionalidade são abertos para escrita do domínio binário; o blob sentinela permanece byte a byte idêntico; nenhuma operação de erase global é emitida. | Teste contra NVS real ou emulação fiel que enumera operações por namespace/chave e compara o blob de settings antes e depois. |
| BCS-AC-007 | BCS-006, BCS-012 | Criar snapshots válidos com zero e oito registros; tentar salvar o nono; fornecer separadamente blob truncado, versão desconhecida, quantidade maior que oito, estado fora de `off`/`on`, `used`/`isOn` fora de domínio, campo sem terminador nulo e registro estruturalmente inválido. | Zero e oito são aceitos; o nono é recusado sem alterar o último snapshot válido. Cada blob inválido é rejeitado integralmente e nenhum de seus valores é aplicado. | Casos executados por condição, com inspeção do resultado do provedor, cache e ausência de aplicação nas capabilities. |
| BCS-AC-008 | BCS-006, BCS-012 | Partir de snapshot válido e alterar separadamente um byte do cabeçalho e um byte de cada região de registro ativo, preservando tamanho e versão externos. | Toda alteração coberta é detectada pela verificação de integridade; o snapshot é rejeitado integralmente e nenhum valor corrompido é aplicado. Aceitar conteúdo apenas porque tamanho e versão coincidem reprova. | Teste de corrupção por mutação de bytes, executado sobre formato real, que comprova rejeição e zero chamadas de aplicação. |
| BCS-AC-009 | BCS-007 | Instrumentar as operações NVS, iniciar o serviço uma vez com snapshot presente e consultar todas as oito identidades repetidamente. | Durante o boot ocorre no máximo uma chamada que copia dados do blob para memória. Todas as consultas posteriores retornam do cache e não acrescentam leitura de dados; consulta apenas de tamanho é contabilizada separadamente. | Contadores executados por operação NVS antes e depois do boot e das consultas, com asserção `data_reads <= 1` e delta posterior igual a zero. |
| BCS-AC-010 | BCS-008 | Após o boot, chamar `handle()` repetidamente nos cinco tipos concretos, incluindo LED parado e, se a política de blink estiver decidida, piscando. | O contador de leituras NVS permanece inalterado em todas as chamadas. Gravações decorrentes de transição confirmada autorizada não contam como leitura. | Teste executado com instrumentação NVS e asserção de delta zero para chamadas de leitura. |
| BCS-AC-011 | BCS-011 | Simular primeiro boot, namespace ausente e identidade ausente em snapshot válido; executar `setup()` de cada tipo concreto. | Cada capability mantém o default vigente obtido pelo caminho interpretado aplicável, conclui a inicialização e não cria gravação ou commit apenas por estar no default. | Casos executados com estado lógico, eventos, chamadas de storage e conclusão do setup observáveis. |
| BCS-AC-012 | BCS-009, BCS-010, BCS-012 | Restaurar registro válido e provocar separadamente rejeição do comando e read-back diferente do valor solicitado. | Em ambos os casos o valor solicitado não vira estado lógico, não é publicado e não é persistido como confirmado; a inicialização continua com o estado efetivamente confirmado pelo caminho interpretado ou o default vigente. | Dois casos executados com adapter programável e spies de estado, publicação e save. |
| BCS-AC-013 | BCS-013, BCS-014 | Para cada sentido `off → on` e `on → off`, confirmar a mudança autorizada e depois atribuir novamente o mesmo valor. | Cada mudança efetiva atualiza o cache, grava um snapshot e conclui um commit exatamente uma vez. A repetição produz zero gravações e zero commits adicionais. | Teste executado com contadores separados de atualização de cache, write e commit. |
| BCS-AC-014 | BCS-016 | Produzir transições separadamente por comando remoto, `turnOn`, `turnOff`, `power`, `toggle` e mudança externa observada no adapter. | Cada origem que resulta em mudança confirmada autorizada percorre read-back, atualização/publicação e exatamente um save/commit; origem rejeitada ou sem mudança produz zero save/commit. | Casos executados e nomeados por origem, com adapter, eventos e storage observáveis. |
| BCS-AC-015 | BCS-001, BCS-013, BCS-016, BCS-DEC-002 | Em `LEDCapability` fora de blink, executar comando aceito e depois `handle()`. O ramo de blink somente é avaliado após `BCS-DEC-002`. | Fora de blink, a transição é confirmada, publicada e persistida uma vez. O override de `handle()` não omite sincronização nem duplica commit. O ramo blink permanece não verificável até a decisão. | Teste com LED concreto, adapter e storage observáveis; contadores após a transição fora de blink. O caso de blink fica bloqueado por decisão pendente. |
| BCS-AC-016 | BCS-017, BCS-018, BCS-021, BCS-027 | Injetar separadamente falha de inicialização NVS, open, leitura do blob, write e commit; incluir o cenário em que a recuperação legada tentaria erase global; após cada falha, executar o próximo ciclo cooperativo. | Nenhuma falha aborta, reinicia, bloqueia ou escapa do ciclo esperado. Nenhuma chamada de erase global da NVS é emitida pelo storage binário. Falhas de init/open/read preservam o fluxo/default; falhas de write/commit preservam o estado já confirmado de hardware, lógico e publicado. Ausência e falha possuem resultados/logs distintos, e o ciclo posterior termina. | Um caso executado por operação, usando seam que devolve os mesmos códigos e preserva a semântica de commit da NVS; spy comprova ausência de erase global, retorno ao chamador, ciclo posterior e classe diagnóstica. |
| BCS-AC-017 | BCS-017, BCS-018 | Persistir `off`, confirmar transição de hardware para `on` e provocar falha de write; repetir provocando falha de commit; reinicializar após cada cenário. | Antes do reboot, hardware, estado lógico e publicação permanecem `on` apesar da falha. Após o reboot, restaura-se `off`, que foi o último commit bem-sucedido. Não há rollback imediato nem promoção do snapshot falho a commit válido. | Casos executados com emulação transacional fiel ou NVS real com injeção equivalente, observando estado antes e depois do reboot. |
| BCS-AC-018 | BCS-019 | Persistir estados opostos para duas capabilities e restaurá-las no mesmo boot, variando a ordem de consulta e setup. | Cada capability recebe somente o próprio estado; restaurar ou alterar uma não muda cache, hardware, estado lógico ou publicação da outra. | Teste executado com duas instâncias, dois adapters, dois sinks e inspeção independente dos registros. |
| BCS-AC-019 | BCS-020 | Persistir uma identidade, reinicializar primeiro com nome diferente e depois com tipo diferente. | Nenhuma das identidades alteradas reutiliza o registro anterior; ambas seguem o fluxo de ausência e preservam o default. A identidade original ainda recupera seu próprio registro. | Teste executado cobrindo mudança de nome e de tipo separadamente. |
| BCS-AC-020 | BCS-021 | Executar ausência, snapshot inválido estrutural/semântico, falha de aplicação e falha de init/open/read/write/commit. | O diagnóstico identifica a classe correta sem imprimir valor do blob, settings, credenciais ou conteúdo privado. Falha de storage nunca é registrada como ausência. | Testes com logger capturado e asserções positivas da classe e negativas para sentinelas privadas. |
| BCS-AC-021 | BCS-022 | Comparar as APIs públicas e defaults com a base anterior à funcionalidade; construir oito capabilities antes de `SmartSysApp::setup()` e executar ciclos cooperativos. | Assinaturas e defaults públicos permanecem compatíveis, oito capabilities continuam aceitas, uma nona continua sujeita ao limite vigente e `handle()` retorna cooperativamente sem espera indefinida por storage. Eventual limite explícito de identidade, se autorizado, deve aparecer no contrato público e ser o mesmo do storage. | Inspeção de diff das APIs públicas mais build e teste executado do limite/configuração/ciclo. |
| BCS-AC-022 | BCS-023, BCS-DEC-003 | Compilar o environment de gate autorizado para o runtime Arduino ESP32 e inspecionar as dependências do contrato Core e do código preparatório ESP-IDF afetado. | O build de gate autorizado termina com sucesso; o Core não inclui APIs NVS/Arduino; o provedor Espressif permanece atrás da fronteira de plataforma e nenhuma fonte preparatória ESP-IDF é removida ou tornada dependente do runtime Arduino. Enquanto `BCS-DEC-003` não escolher o environment/oráculo, este critério permanece não verificável. | Build terminal aprovado no environment autorizado e inspeção estática registrada sobre os arquivos alterados. |
| BCS-AC-023 | BCS-024 | Em um processo limpo, instrumentar a construção de `ServiceManager`, o registro de serviços e `loadSnapshot()`; executar o bootstrap que completa `init()` antes de qualquer callback/task concorrente; depois consultar `instance()` pelo mesmo caminho usado na conclusão do provisionamento, inclusive a partir de contexto que simule a task BLE. Repetir a consulta aos dois accessors. | Os endereços retornados por `init()` e `instance()` são iguais; existe exatamente uma construção do grafo, um registro dos serviços de plataforma e uma leitura inicial do snapshot. A ordem observa inicialização única concluída antes do acesso concorrente simulado. Aumentar a pilha, omitir logs ou invocar garantia de statics thread-safe sob `-fno-threadsafe-statics` sem eliminar a segunda instância/race reprova. | Teste executado em processo limpo ou seam fiel de inicialização, com asserção de identidade, contadores exatamente iguais a um e evidência da ordem pré-concorrência; inspeção estática confirma convergência dos accessors e a flag `-fno-threadsafe-statics` no build aplicável. |
| BCS-AC-024 | BCS-025, BCS-026 | Em ESP32-S3 sem settings armazenados, iniciar provisioning, conectar por BLE, habilitar notifications e enviar uma configuração válida fragmentada em escritas de 9, 96, 96, 48 e 3 bytes, encerrando com `END`; observar o resultado de `SettingsManager::save()` e o boot seguinte. | O fluxo não apresenta stack canary, panic, abort ou reboot antecipado. Se `save()` sucede, o commit termina antes do restart controlado; no boot seguinte o cache é carregado, os valores provisionados são preservados e o dispositivo não reentra em provisioning por `NotFound`. | Execução terminal em hardware ESP32-S3 com captura serial completa, prova do sucesso de `save()`/commit e verificação dos settings após o reboot. Execução interrompida, ausência do reboot observado, falta de prova do commit ou somente build classificam o critério como não verificado. |
| BCS-AC-025 | BCS-026 | No caminho de conclusão do provisioning, injetar separadamente sucesso e falha de `SettingsManager::save()` após configuração válida. | No sucesso, restart controlado e status/log de sucesso ocorrem somente depois do retorno bem-sucedido. Na falha, não há restart de sucesso, o status permanece de falha observável e nova tentativa permanece possível. | Teste executado com seam de `save()`, spies de restart/status/log e asserções de ordem. |
| BCS-AC-026 | BCS-027 | Forçar no storage binário as condições que a implementação legada usava para chamar `nvs_flash_erase()` e `ESP_ERROR_CHECK`. | O runtime não aborta, não reinicia e não apaga a partição/namespace global; o domínio binário reporta falha/ausência e os settings sentinela permanecem intactos. | Teste executado com contadores de erase global, códigos de retorno e sobrevivência do processo. |
| BCS-AC-027 | BCS-006, BCS-012 | Fornecer snapshots com checksum correto porém semanticamente inválidos: `used` fora de `{0,1}`, `isOn` fora de `{0,1}`, nome/tipo sem terminador nulo interno e registro ativo com identidade vazia. | Todos são rejeitados antes de qualquer `strcmp`/aplicação; nenhum valor é exposto via `tryGet` nem aplicado em capability. | Casos executados por classe de invalidez semântica, com zero aplicações e diagnóstico de inválido distinto de ausência quando aplicável. |
| BCS-AC-028 | BCS-029, BCS-DEC-004 | Durante uma ou mais transições confirmadas autorizadas, instrumentar início/fim de write e commit, chamar `handle()`/ciclo cooperativo antes, durante a janela observável e depois da operação, e verificar continuidade. | O caminho cooperativo retorna; ciclos posteriores executam; não ocorre reset por watchdog atribuível a write/commit; a operação de persistência não converte o loop em espera indefinida. Se a política final de `BCS-DEC-004` exigir offload assíncrono, o mesmo oráculo de continuidade permanece válido e o commit efetivo continua observável antes do reboot de verificação. | Teste executado com relógio/contadores de ciclo, sondas de write/commit e asserção de continuidade; ausência de medição de cooperatividade reprova mesmo que o estado final esteja correto. |

### 8.2 Fidelidade obrigatória dos doubles

- O double do adapter de saída aceita somente `on`, `off` e `toggle`, como o
  `OutputHardwareAdapter`; ele deve rejeitar `open` e `closed`.
- Testes de valve usam o `ValveHardwareCommandInterpreter` real ou um double
  que comprove explicitamente as conversões `open ↔ on` e `closed ↔ off` em
  restore, sync e fallbacks.
- O seam NVS permite falhar individualmente init, open, read, write e commit;
  expõe contador de erase global/partição; e preserva a propriedade material de
  que somente commit bem-sucedido sobrevive ao reboot.
- Spies de storage mantêm contadores distintos para consulta de tamanho,
  leitura que copia dados, atualização de cache, write, commit e erase global.
- Relógio usado em testes de cooperatividade e, quando autorizado, de blink é
  controlável e avança sem espera real desnecessária.
- O seam de inicialização de serviços preserva a semântica de construção única
  e permite observar ordem antes de acesso concorrente simulado; contadores não
  podem ser reinicializados entre `init()` e `instance()`.
- O seam de provisioning expõe o retorno de `SettingsManager::save()`, a
  decisão de restart e o status/log de sucesso/falha sem executar reboot real
  quando o teste for unitário; o critério de hardware permanece obrigatório em
  BCS-AC-024.

Um double que aceite um vocabulário rejeitado pela integração real, trate write
como commit, oculte erase global, ignore o retorno de `save()` ou não permita
observar uma operação exigida torna o critério correspondente não verificável.

### 8.3 Checklist de autoria

- [x] BCS-001 a BCS-029 estão relacionados a pelo menos um critério.
- [x] Cada critério identifica cenário, ação, resultado observável e evidência
  terminal.
- [x] Os resultados podem ser convertidos em asserções sem decisão funcional ou
  arquitetural adicional, exceto os ramos explicitamente bloqueados por
  `BCS-DEC-002`, `BCS-DEC-003` e `BCS-DEC-004`.
- [x] Os critérios reprovam os desvios apontados em `EKM-CHG-0018`: statics sob
  `-fno-threadsafe-statics`, restart de provisioning sem sucesso de `save()`,
  erase global/`ESP_ERROR_CHECK`, snapshot só com tamanho/versão/checksum,
  identidade com limite interno menor, fallback da valve sem interpreter e
  ausência de oráculo de cooperatividade.
- [x] Validações automatizáveis estão separadas da validação física posterior.
- [x] Decisões pendentes permanecem explícitas, sem decisão pelo Autor.

### 8.4 Gate da implementação

Para promover a implementação a `Implemented`, todos os critérios BCS-AC-001 a
BCS-AC-028 aplicáveis e não bloqueados por decisão pendente devem estar
aprovados ou possuir evidência automatizada equivalente que demonstre exatamente
o mesmo oráculo. São obrigatórios:

- build terminal com `SUCCESS` no environment de gate autorizado por
  `BCS-DEC-003`;
- `pio test -e esp32s3_test` com estado terminal aprovado, quantidade total de
  casos executados maior que zero e os casos desta especificação efetivamente
  executados;
- `git diff --check` sem erros;
- matriz BCS-AC preenchida com resultado terminal e referência à evidência de
  cada critério.

Enquanto `BCS-DEC-002`, `BCS-DEC-003` ou `BCS-DEC-004` bloquearem critério ou
oráculo indispensável, a implementação não pode ser promovida a `Implemented`
por omissão da decisão. Critério falho, não executado ou não verificável mantém
a implementação `In Progress`. Compilar testes com `--without-testing`, obter
zero casos ou falhar antes da execução não satisfaz o gate.

### 8.5 Gate posterior de validação

`Validated` exige validação em hardware com ao menos duas capabilities binárias
de identidades distintas: aplicar estados opostos, confirmar os estados,
desligar completamente o dispositivo, ligar novamente e observar que ambos são
aplicados ao hardware antes da publicação correspondente. Essa validação física
não substitui nem é exigida para afirmar individualmente os oráculos
automatizados; permanece responsabilidade da etapa de validação posterior.

## 9. Conhecimento afetado

- `src/Core/Capabilities/CapabilityHelpers.h`;
- `src/Contracts/Capabilities/ICapability.h` e/ou o contrato de consulta segura
  necessário para identificar o comportamento binário sem RTTI;
- contrato de storage no domínio de capabilities;
- `src/Contracts/Providers/ServiceProvider.*`;
- `src/Core/Providers/ServiceManager.*`;
- `src/Platform/Espressif/Providers/EspressifPlatformServiceRegistrar.*`;
- `src/App/Managers/ProvisioningController.*`;
- `src/Platform/Espressif/Provisioning/BleProvisioningChannel.*`;
- provedor Espressif de estado binário em NVS;
- `src/App/Builders/Builders/CapabilitiesBuilder.*` e/ou
  `CapabilityManager`, conforme a composição aprovada na revisão;
- eventual contrato público de limite de identidade, se autorizado;
- testes, mocks e configuração de teste aplicáveis;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

A lista identifica responsabilidades provavelmente afetadas, não autoriza
reorganização estrutural nem obriga a alterar todos os arquivos citados.

## 10. Relações

- `IOTSSC-PUBLIC-API`;
- `IOTSSC-RUNTIME`;
- `EKM-CHG-0009`;
- `EKM-CHG-0013`;
- `EKM-CHG-0018`.

## 11. Decisões pendentes do Arquiteto

### BCS-DEC-001 — Factory reset

A intenção recebida não determina se o factory reset deve apagar também o
snapshot de estados binários.

**Recomendação do Autor:** apagar o namespace desta funcionalidade junto com os
demais dados persistentes de dispositivo, para que o primeiro boot após factory
reset use os defaults. A recomendação não é decisão confirmada e não autoriza
alterar o fluxo de factory reset.

**Impacto na revisão futura:** não bloqueante para o recorte de persistência em
reboot comum, queda de energia, atualização de firmware e provisioning. Factory
reset permanece fora do escopo autorizado até decisão específica.

### BCS-DEC-002 — Política de persistência de `blink`

A intenção confirmada de "persistir mudança binária" não decide se cada
alternância de `LEDCapability` em `blink` deve gerar write/commit, se
transições transitórias de blink devem ser excluídas, se o estado deve ser
consolidado ao sair do blink, ou se debounce/batching com perda aceitável é
permitido.

**Recomendação do Autor:** excluir do protocolo de commit as alternâncias
estritamente transitórias de blink e persistir apenas o estado confirmado ao
entrar/sair do modo ou por comando explícito, para evitar desgaste acelerado da
NVS. A recomendação não autoriza implementação.

**Impacto na revisão futura:** bloqueante para o ramo blink de BCS-013,
BCS-016, BCS-AC-015 e para qualquer declaração de implementabilidade integral
que inclua automação blink. Não bloqueia o restante do protocolo fora de blink.

### BCS-DEC-003 — Gate de build `esp32_dev`

O gate histórico exige `pio run -e esp32_dev` com `SUCCESS`. Há registro de
falha preexistente e alheia ao domínio binário nesse environment. O Arquiteto
ainda não autorizou corrigir o baseline, substituir o environment obrigatório
nem definir oráculo equivalente.

**Recomendação do Autor:** ou autorizar correção mínima e separada do baseline
de `esp32_dev`, ou substituir o gate desta especificação por environment
suportado já utilizado na validação do recorte, sem ampliar silenciosamente o
escopo funcional binário. A recomendação não autoriza a correção.

**Impacto na revisão futura:** bloqueante para BCS-AC-022, para o gate 8.4 e
para promoção a `Implemented`, enquanto o oráculo de build permanecer
insatisfazível ou indefinido.

### BCS-DEC-004 — Contexto síncrono ou assíncrono da persistência

Ainda não está decidido se write/commit NVS pode permanecer no caminho
síncrono da transição/capability ou se deve ser deslocado para trabalho
cooperativo/assíncrono com semântica explícita de falha e de "último commit
visível após reboot".

**Recomendação do Autor:** preferir caminho que preserve BCS-029 sem jitter
capaz de comprometer conectividade ou watchdog sob a frequência de transições
autorizada; se a medição do oráculo de cooperatividade não puder ser satisfeita
de forma síncrona com a política de origens escolhida, adotar offload
cooperativo com commit observável. A recomendação não autoriza a escolha.

**Impacto na revisão futura:** bloqueante para fechar a forma de execução de
write/commit e para quantificar limites adicionais de latência além do oráculo
qualitativo de continuidade. Não bloqueia a análise dos demais requisitos se a
revisão declarar explicitamente a dependência residual.

## 12. Estado da autoria

A versão 0.4 corrige a versão 0.3 para incorporar `EKM-CHG-0018`. Ela:

- torna explícita a inicialização única de `ServiceManager` antes de acessos
  concorrentes sob `-fno-threadsafe-statics`;
- condiciona sucesso e restart do provisioning ao sucesso de
  `SettingsManager::save()`;
- proíbe erase global da NVS e abort por checagem fatal no storage binário;
- exige validação estrutural e semântica completa do snapshot;
- reconcilia o limite de identidade com a API pública;
- exige interpreter em todos os fallbacks da valve;
- cria oráculo de cooperatividade para write/commit;
- completa critérios para falhas NVS, isolamento de settings, identidade, valve
  e provisioning após reboot;
- remove metadados Git sem necessidade normativa;
- registra `BCS-DEC-001` a `BCS-DEC-004` sem decidir pelo Arquiteto.

Ao fim desta autoria, a especificação está `Proposed`, a implementação desta
versão está `Not Started`, a entrega está `Not Ready` e a revisão de
implementabilidade está `Pending Review`.

O Autor da Especificação não executou análise de implementabilidade
independente, implementação, build nem testes funcionais. O estado
`Implementable` da versão 0.3 não é reutilizado.

## 13. Revisão de implementabilidade da versão 0.2 (histórico contestado)

**Resultado histórico:** Implementável [`Implementable`]

**Status atual deste resultado:** contestado pelos achados posteriores de
implementação parcial, validação e pela avaliação consultiva da linha 0.3;
preservado apenas como evidência histórica. Não autoriza implementação da
versão 0.4.

A análise histórica da versão 0.2 concluiu implementabilidade de BCS-001 a
BCS-023 com base nos padrões então vigentes e classificou `BCS-DEC-001` como
não bloqueante. Artefatos experimentais e correções parciais posteriores
revelaram desvios materiais de interpreter da valve, protocolo do LED,
integridade/identidade do snapshot e ausência de evidência terminal em
hardware. Esses desvios invalidam o uso deste resultado como fundamento atual.

## 14. Implementação da versão 0.2 (histórico contestado)

**Estado histórico da transação:** Em andamento [`In Progress`], registrada em
`EKM-CHG-0015`.

A implementação parcial da versão 0.2 permaneceu na branch como material de
partida e não como implementação aprovada. A matriz histórica registrou no
máximo cobertura incompleta dos critérios então existentes, com a maior parte
compilada e não executada ou não verificada, e com o gate de testes em hardware
não satisfeito.

Para a versão 0.4, esse código é precedente tático e fonte de riscos
conhecidos — inclusive erase global da NVS, validação semântica incompleta,
fallback da valve, restart de provisioning incondicional e ausência de oráculo
de cooperatividade — não evidência de conformidade.

## 15. Revisão de implementabilidade da versão 0.3 (histórico contestado)

**Resultado histórico:** Implementável [`Implementable`], registrado em
`EKM-CHG-0017`.

**Status atual deste resultado:** contestado por `EKM-CHG-0018`. Não é
reutilizado por esta autoria e não autoriza implementação da versão 0.4.

Inconsistências materiais registradas na contestação:

1. a revisão afirmou thread-safety de estáticas locais, embora o build use
   `-fno-threadsafe-statics`;
2. apresentou-se como revisão independente e, ao mesmo tempo, preservou
   BCS-001 a BCS-023 por reuso da linha 0.2;
3. manteve gate `esp32_dev` sem caminho de aprovação;
4. concluiu que a correção do singleton bastava para o provisioning, apesar do
   retorno de `save()` ser ignorado;
5. copiou metadado Git sem necessidade normativa.

A versão 0.4 reinstaura `Pending Review` e exige nova atuação independente do
Engenheiro Analista sobre este contrato corrigido.
