# Especificação — Persistência do estado de comandos binários

**ID:** IOTSSC-BINARY-COMMAND-STATE

**Classe da fonte:** Normativa

**Versão:** 0.6

**Estado normativo:** Proposta [`Proposed`]

**Estado da implementação:** Em andamento [`In Progress`]

**Estado da entrega:** Não pronta [`Not Ready`]

**Revisão de implementabilidade:** Implementável [`Implementable`]

**Relação normativa:** Corrige [`Corrects`]
`IOTSSC-BINARY-COMMAND-STATE@0.5`

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

- registrar de forma assíncrona o último estado binário estável confirmado,
  excluindo alternâncias transitórias produzidas pelo temporizador de `blink`;
- manter a leitura de boot leve;
- aplicar, durante a inicialização, o último estado validamente registrado para
  cada capability abrangida;
- preservar o grafo único de serviços, o provisionamento BLE e os settings
  existentes.

A versão 0.6 incorpora a avaliação consultiva registrada em `EKM-CHG-0018` e
as decisões arquiteturais confirmadas pelo Arquiteto para `BCS-DEC-002` a
`BCS-DEC-007`. Ela corrige o contrato 0.5 sem reutilizar o estado
`Implementable` de versões anteriores.
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
- antes da versão 0.6, a API pública de configuração expunha `capability_name`
  e o construtor comum expunha `type` como ponteiro/string sem limite
  documental único; o builder possui buffer local de geração automática, mas
  nomes fornecidos externamente não compartilham automaticamente o mesmo teto
  do storage experimental;
- `SmartSysApp::add*Capability()` devolve ponteiro para a capability já
  registrada; na implementação vigente, `ICapability::capability_name` e
  `ICapability::type` ainda são campos públicos mutáveis, enquanto `rename()` e
  `applyRenamedName()` também podem mudar a identidade e retornam `void`; o
  contrato confirmado em `BCS-DEC-006` elimina essa mutabilidade sem alterar o
  retorno dos métodos nem os ponteiros devolvidos por `add*Capability()`;
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
- contrato público de identidade limitado a 63 bytes para `capability_name` e
  31 bytes para `type`, excluído o terminador nulo, com rejeição observável
  antes do registro;
- finalização da identidade antes do registro e imutabilidade pública de
  `capability_name` e `type` durante toda a vida da capability registrada;
- representação persistida dos dois estados lógicos suportados;
- leitura, validação estrutural/semântica e cache do snapshot durante o boot;
- aplicação do estado restaurado ao hardware durante `setup()`;
- solicitação assíncrona de gravação e commit em NVS após mudança lógica
  estável confirmada, com exclusão das alternâncias transitórias de `blink`;
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
- build do runtime Arduino sobre ESP32 no environment canônico `esp32_dev`.

## 4. Fora de escopo

- persistir estados de capabilities que não derivem de
  `BinaryCommandCapability`;
- persistir comandos transitórios como o texto `toggle`;
- alterar nomes públicos de capabilities, comandos, tipos ou estados além dos
  limites e da imutabilidade explícita da identidade autorizados em
  `BCS-DEC-005` e `BCS-DEC-006`;
- alterar defaults públicos das capabilities;
- alterar o limite de oito capabilities;
- sincronizar o estado persistido com API remota, MQTT ou settings do
  dispositivo;
- criar histórico de estados, contador de acionamentos ou telemetria;
- persistir cada alternância produzida exclusivamente pelo temporizador de
  `blink`, ou alterar a política estável de `blink` confirmada nesta versão;
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

O contrato público da identidade passa a aceitar:

- `capability_name` definitivo não vazio com no máximo 63 bytes de sua
  representação UTF-8, excluído o terminador nulo;
- `type` não vazio com no máximo 31 bytes de sua representação UTF-8, excluído
  o terminador nulo.

O limite é medido em bytes antes do primeiro `\0`; nenhum valor pode ser
cortado no limite, inclusive no meio de uma sequência multibyte. Nome ou tipo
acima do respectivo limite deve ser rejeitado de forma observável antes de a
capability ou sua identidade ocupar um dos oito slots, antes de consultar ou
alterar o cache binário e antes de qualquer solicitação de persistência. A
rejeição não pode deixar capability, adapter ou registro parcial e deve usar o
mecanismo de falha já observável do builder/registro público aplicável.

Omissão de `capability_name` continua autorizando a geração automática vigente;
o limite é verificado sobre o nome definitivo gerado antes do registro. Esta
mudança não altera assinaturas nem nomes de tipos/configs. Configurações
existentes dentro dos limites permanecem compatíveis; consumidor que forneça
nome ou tipo excedente deve adequar a identidade antes de adotar esta versão.
Não existe truncamento, alias automático nem migração silenciosa de registro
persistido excedente.

O builder deve resolver o nome definitivo — fornecido ou gerado — e o tipo,
validá-los e fixá-los antes do registro. A capability registrada conserva
acesso público de leitura a `capability_name` e `type`, mas não oferece
atribuição pública nem qualquer outro caminho suportado que os modifique. Esta
quebra da escrita direta é a mudança de compatibilidade aprovada em
`BCS-DEC-006`.

O storage deve reservar capacidade para os valores públicos completos e seus
terminadores: ao menos 64 bytes para `capability_name` e 32 bytes para `type`
quando usar campos fixos. Representação variável é permitida somente se
preservar os mesmos máximos e todos os demais oráculos do snapshot.

A identidade deve ser preservada integralmente para todo nome e tipo aceitos
por esse contrato. São reprovações:

- truncamento silencioso;
- colisão por prefixo;
- gravação parcial;
- rejeição causada apenas por limite interno de storage menor do que o limite
  efetivamente aceito pela API/configuração pública.

Não é aceitável um storage "interno" mais restritivo do que esses limites nem
um caminho alternativo de construção ou renomeação que aceite identidade maior
para uma capability abrangida.

O estado semântico deve ser convertido para `_offValue` ou `_onValue` pela
própria `BinaryCommandCapability`. Assim, o mesmo storage atende vocabulários
como `off`/`on` e `closed`/`open` sem persistir comandos transitórios.

O snapshot deve ser obtido por no máximo uma chamada que copie o blob da NVS
para memória durante a inicialização do serviço. Uma consulta de metadado para
obter o tamanho do blob não conta como leitura de dados; qualquer chamada
adicional que copie conteúdo conta. Consultas posteriores no mesmo boot devem
ocorrer somente no cache em memória. O ciclo cooperativo de `handle()` não pode
fazer leituras NVS.

Uma solicitação de persistência deve atualizar, sob sincronização interna do
provedor, o estado desejado em memória para a identidade afetada. A chamada
originada pela capability deve retornar sem executar `nvs_set_blob()`,
`nvs_commit()` ou esperar a conclusão dessas operações.

Um único escritor assíncrono deve serializar write e commit do snapshot. O
trabalho pendente deve ser limitado pelo máximo de oito identidades: cada
identidade pode manter somente o estado estável confirmado mais recente ainda
não processado. Se novas transições da mesma identidade ocorrerem antes do
início do próximo write, elas devem ser consolidadas no valor mais recente,
sem crescimento de fila, alocação por transição ou obrigação de persistir
estados intermediários já substituídos.

O precedente técnico mais próximo é o worker FreeRTOS com sincronização e
estado observável já usado pela integração Espressif de settings. A
implementação deve permanecer no provedor Espressif, usando primitives da
plataforma e sem introduzir dependência FreeRTOS no contrato Core. Isso não
autoriza criar scheduler, camada transversal ou fila genérica nova.

O worker deve ser criado uma única vez no bootstrap, somente depois de
`ServiceManager::init()` retornar com o grafo completamente construído e da
leitura inicial do snapshot, mas antes de capabilities poderem solicitar
persistência. Falha de criação deve tornar o escritor indisponível de forma
observável; não autoriza fallback síncrono. Sua pilha deve ser dimensionada e
medida no target, e sua prioridade não pode impedir a continuidade das tasks de
conectividade, BLE ou do loop cooperativo.

O retorno da operação chamada pela capability informa somente se a identidade,
o valor e a solicitação assíncrona foram aceitos; não significa que write ou
commit terminaram. Falha posterior deve ser diagnosticada pelo próprio
provedor e refletida no estado terminal observável, sem callback para a
capability.

Uma transição ocorrida durante write/commit deve permanecer pendente quando
divergir do snapshot efetivamente confirmado pelo commit em curso. Somente um
commit NVS bem-sucedido pode atualizar a visão de último snapshot persistido.
Falha de write/commit deve ser observável, não pode ser promovida a sucesso e
não deve criar retry contínuo; nova tentativa ocorre quando outra transição
estável confirmada atualizar a identidade. O provedor deve oferecer seam
interno observável para distinguir trabalho pendente, operação em curso,
último commit bem-sucedido e falha, permitindo aguardar quiescência nos testes
e antes de um reboot controlado que pretenda verificar persistência.

Gravação e leitura não podem tocar o namespace ou a chave de settings. O
escritor de estado binário não substitui nem altera a semântica síncrona exigida
para `SettingsManager::save()` no encerramento do provisioning.

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

Depois do registro, `capability_name` e `type` são imutáveis. Os métodos
`rename()` e `applyRenamedName()` permanecem na API pública, preservam seus
retornos `void` e devem ser marcados como obsoletos, mas não podem alterar a
identidade finalizada de uma capability registrada. Chamá-los nesse estado
preserva silenciosamente o nome e o tipo vigentes. Nenhum novo uso desses
métodos é autorizado. Os métodos `SmartSysApp::add*Capability()` continuam
devolvendo o ponteiro para a capability já registrada, agora com identidade
somente para leitura.

O fluxo de inicialização proposto é:

```text
bootstrap da plataforma
→ inicialização única e completa de ServiceManager antes de tasks/callbacks concorrentes
→ inicialização do storage e leitura única do snapshot NVS
→ validação estrutural e semântica do snapshot
→ retorno de ServiceManager::init() e ativação única do escritor assíncrono
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

Toda transição lógica estável confirmada entre os dois valores binários, cuja
origem esteja autorizada pela política vigente, deve atualizar o estado
desejado em memória e sinalizar o escritor assíncrono. Isso inclui, no
protocolo comum:

- comandos remotos;
- chamadas públicas como `turnOn`, `turnOff`, `power` e `toggle`;
- sincronização com mudança observada no adapter;
- comportamento automático de classe derivada que produza estado estável.

Alternâncias produzidas exclusivamente pelo temporizador de
`LEDCapability::blink` são transitórias: continuam aplicadas, confirmadas e
publicadas pelo protocolo comum, mas não atualizam o estado persistente nem
sinalizam o escritor. Iniciar `blink` não substitui o último estado estável
persistido. Quando `blink` terminar ou for substituído por comando explícito,
o primeiro estado estável confirmado deve solicitar persistência uma única vez
se diferir do último estado estável solicitado.

Repetir o mesmo valor não constitui mudança e não pode gerar nova gravação. O
texto `toggle` nunca deve ser persistido; deve ser registrado apenas o estado
resultante confirmado.

O caminho comum de `BinaryCommandCapability` deve centralizar a detecção da
transição e a solicitação de persistência. Um override de classe derivada,
inclusive `LEDCapability::handle()`, não pode contornar aplicação, read-back,
atualização do estado lógico, publicação e o protocolo de persistência
aplicável. Uma falha de gravação deve ser registrada em log, mas não pode
reverter um comando já aplicado nem impedir o processamento cooperativo.

A execução de write/commit deve ocorrer fora de callbacks BLE, do caminho
síncrono de comando e de `handle()` das capabilities. O escritor deve ser
único, ter memória limitada pelo contrato de oito identidades e não pode chamar
de volta uma capability a partir do contexto de persistência.

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
  definitivo e o `type`. O contrato público aceita no máximo 63 bytes para o
  nome e 31 bytes para o tipo, excluídos os terminadores; o storage deve
  preservar integralmente esses máximos, sem truncamento silencioso, colisão
  por prefixo, gravação parcial ou limite interno menor. Identidade definitiva
  vazia ou excedente deve ser rejeitada de forma observável antes do registro e
  sem consumir slot, alterar cache ou solicitar persistência; nome omitido
  continua sujeito à geração automática vigente e é validado depois de gerado.
  Nome e tipo devem ser finalizados antes do registro e permanecer imutáveis
  durante toda a vida da capability registrada, com leitura pública e sem
  atribuição pública.
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
- **BCS-013:** toda transição estável confirmada autorizada de `off` para `on`
  ou de `on` para `off` deve atualizar o estado desejado e sinalizar o escritor
  assíncrono. Alternâncias produzidas exclusivamente pelo temporizador de
  `blink` não são persistíveis; estados pendentes da mesma identidade podem ser
  consolidados no valor mais recente antes do próximo write.
- **BCS-014:** uma tentativa de atribuir novamente o estado lógico corrente não
  pode gerar gravação nem commit.
- **BCS-015:** `toggle` deve persistir somente o valor final confirmado, nunca
  o comando transitório.
- **BCS-016:** mudanças originadas por comandos, API pública, leitura do
  adapter ou automação de classe derivada devem seguir o mesmo protocolo de
  aplicação/read-back/publicação e o protocolo de persistência aplicável, mesmo
  quando a classe derivada sobrescrever `handle()` ou outro ponto do ciclo;
  somente as alternâncias transitórias de `blink` ficam excluídas da
  persistência.
- **BCS-017:** falha ao inicializar, abrir, ler, gravar ou executar commit na
  NVS do storage binário não pode bloquear o loop, abortar ou reiniciar o
  dispositivo nem desfazer o estado de hardware já confirmado; write/commit
  não podem executar em callback BLE, caminho síncrono de comando ou
  `handle()` de capability.
- **BCS-018:** após falha de persistência, o estado lógico e sua publicação
  devem continuar refletindo o hardware; após reboot, o último estado
  persistido continua sendo o snapshot do último commit concluído com sucesso.
- **BCS-019:** a restauração de uma capability não pode alterar outra
  capability, mesmo quando ambas possuem o mesmo estado semântico.
- **BCS-020:** mudança de nome ou tipo configurada entre boots não pode
  reutilizar silenciosamente o registro da identidade anterior; a capability
  já registrada não admite mudança de identidade durante o mesmo boot.
- **BCS-021:** logs devem permitir distinguir ausência de registro, registro
  inválido, falha de aplicação e cada falha de storage observada, sem converter
  falha de storage em ausência e sem expor settings, credenciais ou conteúdo
  privado.
- **BCS-022:** APIs públicas, defaults, ordem de configuração antes de
  `SmartSysApp::setup()`, processamento cooperativo e limite de oito
  capabilities devem ser preservados, ressalvados os limites, a rejeição
  pré-registro e a remoção da escrita pública de identidade autorizados em
  BCS-002, `BCS-DEC-005` e `BCS-DEC-006`. `rename()` e
  `applyRenamedName()` permanecem públicos, obsoletos e com retorno `void`, sem
  alterar a identidade registrada; `SmartSysApp::add*Capability()` preserva
  suas assinaturas e continua retornando a mesma capability registrada.
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
- **BCS-029:** write e commit do snapshot devem ser executados por um único
  escritor assíncrono, com trabalho pendente limitado a uma entrada por cada
  uma das oito identidades, consolidação do estado mais recente antes do write,
  sincronização entre snapshot desejado e confirmado e estado terminal
  observável. O caminho solicitante deve retornar sem executar ou aguardar NVS;
  `handle()` e ciclos posteriores devem continuar, sem reset por watchdog
  atribuível à operação. O worker deve ser criado uma única vez, após a
  conclusão de `ServiceManager::init()` e antes do uso; falha de criação não
  pode provocar fallback síncrono, e medição em target sob carga deve preservar
  ao menos 25% de margem da pilha configurada.

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
  transição estável autorizada; esta especificação não cria retry contínuo.
- Reinicialização após commit bem-sucedido: o valor confirmado é restaurado.
- Reinicialização após falha de commit: restaura-se o último snapshot cujo
  commit foi concluído com sucesso.
- Duas capabilities no mesmo estado: cada registro permanece isolado pela
  identidade composta.
- `LEDCapability` em `blink`: alternâncias produzidas exclusivamente pelo
  temporizador são aplicadas e publicadas, mas não persistidas. O último estado
  estável permanece válido durante `blink`; ao encerrar o modo, o primeiro
  estado estável confirmado é solicitado uma vez se tiver mudado.
- Rajada de transições estáveis antes do próximo write: cada identidade ocupa
  no máximo uma entrada pendente e conserva somente seu valor mais recente;
  após quiescência e commit bem-sucedido, esse valor é o restaurado no reboot.
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
- Tentativa de escrita direta em `capability_name` ou `type` de uma capability
  registrada: a API não oferece operação de atribuição; chamadas legadas a
  `rename()` ou `applyRenamedName()` preservam a identidade sem alteração.

## 8. Critérios de aceite e validações

Um critério comportamental só é aprovado quando o teste indicado executa ao
menos um caso, termina sem falha nem erro e todas as asserções descritas são
satisfeitas. Compilação, descoberta sem execução, zero casos, erro de upload,
erro de infraestrutura ou resultado desconhecido classificam o critério como
**não verificado**, nunca como aprovado.

### 8.1 Matriz assertável

| Critério | Requisito | Cenário e ação | Resultado observável | Evidência terminal |
|---|---|---|---|---|
| BCS-AC-001 | BCS-001 | Para cada tipo concreto `SwitchCapability`, `SwitchPlugCapability`, `LightCapability`, `LEDCapability` e `ValveCapability`, partir de snapshot válido com estado oposto ao default, executar `setup()`, produzir uma transição estável autorizada e aguardar quiescência do escritor. | Cada um dos cinco tipos restaura o estado correto e conclui exatamente um commit da transição posterior, sem opt-in específico do tipo. Se qualquer tipo não restaurar ou não persistir, o critério reprova. | Suíte parametrizada ou cinco casos nomeados, todos executados e aprovados, com adapter e estados do escritor observáveis. |
| BCS-AC-002 | BCS-002, BCS-019 | Salvar duas capabilities cujas identidades diferem apenas pelo nome, duas que compartilham o nome mas diferem pelo tipo e pares com prefixo longo comum e sufixos distintos, incluindo `capability_name` de 63 bytes e `type` de 31 bytes. Tentar separadamente nome de 64 bytes e tipo de 32 bytes; omitir o nome em outro caso para exercer a geração automática; reinicializar o provedor e consultar cada identidade válida completa. | Cada identidade válida, inclusive a gerada automaticamente, retorna somente o próprio estado, sem truncamento, colisão, gravação parcial ou limite interno menor. Cada identidade excedente é rejeitada antes do registro, não consome slot, não cria capability/adapter parcial, não altera cache e não sinaliza o escritor. Nome e tipo observados após o registro permanecem os valores validados e finalizados. | Teste de round-trip, isolamento e limites que compara nome e tipo integrais antes e depois do reboot simulado, observa geração automática, falha pública pré-registro e contadores de slots, cache e escritor. |
| BCS-AC-003 | BCS-003, BCS-015 | Partir de `off`, executar `toggle`, confirmar `on`, persistir e decodificar o snapshot; repetir partindo de `on`. | O snapshot contém somente o estado semântico final `on` ou `off`; nunca contém `toggle`, `open`, `closed` ou outro comando transitório/concreto. | Teste executado que observa a entrada do contrato de storage e decodifica o blob persistido nos dois sentidos. |
| BCS-AC-004 | BCS-004, BCS-009, BCS-010, BCS-028 | Configurar uma `ValveCapability` com o `ValveHardwareCommandInterpreter` e um double fiel ao `OutputHardwareAdapter`, que aceita somente `on`, `off` e `toggle`. Restaurar semanticamente `on` e depois `off`. Em seguida, forçar o caminho de fallback pós rejeição/read-back não confirmado e o sync inicial sem registro. | Para `on`, a capability solicita `open`, o interpreter envia `on` ao adapter, o read-back `on` é interpretado como `open` e somente então o estado lógico/publicado se torna `open`. Para `off`, ocorre `closed → off → off → closed`. Em todos os fallbacks, nenhum `off`/`on` cruza para estado lógico/publicação/persistência da valve sem interpreter. Envio direto de `open`/`closed` ao adapter reprova. | Casos executados de restore e fallback com sequência de chamadas e valores observáveis; o double rejeita o vocabulário da valve quando recebido diretamente. |
| BCS-AC-005 | BCS-004, BCS-009, BCS-010 | Para switch, switch plug, light e LED, restaurar `on` e `off` após o `setup()` do adapter, registrando ordem, aceitação, read-back e eventos. | A ordem é `adapter setup → comando interpretado/aplicado → read-back → atualização/publicação`. Estado rejeitado ou não confirmado não é assumido nem publicado. | Casos executados para os quatro tipos, com spy de ordem e asserções de ausência de atualização antes do read-back. |
| BCS-AC-006 | BCS-005 | Gravar e restaurar um snapshot enquanto existe um blob sentinela no namespace/chave de settings; enumerar aberturas e escritas por namespace/chave. | Somente o namespace e a chave exclusivos da funcionalidade são abertos para escrita do domínio binário; o blob sentinela permanece byte a byte idêntico; nenhuma operação de erase global é emitida. | Teste contra NVS real ou emulação fiel que enumera operações por namespace/chave e compara o blob de settings antes e depois. |
| BCS-AC-007 | BCS-006, BCS-012 | Criar snapshots válidos com zero e oito registros; tentar salvar o nono; fornecer separadamente blob truncado, versão desconhecida, quantidade maior que oito, estado fora de `off`/`on`, `used`/`isOn` fora de domínio, campo sem terminador nulo e registro estruturalmente inválido. | Zero e oito são aceitos; o nono é recusado sem alterar o último snapshot válido. Cada blob inválido é rejeitado integralmente e nenhum de seus valores é aplicado. | Casos executados por condição, com inspeção do resultado do provedor, cache e ausência de aplicação nas capabilities. |
| BCS-AC-008 | BCS-006, BCS-012 | Partir de snapshot válido e alterar separadamente um byte do cabeçalho e um byte de cada região de registro ativo, preservando tamanho e versão externos. | Toda alteração coberta é detectada pela verificação de integridade; o snapshot é rejeitado integralmente e nenhum valor corrompido é aplicado. Aceitar conteúdo apenas porque tamanho e versão coincidem reprova. | Teste de corrupção por mutação de bytes, executado sobre formato real, que comprova rejeição e zero chamadas de aplicação. |
| BCS-AC-009 | BCS-007 | Instrumentar as operações NVS, iniciar o serviço uma vez com snapshot presente e consultar todas as oito identidades repetidamente. | Durante o boot ocorre no máximo uma chamada que copia dados do blob para memória. Todas as consultas posteriores retornam do cache e não acrescentam leitura de dados; consulta apenas de tamanho é contabilizada separadamente. | Contadores executados por operação NVS antes e depois do boot e das consultas, com asserção `data_reads <= 1` e delta posterior igual a zero. |
| BCS-AC-010 | BCS-008 | Após o boot, chamar `handle()` repetidamente nos cinco tipos concretos, incluindo LED parado e piscando. | O contador de leituras NVS permanece inalterado em todas as chamadas. Gravações assíncronas decorrentes de transição estável autorizada não contam como leitura. | Teste executado com instrumentação NVS e asserção de delta zero para chamadas de leitura. |
| BCS-AC-011 | BCS-011 | Simular primeiro boot, namespace ausente e identidade ausente em snapshot válido; executar `setup()` de cada tipo concreto. | Cada capability mantém o default vigente obtido pelo caminho interpretado aplicável, conclui a inicialização e não cria gravação ou commit apenas por estar no default. | Casos executados com estado lógico, eventos, chamadas de storage e conclusão do setup observáveis. |
| BCS-AC-012 | BCS-009, BCS-010, BCS-012 | Restaurar registro válido e provocar separadamente rejeição do comando e read-back diferente do valor solicitado. | Em ambos os casos o valor solicitado não vira estado lógico, não é publicado e não é persistido como confirmado; a inicialização continua com o estado efetivamente confirmado pelo caminho interpretado ou o default vigente. | Dois casos executados com adapter programável e spies de estado, publicação e save. |
| BCS-AC-013 | BCS-013, BCS-014 | Para cada sentido `off → on` e `on → off`, confirmar uma mudança estável isolada, aguardar quiescência do escritor e depois atribuir novamente o mesmo valor. Repetir com uma rajada `off → on → off → on` antes de liberar o write. | Cada mudança isolada atualiza o estado desejado e termina em um commit. A repetição produz zero sinalizações, writes e commits adicionais. Na rajada, existe no máximo uma entrada pendente para a identidade, estados substituídos são consolidados e, após quiescência, `on` é o último estado confirmado pelo commit e restaurado no reboot simulado. | Teste executado com escritor controlável e contadores separados de atualização desejada, sinalização, write e commit, incluindo inspeção do snapshot após reboot simulado. |
| BCS-AC-014 | BCS-016 | Produzir transições estáveis separadamente por comando remoto, `turnOn`, `turnOff`, `power`, `toggle` e mudança externa observada no adapter; aguardar quiescência após cada caso. | Cada origem que resulta em mudança estável confirmada percorre read-back e atualização/publicação, sinaliza o escritor e termina em commit; origem rejeitada ou sem mudança produz zero sinalizações/write/commit. | Casos executados e nomeados por origem, com adapter, eventos e estados do escritor observáveis. |
| BCS-AC-015 | BCS-001, BCS-013, BCS-016, BCS-DEC-002 | Em `LEDCapability`, persistir um estado estável, iniciar `blink`, executar múltiplas alternâncias pelo temporizador e depois encerrar o modo em estado oposto. | As alternâncias são aplicadas, confirmadas e publicadas sem sinalização, write ou commit. Durante `blink`, o estado persistido anterior permanece inalterado. Ao encerrar, o novo estado estável sinaliza o escritor uma vez e, após quiescência, é restaurado no reboot simulado. | Teste com LED concreto, relógio controlável, adapter, publicação e escritor observáveis; contadores separam alternâncias transitórias da consolidação estável final. |
| BCS-AC-016 | BCS-017, BCS-018, BCS-021, BCS-027 | Injetar separadamente falha de inicialização NVS, open, leitura do blob, write e commit; incluir o cenário em que a recuperação legada tentaria erase global; após cada falha, executar o próximo ciclo cooperativo. | Nenhuma falha aborta, reinicia, bloqueia ou escapa do ciclo esperado. Nenhuma chamada de erase global da NVS é emitida pelo storage binário. Falhas de init/open/read preservam o fluxo/default; falhas de write/commit preservam o estado já confirmado de hardware, lógico e publicado. Ausência e falha possuem resultados/logs distintos, e o ciclo posterior termina. | Um caso executado por operação, usando seam que devolve os mesmos códigos e preserva a semântica de commit da NVS; spy comprova ausência de erase global, retorno ao chamador, ciclo posterior e classe diagnóstica. |
| BCS-AC-017 | BCS-017, BCS-018 | Persistir `off`, confirmar transição de hardware para `on` e provocar falha de write; repetir provocando falha de commit; reinicializar após cada cenário. | Antes do reboot, hardware, estado lógico e publicação permanecem `on` apesar da falha. Após o reboot, restaura-se `off`, que foi o último commit bem-sucedido. Não há rollback imediato nem promoção do snapshot falho a commit válido. | Casos executados com emulação transacional fiel ou NVS real com injeção equivalente, observando estado antes e depois do reboot. |
| BCS-AC-018 | BCS-019 | Persistir estados opostos para duas capabilities e restaurá-las no mesmo boot, variando a ordem de consulta e setup. | Cada capability recebe somente o próprio estado; restaurar ou alterar uma não muda cache, hardware, estado lógico ou publicação da outra. | Teste executado com duas instâncias, dois adapters, dois sinks e inspeção independente dos registros. |
| BCS-AC-019 | BCS-020 | Persistir uma identidade, reinicializar primeiro com nome diferente e depois com tipo diferente. | Nenhuma das identidades alteradas reutiliza o registro anterior; ambas seguem o fluxo de ausência e preservam o default. A identidade original ainda recupera seu próprio registro. | Teste executado cobrindo mudança de nome e de tipo separadamente. |
| BCS-AC-020 | BCS-021 | Executar ausência, snapshot inválido estrutural/semântico, falha de aplicação e falha de init/open/read/write/commit. | O diagnóstico identifica a classe correta sem imprimir valor do blob, settings, credenciais ou conteúdo privado. Falha de storage nunca é registrada como ausência. | Testes com logger capturado e asserções positivas da classe e negativas para sentinelas privadas. |
| BCS-AC-021 | BCS-022, BCS-DEC-005, BCS-DEC-006 | Comparar as APIs públicas e defaults com a base anterior à funcionalidade; construir oito capabilities com identidades nos limites de 63/31 bytes antes de `SmartSysApp::setup()`, tentar uma nona e tentar separadamente identidades de 64/31 e 63/32 bytes; sobre um ponteiro retornado por `add*Capability()`, comprovar leitura pública, ausência de atribuição pública e chamar os overloads `void` de `rename()` e `applyRenamedName()`; executar ciclos cooperativos. | Assinaturas e defaults permanecem compatíveis salvo os limites e a imutabilidade autorizados; oito capabilities válidas continuam aceitas, a nona continua sujeita ao limite vigente, identidades excedentes falham observavelmente antes do registro sem consumir slot ou criar artefato parcial. O ponteiro retornado continua sendo o da capability registrada; nome e tipo são legíveis, não atribuíveis e não mudam após chamadas aos métodos obsoletos, cujas assinaturas continuam `void`. `handle()` retorna sem espera indefinida por storage. | Inspeção de diff e documentação da API pública; verificação de compilação/tipos para leitura, não atribuição e retornos `void`; teste executado dos ponteiros, métodos obsoletos, limites de identidade, oito slots, falha pré-registro, configuração e ciclo. |
| BCS-AC-022 | BCS-023, BCS-DEC-003 | Executar `pio run -e esp32_dev` e inspecionar as dependências do contrato Core e do código preparatório ESP-IDF afetado. | O build canônico termina com `SUCCESS`; o Core não inclui APIs NVS/Arduino; o provedor Espressif permanece atrás da fronteira de plataforma e nenhuma fonte preparatória ESP-IDF é removida ou tornada dependente do runtime Arduino. Falha preexistente do baseline não autoriza substituir ou dispensar o gate. | Build terminal aprovado em `esp32_dev` e inspeção estática registrada sobre os arquivos alterados. Se o baseline exigir correção alheia, sua autorização e entrega separadas devem anteceder este gate. |
| BCS-AC-023 | BCS-024 | Em um processo limpo, instrumentar a construção de `ServiceManager`, o registro de serviços, `loadSnapshot()` e a ativação do escritor; executar o bootstrap que completa `init()` antes de qualquer callback/task concorrente; depois consultar `instance()` pelo mesmo caminho usado na conclusão do provisionamento, inclusive a partir de contexto que simule a task BLE. Repetir a consulta aos dois accessors. | Os endereços retornados por `init()` e `instance()` são iguais; existe exatamente uma construção do grafo, um registro dos serviços de plataforma, uma leitura inicial do snapshot e uma ativação do escritor. A ativação ocorre somente após `init()` retornar e antes da primeira solicitação de persistência. Aumentar a pilha, omitir logs ou invocar garantia de statics thread-safe sob `-fno-threadsafe-statics` sem eliminar a segunda instância/race reprova. | Teste executado em processo limpo ou seam fiel de inicialização, com asserção de identidade, contadores exatamente iguais a um e evidência da ordem completa; inspeção estática confirma convergência dos accessors e a flag `-fno-threadsafe-statics` no build aplicável. |
| BCS-AC-024 | BCS-025, BCS-026 | Em ESP32-S3 sem settings armazenados, iniciar provisioning, conectar por BLE, habilitar notifications e enviar uma configuração válida fragmentada em escritas de 9, 96, 96, 48 e 3 bytes, encerrando com `END`; observar o resultado de `SettingsManager::save()` e o boot seguinte. | O fluxo não apresenta stack canary, panic, abort ou reboot antecipado. Se `save()` sucede, o commit termina antes do restart controlado; no boot seguinte o cache é carregado, os valores provisionados são preservados e o dispositivo não reentra em provisioning por `NotFound`. | Execução terminal em hardware ESP32-S3 com captura serial completa, prova do sucesso de `save()`/commit e verificação dos settings após o reboot. Execução interrompida, ausência do reboot observado, falta de prova do commit ou somente build classificam o critério como não verificado. |
| BCS-AC-025 | BCS-026 | No caminho de conclusão do provisioning, injetar separadamente sucesso e falha de `SettingsManager::save()` após configuração válida. | No sucesso, restart controlado e status/log de sucesso ocorrem somente depois do retorno bem-sucedido. Na falha, não há restart de sucesso, o status permanece de falha observável e nova tentativa permanece possível. | Teste executado com seam de `save()`, spies de restart/status/log e asserções de ordem. |
| BCS-AC-026 | BCS-027 | Forçar no storage binário as condições que a implementação legada usava para chamar `nvs_flash_erase()` e `ESP_ERROR_CHECK`. | O runtime não aborta, não reinicia e não apaga a partição/namespace global; o domínio binário reporta falha/ausência e os settings sentinela permanecem intactos. | Teste executado com contadores de erase global, códigos de retorno e sobrevivência do processo. |
| BCS-AC-027 | BCS-006, BCS-012 | Fornecer snapshots com checksum correto porém semanticamente inválidos: `used` fora de `{0,1}`, `isOn` fora de `{0,1}`, nome/tipo sem terminador nulo interno e registro ativo com identidade vazia. | Todos são rejeitados antes de qualquer `strcmp`/aplicação; nenhum valor é exposto via `tryGet` nem aplicado em capability. | Casos executados por classe de invalidez semântica, com zero aplicações e diagnóstico de inválido distinto de ausência quando aplicável. |
| BCS-AC-028 | BCS-029, BCS-DEC-004 | Bloquear controladamente o escritor antes de write/commit; produzir transições confirmadas por comando, `handle()` e callback BLE simulado; executar ciclos durante o bloqueio, liberar o escritor e aguardar quiescência. Repetir com transição da mesma identidade durante o commit, oito identidades pendentes e falha de criação do worker. No target ESP32-S3, repetir sob carga medindo stack high-water mark. | Nenhum caminho solicitante executa NVS nem aguarda o escritor; callbacks e `handle()` retornam, ciclos posteriores continuam e não ocorre watchdog. Existe um único worker e um único write/commit em curso, no máximo uma entrada pendente por identidade, a mudança concorrente não é perdida e o estado terminal distingue pendente, em curso, sucesso e falha. Falha de criação é observável e não faz fallback síncrono. Após o commit final e reboot simulado, cada identidade restaura seu valor estável mais recente; no target, a margem mínima observada da pilha do worker permanece em pelo menos 25% da capacidade configurada. | Testes com barreiras, contexto/contador de chamadas NVS, relógio, sondas do escritor, oito identidades, injeção de falha e reboot simulado, mais execução terminal instrumentada no ESP32-S3. Qualquer NVS no contexto solicitante, crescimento além do limite, múltiplos workers, perda de atualização, margem inferior a 25% ou ausência de estado terminal reprova. |

### 8.2 Fidelidade obrigatória dos doubles

Os contratos desta seção ficam preservados como requisitos futuros, mas os
doubles e suítes existentes estão em quarentena conforme `BCS-DEC-007`. Eles
não são executados nem aceitos como evidência da implementação atual.

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
- Relógio usado em testes de cooperatividade e de blink é controlável e avança
  sem espera real desnecessária.
- O seam do escritor assíncrono permite bloquear e liberar write/commit,
  identificar o contexto executor, observar uma única operação em curso,
  inspecionar as oito entradas pendentes consolidadas e aguardar estado
  terminal sem converter quiescência em sucesso presumido.
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
- [x] BCS-002, BCS-022, BCS-AC-002 e BCS-AC-021 fixam a identidade antes do
  registro, removem sua escrita pública, preservam os métodos obsoletos com
  retorno `void` sem mutação e mantêm os ponteiros de `add*Capability()`.
- [x] Os critérios reprovam os desvios apontados em `EKM-CHG-0018`: statics sob
  `-fno-threadsafe-statics`, restart de provisioning sem sucesso de `save()`,
  erase global/`ESP_ERROR_CHECK`, snapshot só com tamanho/versão/checksum,
  identidade com limite interno menor, fallback da valve sem interpreter e
  ausência de oráculo de cooperatividade.
- [x] Validações automatizáveis estão separadas da validação física posterior.
- [x] `BCS-DEC-002` a `BCS-DEC-007` refletem as decisões confirmadas pelo
  Arquiteto; `BCS-DEC-001` permanece explícita e não bloqueante.
- [x] `BCS-DEC-006` encerra `EKM-GAP-0011`.

### 8.4 Gate da implementação

Durante a quarentena definida em `BCS-DEC-007`, as suítes existentes e seus
resultados não integram o gate e não podem ser usados como evidência positiva ou
negativa de conformidade. Para promover a implementação a `Implemented` neste
período, são obrigatórios:

- `pio run -e esp32_dev` terminal com `SUCCESS`; falha preexistente do baseline
  exige correção mínima, autorizada e entregue separadamente antes deste gate,
  sem substituição silenciosa do environment;
- `git diff --check` sem erros;
- revisão estática terminal da implementação, com todos os achados funcionais e
  de segurança encerrados ou explicitamente aceitos pelo Arquiteto;
- matriz BCS-AC preservada, classificando como `Deferred` os critérios cuja
  única evidência disponível dependeria das suítes em quarentena, sem promovê-los
  artificialmente a aprovados.

`pio test -e esp32s3_test` deixa de ser executado como gate enquanto a decisão
estiver vigente. `configs/esp32s3-test.ini` enumera as 18 suítes existentes em
01/08/2026 por `test_ignore`, fazendo o Test Runner marcá-las como `SKIPPED` sem
build, upload ou execução. A reativação exige decisão explícita, remoção
controlada da quarentena e definição de uma estratégia de testes capaz de
produzir evidência confiável. Substituir `esp32_dev` por outro environment
continua sem satisfazer o gate de build.

### 8.5 Gate posterior de validação

`Validated` exige validação em hardware com ao menos duas capabilities binárias
de identidades distintas: aplicar estados opostos, confirmar os estados,
desligar completamente o dispositivo, ligar novamente e observar que ambos são
aplicados ao hardware antes da publicação correspondente. Essa validação física
não substitui nem é exigida para afirmar individualmente os oráculos
automatizados quando reativados; permanece responsabilidade da etapa de
validação posterior.
A quarentena de testes não constitui aprovação de nenhum BCS-AC nem substitui
essa validação física.

## 9. Conhecimento afetado

- `src/Core/Capabilities/CapabilityHelpers.h`;
- `src/SmartSysApp.*`, para ativação do escritor somente após a conclusão da
  inicialização única do grafo;
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
- contrato público de limite e rejeição da identidade autorizado em
  `BCS-DEC-005`, e seu ciclo de vida imutável autorizado em `BCS-DEC-006`;
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

## 11. Decisões do Arquiteto

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

**Estado:** confirmada pelo Arquiteto para a versão 0.5.

Alternâncias produzidas exclusivamente pelo temporizador de `blink` são
transitórias e não devem gerar solicitação, write ou commit. O último estado
estável solicitado permanece como estado persistente durante o modo. Ao
encerrar ou substituir `blink`, o primeiro estado estável confirmado deve ser
solicitado uma única vez se tiver mudado.

**Consequência:** BCS-013, BCS-016 e BCS-AC-015 deixam de estar bloqueados. A
decisão limita desgaste de flash e define deterministicamente o estado
restaurado quando ocorrer reboot durante `blink`.

### BCS-DEC-003 — Gate de build `esp32_dev`

**Estado:** confirmada pelo Arquiteto para a versão 0.5.

`pio run -e esp32_dev` permanece o gate canônico obrigatório. Falha
preexistente e alheia ao domínio binário não autoriza dispensar ou substituir
o environment. Se necessária, a correção mínima do baseline deve receber
autorização e entrega separadas e anteceder a avaliação de BCS-AC-022.

**Consequência:** BCS-AC-022 possui oráculo definido. O critério e a promoção a
`Implemented` permanecem reprovados enquanto o build canônico não terminar com
`SUCCESS`, independentemente de sucesso em outro environment.

### BCS-DEC-004 — Contexto assíncrono da persistência

**Estado:** confirmada pelo Arquiteto para a versão 0.5.

Write e commit NVS devem executar fora de callback BLE, caminho síncrono de
comando e `handle()` das capabilities. Um único escritor assíncrono serializa
as operações. O trabalho pendente é limitado a uma entrada consolidada por
identidade, até oito; estados intermediários da mesma identidade podem ser
substituídos antes do write, mas uma mudança ocorrida durante a operação não
pode ser perdida. Sucesso, falha, trabalho pendente e operação em curso devem
ser observáveis.

**Consequência:** BCS-029 e BCS-AC-028 deixam de estar bloqueados e passam a
reprovar qualquer chamada NVS ou espera pelo commit no contexto solicitante,
fila sem limite, múltiplos escritores ou reboot de verificação antes de estado
terminal bem-sucedido.

### BCS-DEC-005 — Limite público da identidade persistente

**Estado:** confirmada pelo Arquiteto para a versão 0.6.

A revisão da versão 0.5 constatou que a API implementada aceita
`capability_name` como `const char *`, converte o valor para `std::string` e
também permite renomeação sem publicar comprimento máximo. O buffer de 32 bytes
usado apenas para nomes gerados automaticamente não limita nomes fornecidos
externamente. Por isso não existia um "maior comprimento público aceito" que
permitisse implementar e afirmar BCS-002, BCS-AC-002 e BCS-AC-021.

O contrato público limita o `capability_name` definitivo a 63 bytes e `type` a
31 bytes de sua representação UTF-8, sem contar o terminador nulo. Valor acima
do limite é rejeitado observavelmente antes do registro da capability, sem
truncamento, consumo de slot, objeto/adapter parcial, alteração do cache ou
solicitação de persistência. Omissão de nome preserva a geração automática
vigente, seguida da mesma validação sobre o nome definitivo.

**Consequência:** o storage deve preservar campos completos de 63/31 bytes e
seus terminadores; BCS-002, BCS-022, BCS-AC-002 e BCS-AC-021 possuem agora
limites e oráculos executáveis. `EKM-GAP-0010` é encerrada.

### BCS-DEC-006 — Mutabilidade da identidade após o registro

**Estado:** confirmada pelo Arquiteto para a versão 0.6.

O contrato 63/31 e a rejeição pré-registro são assertáveis quando nome e tipo
entram pelo builder. Porém a superfície pública vigente devolve a capability já
registrada e mantém `capability_name` e `type` como `std::string` públicos,
além de expor `rename()` e `applyRenamedName()` com retorno `void`. Um
consumidor pode alterar a identidade depois que slot, capability e adapter já
existem e antes de `SmartSysApp::setup()`, contrariando simultaneamente a
rejeição pré-registro sem efeito parcial e a proibição de caminho alternativo
de renomeação acima dos limites.

O Arquiteto decidiu que `capability_name` e `type` passam a ser imutáveis. O
builder deve gerar ou receber, validar e finalizar ambos antes do registro; o
ponteiro público devolvido por `SmartSysApp::add*Capability()` permanece como
está e oferece leitura, mas não atribuição da identidade.

`rename()` e `applyRenamedName()` permanecem públicos e com retorno `void`, são
marcados como obsoletos e não recebem novos usos. Como não podem contrariar a
imutabilidade, chamadas sobre uma capability registrada preservam
silenciosamente `capability_name` e `type` sem alteração. A manutenção das
assinaturas não mantém a mutabilidade anterior.

**Consequência:** a escrita direta deixa de integrar a API suportada, a
identidade usada pelo registro, cache e storage não pode divergir depois do
registro, e BCS-AC-002/BCS-AC-021 passam a comprovar a imutabilidade, os retornos
`void` e a preservação dos ponteiros públicos. `EKM-GAP-0011` é encerrada.

### BCS-DEC-007 — Quarentena das suítes de teste existentes

**Estado:** confirmada pelo Arquiteto para a versão 0.6.

O Arquiteto determinou que todas as 18 suítes existentes no repositório em
01/08/2026, incluindo as criadas durante esta implementação, são antigas ou
insuficientemente confiáveis para atestar comportamento. Corrigi-las agora
imporia custo desproporcional ao estágio de maturidade do projeto. Elas devem
ser preservadas, mas marcadas para não compilar, carregar nem executar até que
o repositório esteja maduro para uma estratégia de testes confiável.

O mecanismo operacional é `test_ignore` no environment `esp32s3_test`, com
enumeração nominal das suítes atuais. Não se usa curinga: uma suíte futura não
pode ser ignorada silenciosamente sem nova decisão. O Test Runner pode ser
consultado somente em modo de listagem para comprovar `SKIPPED`; seus resultados
anteriores deixam de constituir gate ou evidência desta versão.

**Consequência:** `pio test -e esp32s3_test` fica suspenso como gate. Os
BCS-AC permanecem registrados como contrato futuro, mas os critérios dependentes
dessas suítes são classificados `Deferred`, nunca aprovados por inferência. A
promoção a `Implemented` durante a quarentena depende do build canônico, da
integridade textual e de revisão estática terminal sem achados funcionais ou de
segurança abertos. `Validated` continua exigindo a evidência física da seção
8.5. A reativação requer decisão posterior do Arquiteto.

## 12. Estado da especificação

A versão 0.6 corrige a versão 0.5 para incorporar `BCS-DEC-005`, preservando as
decisões arquiteturais confirmadas após `EKM-CHG-0018`. Além das correções da
versão anterior, ela:

- torna explícita a inicialização única de `ServiceManager` antes de acessos
  concorrentes sob `-fno-threadsafe-statics`;
- condiciona sucesso e restart do provisioning ao sucesso de
  `SettingsManager::save()`;
- proíbe erase global da NVS e abort por checagem fatal no storage binário;
- exige validação estrutural e semântica completa do snapshot;
- publica limites de 63/31 bytes para nome/tipo, exige rejeição observável
  antes do registro e reconcilia integralmente storage e API pública;
- finaliza nome/tipo antes do registro, remove sua escrita pública e preserva
  os métodos obsoletos `void` sem mutação posterior;
- exige interpreter em todos os fallbacks da valve;
- exclui alternâncias transitórias de `blink` da persistência e define a
  consolidação do estado estável ao encerrar o modo;
- mantém `esp32_dev` como gate canônico, tornando eventual correção do baseline
  um pré-requisito separado;
- determina um único escritor assíncrono, limitado e observável para
  write/commit;
- coloca nominalmente em quarentena as 18 suítes existentes e suspende seu uso
  como gate ou evidência até nova decisão de maturidade;
- completa critérios para falhas NVS, isolamento de settings, identidade, valve
  e provisioning após reboot;
- remove metadados Git sem necessidade normativa;
- preserva `BCS-DEC-001` como pendente não bloqueante e registra
  `BCS-DEC-002` a `BCS-DEC-007` como confirmadas.

Os estados normativo, de implementação e de entrega permanecem `Proposed`,
`Not Started` e `Not Ready`. A revisão independente da versão 0.6 registrada em
`EKM-CHG-0023` encontrou `BCS-DEC-006`; após as decisões do Arquiteto, a análise
integral complementar em `EKM-CHG-0025` promove a revisão para `Implementable`,
sem reutilizar o estado de versões anteriores.

### 12.1 Revisão de implementabilidade da versão 0.5

**Resultado:** Precisa de esclarecimento [`Needs Clarification`].

A confrontação integral de BCS-001 a BCS-029, BCS-AC-001 a BCS-AC-028,
decisões, falhas, relações e gates confirmou precedentes implementáveis para a
fronteira Core/plataforma, composição de serviços, inicialização antes de
concorrência, worker FreeRTOS, interpreter da valve, protocolo comum das
capabilities e seams de teste. `BCS-DEC-001` permanece não bloqueante porque
factory reset está explicitamente fora do escopo.

O bloqueio material é `BCS-DEC-005`: a superfície pública não define limite de
comprimento para `capability_name`, mas BCS-002 exige que o storage não seja
mais restritivo e BCS-AC-002/BCS-AC-021 exigem testar o maior comprimento
público aceito. Um implementador teria de inventar um teto público, aceitar
rejeição não autorizada ou assumir uma representação ilimitada incompatível
com storage finito. A lacuna está registrada em `EKM-GAP-0010`.

Como evidência adicional, `pio run -e esp32_dev` terminou `FAILED` antes da
implementação desta versão porque `ESP32_LED_GREEN` e `ESP32_LED_BLUE` não
estão definidos em `src/main.cpp`. BCS-DEC-003 já define esse baseline como
dependência externa: sua correção mínima requer autorização e entrega
separadas antes de BCS-AC-022. O fato não acrescenta decisão ausente à versão
0.5, mas impede aprovação futura do gate enquanto persistir.

Nenhum código, teste ou configuração de implementação foi alterado, e nenhum
teste funcional ou upload foi executado nesta análise.

### 12.2 Autoria da versão 0.6

`BCS-DEC-005` foi incorporada integralmente ao modelo de identidade, BCS-002,
BCS-022, BCS-AC-002, BCS-AC-021, checklist, conhecimento afetado e estado da
especificação. A decisão define limites públicos de 63/31 bytes e rejeição
observável pré-registro sem efeito parcial. `EKM-GAP-0010` foi encerrada.

A autoria deixou a versão 0.6 como `Proposed` / `Not Started` / `Not Ready` /
`Pending Review` e não promoveu a própria revisão de implementabilidade. O
resultado independente posterior está na seção 12.3.

### 12.3 Revisão de implementabilidade da versão 0.6

**Resultado:** Precisa de esclarecimento [`Needs Clarification`].

A confrontação integral de BCS-001 a BCS-029, BCS-AC-001 a BCS-AC-028,
decisões, falhas, relações, dependências e gates confirmou que `BCS-DEC-005`
resolveu o teto do storage e o fluxo inicial do builder. Os precedentes de
Core/plataforma, composição de serviços, inicialização antes da concorrência,
worker FreeRTOS, interpreter da valve, provisioning condicionado a `save()` e
seams de teste continuam suficientes. `BCS-DEC-001` permanece fora do escopo e
não bloqueante; o baseline `esp32_dev` falho continua dependência externa com
contrato responsável já definido em `BCS-DEC-003`.

O bloqueio material é `BCS-DEC-006`: os campos públicos e os métodos de
renomeação permitem alterar nome/tipo depois do registro, mas a versão exige
rejeição antes do registro sem efeito parcial e não define imutabilidade,
rollback, erro observável nem compatibilidade para esse caminho. O
Implementador teria de decidir qual contrato público preservar ou quebrar.

Após esta revisão, o Arquiteto confirmou que `rename()` e
`applyRenamedName()` serão marcados como obsoletos. A decisão parcial reduz a
superfície futura, mas a revisão permanece `Needs Clarification` porque chamadas
legadas e atribuições diretas aos campos públicos continuam sem semântica
normativa para identidade excedente após registro.

Nenhum código, teste ou configuração de implementação foi alterado. Nenhum
build, teste funcional, upload ou validação física foi iniciado nesta revisão.
Uma versão reconciliada com `BCS-DEC-006` deve retornar a análise independente
antes de qualquer ordem de implementação.

### 12.4 Complemento da revisão de implementabilidade da versão 0.6

**Resultado:** Implementável [`Implementable`].

O Arquiteto completou `BCS-DEC-006`: `capability_name` e `type` tornam-se
imutáveis, os métodos públicos obsoletos `rename()` e `applyRenamedName()`
preservam retorno `void` sem alterar a identidade registrada, e os ponteiros
devolvidos por `SmartSysApp::add*Capability()` permanecem como estão. A decisão
foi reconciliada em BCS-002, BCS-020, BCS-022, BCS-AC-002 e BCS-AC-021 e encerra
`EKM-GAP-0011`.

A confrontação integral de BCS-001 a BCS-029, BCS-AC-001 a BCS-AC-028,
decisões, falhas, relações, dependências e gates não encontrou outra decisão
normativa, de produto ou arquitetura ausente. O builder vigente já concentra a
geração do nome e o registro, fornecendo precedente suficiente para finalizar e
validar a identidade antes de expor a capability; a forma interna de obter
imutabilidade permanece escolha de implementação, desde que preserve os
oráculos públicos definidos.

`BCS-DEC-001` continua explicitamente fora do escopo e não bloqueante. A falha
conhecida do baseline `esp32_dev` permanece dependência externa governada por
`BCS-DEC-003`: impede aprovação futura de BCS-AC-022 enquanto existir, mas não
exige nova decisão nesta versão. A implementação permanece `Not Started` e
depende de ordem posterior do Arquiteto.

Nenhum código, teste ou configuração de implementação foi alterado. Nenhum
build, teste funcional, upload ou validação física foi iniciado nesta análise.

### 12.5 Implementação da versão 0.6

**Estado da implementação:** Em andamento [`In Progress`].

A implementação integral do contrato 0.6 foi realizada em código e testes. O
estado permanece `In Progress` porque os dois gates obrigatórios da seção 8.4
não alcançaram estado terminal aprovado por dependências externas descritas
abaixo; nenhum critério comportamental foi executado.

#### Implementação entregue

- identidade pública (BCS-002, BCS-022, `BCS-DEC-005`, `BCS-DEC-006`):
  `ICapability` publica `kMaxCapabilityNameBytes` (63) e
  `kMaxCapabilityTypeBytes` (31); `capability_name` e `type` passaram a ser
  campos de leitura pública sem atribuição pública, e `rename()` /
  `applyRenamedName()` permanecem públicos, `void`, obsoletos e inertes. O
  builder resolve o nome definitivo — fornecido ou gerado —, valida nome e tipo
  e finaliza a identidade antes do registro; a rejeição ocorre antes de criar
  adapter, capability ou slot;
- protocolo comum (BCS-004, BCS-009 a BCS-016, BCS-028): `BinaryCommandCapability`
  concentra restauração, read-back, publicação e solicitação de persistência, e
  toda leitura de estado confirmado passa pelo interpreter quando configurado,
  inclusive nos fallbacks da valve;
- política de `blink` (`BCS-DEC-002`): alternâncias do temporizador são marcadas
  como transitórias pelo `LEDCapability::handle()` e não sinalizam o escritor;
  o encerramento do modo confirma o estado estável uma única vez se mudou;
- escritor assíncrono (BCS-029, `BCS-DEC-004`): o provedor Espressif mantém
  snapshot desejado e snapshot confirmado sob mutex, um único worker FreeRTOS
  serializa write e commit, o caminho solicitante retorna sem tocar a NVS, e
  `BinaryStateWriterStatus` expõe disponibilidade, pendências, operação em
  curso, writes, commits, falhas e último erro;
- storage (BCS-005, BCS-006, BCS-012, BCS-027): campos de 64/32 bytes, versão 2,
  validação estrutural, semântica e de integridade antes de qualquer `strcmp`,
  namespace exclusivo, e nenhum caminho com erase global, `ESP_ERROR_CHECK`,
  abort ou restart;
- grafo de serviços (BCS-024): `ServiceManager::init()` e
  `ServiceManager::instance()` convergem para uma instância única construída em
  armazenamento de escopo de namespace, sem depender de estáticas locais
  thread-safe sob `-fno-threadsafe-statics`; a ativação do escritor ocorre uma
  única vez em `SmartSysApp::setup()`, após a conclusão do grafo;
- provisioning (BCS-025, BCS-026): `ProvisioningController::completeProvisioning()`
  condiciona restart controlado e status/log de sucesso ao sucesso de
  `SettingsManager::save()`, e o callback BLE reutiliza o grafo único.

Foram adicionados os seams exigidos pela seção 8.2: `NvsOps` no provedor
Espressif (falha individual de init, open, read, write e commit; contagem por
operação; ausência de qualquer ponto de entrada de erase), `waitForQuiescence()`
e `writerStatus()` para estado terminal, e contadores de registro/leitura
no `EspressifPlatformServiceRegistrar`.

#### Evidência executada

| Comando | Resultado terminal |
|---|---|
| `pio run -e esp32_dev` | `FAILED` — dois erros preexistentes em `src/main.cpp` (`ESP32_LED_GREEN` e `ESP32_LED_BLUE` não declarados). Idênticos aos do baseline registrado na seção 12.1; nenhum erro novo foi introduzido. |
| `pio test -e esp32s3_test --without-uploading --without-testing` | Compilação aprovada para `test_binary_command_capability_state`, `test_binary_capability_state_storage`, `test_capability_identity`, `test_service_graph_identity`, `test_provisioning_save_gate` e `test_settings_provider`, entre outras. `test_builder`, `test_waterflow`, `test_humidity` e `test_mqtt_settings` falham na compilação. |
| `pio test -e esp32s3_test` | **Não executado.** Nenhum alvo ESP32-S3 conectado (`/dev/cu.*` expõe apenas `Bluetooth-Incoming-Port` e `debug-console`). |
| `git diff --check` | Aprovado, sem erros. |

#### Matriz BCS-AC — resultado terminal

Nenhum critério comportamental foi executado. Compilação não comprova execução;
todos os critérios abaixo permanecem **não verificados**, nunca aprovados.

| Critério | Teste automatizado implementado | Resultado terminal |
|---|---|---|
| BCS-AC-001 | `test_binary_command_capability_state` (restauração e transição por tipo concreto) | Não verificado — não executado |
| BCS-AC-002 | `test_capability_identity`, `test_binary_capability_state_storage` | Não verificado — não executado |
| BCS-AC-003 | `test_binary_command_capability_state` | Não verificado — não executado |
| BCS-AC-004 | `test_binary_command_capability_state` (valve: restore, sync e fallbacks) | Não verificado — não executado |
| BCS-AC-005 | `test_binary_command_capability_state` | Não verificado — não executado |
| BCS-AC-006 | `test_binary_capability_state_storage` (sentinela de settings) | Não verificado — não executado |
| BCS-AC-007 | `test_binary_capability_state_storage` | Não verificado — não executado |
| BCS-AC-008 | `test_binary_capability_state_storage` (mutação de bytes) | Não verificado — não executado |
| BCS-AC-009 | `test_binary_capability_state_storage` (contadores do seam NVS) | Não verificado — não executado |
| BCS-AC-010 | `test_binary_capability_state_storage` (cinco tipos concretos, LED parado e piscando) | Não verificado — não executado |
| BCS-AC-011 | `test_binary_command_capability_state`, `test_binary_capability_state_storage` | Não verificado — não executado |
| BCS-AC-012 | `test_binary_command_capability_state` | Não verificado — não executado |
| BCS-AC-013 | `test_binary_command_capability_state` (transição isolada e rajada) | Não verificado — não executado |
| BCS-AC-014 | `test_binary_command_capability_state` (origens nomeadas) | Não verificado — não executado |
| BCS-AC-015 | `test_binary_command_capability_state` (blink transitório e saída do modo) | Não verificado — não executado |
| BCS-AC-016 | `test_binary_capability_state_storage` (injeção por operação) | Não verificado — não executado |
| BCS-AC-017 | `test_binary_command_capability_state`, `test_binary_capability_state_storage` | Não verificado — não executado |
| BCS-AC-018 | `test_binary_command_capability_state` | Não verificado — não executado |
| BCS-AC-019 | `test_binary_command_capability_state` | Não verificado — não executado |
| BCS-AC-020 | `test_binary_capability_state_storage` (logger capturado, classes distintas e sentinelas privadas) | Não verificado — não executado |
| BCS-AC-021 | `test_capability_identity` (oráculo de tipos compilado com sucesso; asserções em runtime não executadas) | Não verificado — execução não iniciada |
| BCS-AC-022 | Gate `pio run -e esp32_dev` | **Reprovado** — baseline preexistente falho, governado por `BCS-DEC-003` |
| BCS-AC-023 | `test_service_graph_identity` | Não verificado — não executado |
| BCS-AC-024 | Não implementável sem hardware | Não verificado — sem alvo ESP32-S3 |
| BCS-AC-025 | `test_provisioning_save_gate` | Não verificado — não executado |
| BCS-AC-026 | `test_binary_capability_state_storage` | Não verificado — não executado |
| BCS-AC-027 | `test_binary_capability_state_storage` (espelho com checksum correto) | Não verificado — não executado |
| BCS-AC-028 | `test_binary_command_capability_state`, `test_binary_capability_state_storage`; medição de pilha no target não realizada | Não verificado — não executado |

#### Impedimentos materiais

1. **Baseline `esp32_dev` falho.** `ESP32_LED_GREEN` e `ESP32_LED_BLUE` não estão
   definidos em `src/main.cpp`. `BCS-DEC-003` exige autorização e entrega
   separadas para essa correção; ela não foi realizada nesta atuação. BCS-AC-022
   permanece reprovado.
2. **Ausência de alvo ESP32-S3.** Nenhuma porta serial de dispositivo está
   presente, portanto `pio test -e esp32s3_test` não pôde ser iniciado e
   BCS-AC-024 e a medição de pilha de BCS-AC-028 não puderam ser observadas.
3. **Suítes de teste preexistentes quebradas.** `test_builder`, `test_waterflow`,
   `test_humidity` e `test_mqtt_settings` não compilam na `main` nem nesta
   branch: usam assinatura antiga do `CapabilitiesBuilder`, membros de config
   inexistentes (`pin`, `activeHigh`), `CapabilityManager::count` privado e um
   caminho de header inexistente. A verificação foi confirmada com a árvore
   revertida ao baseline. Enquanto persistirem, `pio test -e esp32s3_test` não
   pode alcançar estado terminal aprovado. A correção é alheia ao domínio
   binário e exige autorização e entrega separadas, como em `BCS-DEC-003`.
Nenhum upload, release, deploy ou validação física foi realizado. `BCS-DEC-001`
permanece fora do escopo. A implementação não pode ser promovida a `Implemented`
enquanto os impedimentos acima persistirem.

### 12.6 Revisão técnica da implementação da versão 0.6

**Resultado:** não aprovada para promoção; permanece Em andamento [`In
Progress`] e Não pronta [`Not Ready`].

A revisão independente confrontou a implementação e seus testes com os
requisitos, decisões e critérios da versão 0.6. Foram encontrados três desvios
materiais que não dependem dos impedimentos preexistentes do baseline.

#### Achados

1. **BCS-REV-001 — Alta — falhas de NVS são classificadas como ausência.**
   `EspNvsBinaryCapabilityStateProvider::loadSnapshot()` devolve `Ok` para
   qualquer erro de abertura do namespace e para qualquer erro da consulta de
   tamanho do blob. Somente `ESP_ERR_NVS_NOT_FOUND` representa ausência; erros
   como `ESP_ERR_NVS_NOT_INITIALIZED` são falhas de storage. O teste
   `test_open_and_read_failures_preserve_the_default_flow` injeta precisamente
   `ESP_ERR_NVS_NOT_INITIALIZED` e exige `Ok`, consolidando o comportamento
   contrário a BCS-017, BCS-021, BCS-AC-016 e BCS-AC-020. O desvio oculta
   indisponibilidade do domínio binário como primeiro boot e impede diagnóstico
   correto.
2. **BCS-REV-002 — Alta — comando explícito não substitui `blink`.**
   `LEDCapability::executeCommand()` delega a `power()`, mas nenhum dos caminhos
   encerra `blinking` nem confirma o novo estado estável. Assim, depois do
   comando explícito, `handle()` continua alternando o LED e o estado solicitado
   não é consolidado uma única vez. Os testes exercitam a saída por `blink(0)`,
   mas não a substituição por comando. O comportamento viola BCS-013, BCS-016,
   `BCS-DEC-002` e BCS-AC-015.
3. **BCS-REV-003 — Alta — o oráculo de BCS-AC-028 não foi implementado.** O
   double `FakeBinaryCapabilityStateProvider` informa `inProgress=false` em
   todos os estados e, ao liberar o escritor, simula um write/commit por
   identidade, enquanto o provedor real grava um snapshot consolidado. Não há
   barreira que bloqueie controladamente o escritor real entre write e commit,
   nem casos que comprovem atualização durante o commit, oito identidades
   pendentes e margem mínima de 25% da pilha no target. Portanto os testes
   indicados na matriz da seção 12.5 não implementam o conjunto de oráculos
   exigido por BCS-AC-028 e não podem sustentar conformidade do escritor
   assíncrono.

#### Evidência da revisão

| Comando ou inspeção | Resultado terminal |
|---|---|
| `pio run -e esp32_dev` | `FAILED` — `ESP32_LED_GREEN` e `ESP32_LED_BLUE` continuam não declarados em `src/main.cpp`; BCS-AC-022 permanece reprovado. |
| `pio test -e esp32s3_test` | `FAILED` — 18 suítes coletadas, 0 aprovadas e 18 com erro. As suítes novas que chegaram à etapa de upload não executaram por ausência de hardware; além das quatro suítes citadas na seção 12.5, também houve falha de compilação em `test_waterlevelpercent`, `test_heightwaterlevel`, `test_glp_meter`, `test_operational_color_sensor`, `test_waterlevelliters`, `test_temperature` e `test_glp_sensor`. Nenhum caso de teste foi executado. |
| Inspeção estática de provider, LED, seams e testes | Confirmou BCS-REV-001 a BCS-REV-003. |
| `git diff --check` antes do registro documental | Aprovado, sem erros. |

A evidência desta revisão corrige a limitação factual da seção 12.5 quanto à
execução canônica: o comando completo foi executado nesta atuação e terminou
reprovado. Isso não altera o caráter histórico da evidência registrada pelo
Implementador, mas impede tratá-la como estado atual das suítes.

#### Recomendação do Revisor

Não promover a implementação. Ela deve retornar ao Engenheiro Implementador
para corrigir BCS-REV-001 e BCS-REV-002, implementar os seams e casos completos
de BCS-AC-028 e reconciliar a evidência das suítes. Depois das correções, os
gates da seção 8.4 devem ser repetidos em estado terminal, incluindo execução
em alvo ESP32-S3 para os critérios dependentes de hardware. Os estados
`Proposed`, `In Progress`, `Not Ready` e `Implementable` permanecem inalterados;
esta revisão não fornece aprovação arquitetural, validação física nem
autorização de integração.

### 12.7 Decisão posterior sobre as suítes existentes

O Arquiteto determinou em `BCS-DEC-007` que todas as 18 suítes existentes em
01/08/2026 sejam preservadas e marcadas como `SKIPPED`, inclusive as adicionadas
pela implementação 0.6. A decisão reconhece que esses testes não produzem
evidência confiável no estágio atual e que sua recuperação teria custo
desproporcional.

`configs/esp32s3-test.ini` passou a enumerar nominalmente as 18 suítes em
`test_ignore`. A listagem do PlatformIO confirma todas como `SKIPPED`, sem
build, upload ou execução. O gate da seção 8.4 foi reconciliado: execução de
testes deixa de bloquear ou promover a implementação; critérios dependentes
dessas suítes ficam `Deferred`. O achado BCS-REV-003 deixa de ser correção
exigida para esta implementação e passa a representar dívida futura para a
reativação dos testes. BCS-REV-001 e BCS-REV-002 permanecem achados funcionais
abertos e continuam impedindo aprovação da revisão estática.

A decisão não aprova nenhum BCS-AC, não valida a implementação e não altera os
estados `Proposed`, `In Progress`, `Not Ready` e `Implementable`.

### 12.8 Nova revisão técnica após a quarentena de testes

**Resultado:** não aprovada para promoção; permanece Em andamento [`In
Progress`] e Não pronta [`Not Ready`].

A revisão integral foi repetida sob `BCS-DEC-007`, sem compilar nem executar
qualquer suíte em quarentena e sem usar seus resultados como evidência. A
comparação desde a entrega de código `0d7f151` confirmou que os commits
posteriores alteraram somente especificação, conhecimento e configuração da
quarentena; nenhum código de produção corrigiu os achados funcionais.

#### Achados vigentes

1. **BCS-REV-001 — Alta — aberto.**
   `EspNvsBinaryCapabilityStateProvider::loadSnapshot()` ainda retorna `Ok` e
   registra ausência para qualquer erro de abertura do namespace e de consulta
   do tamanho do blob. Erros diferentes de `ESP_ERR_NVS_NOT_FOUND` continuam
   sendo falhas de storage, não ausência. O comportamento permanece contrário a
   BCS-017 e BCS-021 e impede aprovação estática de BCS-AC-016/020.
2. **BCS-REV-002 — Alta — aberto.** `LEDCapability::executeCommand()` ainda
   chama somente `power()`, sem encerrar `blinking` nem consolidar o primeiro
   estado estável. Um comando explícito durante o modo continua sujeito às
   alternâncias posteriores do temporizador, em desacordo com BCS-013, BCS-016
   e `BCS-DEC-002`; BCS-AC-015 não pode ser aprovado por inspeção.
3. **BCS-REV-003 — Deferred por decisão.** A insuficiência dos seams e oráculos
   do writer permanece tecnicamente existente, mas `BCS-DEC-007` a retirou do
   gate atual e a classificou como dívida para futura reativação dos testes.
   Esta revisão não a converte em aprovação.

#### Evidência e limitações

| Verificação | Resultado terminal |
|---|---|
| Inspeção do diff de produção e das fontes afetadas | BCS-REV-001 e BCS-REV-002 confirmados sem correção; nenhuma nova alteração de produção após `0d7f151`. |
| `pio run -e esp32_dev` | `FAILED` — `ESP32_LED_GREEN` e `ESP32_LED_BLUE` continuam não declarados em `src/main.cpp`; BCS-AC-022 permanece reprovado conforme `BCS-DEC-003`. |
| Suítes de teste | Não executadas por decisão; critérios dependentes permanecem `Deferred` conforme `BCS-DEC-007`. |
| `git diff --check` antes do registro documental | Aprovado, sem erros. |

Nenhum teste, upload, validação física, release ou deploy foi realizado. A
revisão não afirma comportamento operacional onde somente inspeção estática
foi possível.

#### Recomendação do Revisor

Não promover a implementação. Ela deve retornar ao Engenheiro Implementador
para corrigir BCS-REV-001 e BCS-REV-002. O build canônico também deve terminar
com `SUCCESS` por meio da entrega separada governada por `BCS-DEC-003`. Depois
das correções, uma nova revisão estática terminal deve ser ordenada; as suítes
permanecem em quarentena e não integram essa recomendação. Os estados
`Proposed`, `In Progress`, `Not Ready` e `Implementable` permanecem
inalterados, sem aprovação de integração.

## 13. Revisão de implementabilidade da versão 0.2 (histórico contestado)

**Resultado histórico:** Implementável [`Implementable`]

**Status atual deste resultado:** contestado pelos achados posteriores de
implementação parcial, validação e pela avaliação consultiva da linha 0.3;
preservado apenas como evidência histórica. Não autoriza implementação da
versão 0.6.

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

Para a versão 0.6, esse código é precedente tático e fonte de riscos
conhecidos — inclusive erase global da NVS, validação semântica incompleta,
fallback da valve, restart de provisioning incondicional e ausência de oráculo
de cooperatividade — não evidência de conformidade.

## 15. Revisão de implementabilidade da versão 0.3 (histórico contestado)

**Resultado histórico:** Implementável [`Implementable`], registrado em
`EKM-CHG-0017`.

**Status atual deste resultado:** contestado por `EKM-CHG-0018`. Não é
reutilizado por esta atuação e não autoriza implementação da versão 0.6.

Inconsistências materiais registradas na contestação:

1. a revisão afirmou thread-safety de estáticas locais, embora o build use
   `-fno-threadsafe-statics`;
2. apresentou-se como revisão independente e, ao mesmo tempo, preservou
   BCS-001 a BCS-023 por reuso da linha 0.2;
3. manteve gate `esp32_dev` sem caminho de aprovação;
4. concluiu que a correção do singleton bastava para o provisioning, apesar do
   retorno de `save()` ser ignorado;
5. copiou metadado Git sem necessidade normativa.

A autoria da versão 0.5 havia restaurado `Pending Review`; a atuação independente
registrada na seção 12.1 resultou em `Needs Clarification`. A autoria 0.6 da
seção 12.2 incorpora a decisão devolvida e restaura `Pending Review` sem
reutilizar resultados anteriores; a seção 12.3 registra o bloqueio encontrado
em `BCS-DEC-006`, e a análise complementar da seção 12.4 resulta em
`Implementable` após a decisão do Arquiteto.
