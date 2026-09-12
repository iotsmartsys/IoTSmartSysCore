# Especificação — Transporte BLE de comandos V1

**ID:** `IOTSSC-BLE-COMMAND-TRANSPORT`

**Classe da fonte:** Normativa

**Versão:** 0.2

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Pendente [`Pending`]

**Revisão de implementabilidade:** Pendente [`Pending`]

**Bloqueio arquitetural:** Não avaliado formalmente; fronteiras na seção 11.

**Relações normativas e de dependência:** Altera [`Amends`]
`IOTSSC-BLE-COMMAND-TRANSPORT@0.1` para incorporar vínculo local e autenticação
por segredo compartilhado em cada sessão. Preserva
`IOTSSC-PUBLIC-API`, `IOTSSC-RUNTIME`,
`IOTSSC-RUNTIME-CAPABILITY-CAPACITY@0.2`, `ADR-0001` e os contratos das
capabilities, incluindo persistência binária e controle de garagem.

## 1. Objetivo e contexto

Oferecer um caminho BLE local, sob demanda, exclusivamente para comandos de
capabilities. No app Swift, um segundo toggle BLE procura o dispositivo,
conecta, envia o mesmo comando lógico usado pelos transportes atuais, recebe
resposta e desconecta. MQTT continua principal e Serial continua auxiliar.

Baseline inspecionada: `e12fe2ab520e3a20129d56fd06fa4c8937c4ef0a`.
Fatos observados, sem transformar limitações do código em novas garantias:

- `TransportController` compõe `TransportHub`, canais e dispatchers;
  `ITransportChannel` separa transporte de `ITransportDispatcher`.
- `CapabilityCommandTransportDispatcher` usa `ICommandParser`,
  `CommandProcessorFactory` e processadores comuns a todos os transportes.
- `EspIdfCommandParser`, utilizado pelo runtime Arduino, produz `DeviceCommand`
  com `device_id`, `capability_name`, `value`, `type` e `args`.
- `CapabilityCommandProcessor` localiza `ICommandCapability` por nome e chama
  `applyCommand()`. Seu retorno positivo confirma encaminhamento à capability;
  não confirma alteração física, estado final nem aceitação semântica do valor.
- `TransportHub::onMessageReceived()` percorre todos os dispatchers; não para
  no primeiro `true` e não retorna resultado ao canal. Seu forwarding aplica-se
  a mensagens `Raw`. Não existe ali contrato completo de request/reply.
- Serial usa JSON delimitado por LF no caminho ativo; seus helpers binários
  não são o protocolo a copiar para BLE. `TransportKind::Ack` e `id` existem
  como metadados, mas não definem um payload de ACK compartilhado.
- `BleProvisioningChannel` já usa callbacks GAP/GATT globais e pode reinicializar
  o controlador. Não é transporte de comandos nem precedente seguro para
  compartilhar simultaneamente a stack sem análise.

## 2. Escopo

- Identificação por Service Data, serviço GATT fixo e sessão curta por comando.
- Responsabilidades de `BluetoothDispatcher` e do adapter BLE da plataforma.
- Reuso do parser, processadores e capabilities existentes.
- Framing limitado, resposta de encaminhamento, estados, erros e segurança.
- Auth por HMAC-SHA-256, integração local de janela/revogação e persistência
  privada de autorização exclusiva do transporte BLE Control.
- Contrato de interoperabilidade com o app Swift, sem alterar seu repositório.
- BLE opt-in em Arduino sobre ESP32 com hardware BLE; MQTT/Serial preservados.

## 3. Fora de escopo

Telemetria, catálogo de capabilities, sincronização de estados por BLE,
provisioning Wi-Fi, OTA, comandos SYSTEM, factory reset remoto, conexão
permanente, operação em background no iOS, reconexão/reenvio automático,
bridge BLE–MQTT/Serial, mudanças de semântica de capabilities, novo protocolo
lógico de comandos, migração ESP-IDF e suporte ESP8266. Esta atuação produz
somente documentação: não implementa firmware, app, testes ou configuração.

## 4. Configuração, identidade e GATT

### 4.1 Ativação e identidade

- **BLE-001:** habilitação explícita antes de `app.setup()`, default desabilitado.
  Proposta de flag: `IOTSMARTSYS_BLE_COMMAND_ENABLED=0`. Desabilitado não inicia
  stack, advertising ou buffers de sessão, nem altera MQTT/Serial.
- **BLE-002:** o `deviceId` é a identidade vigente do dispositivo nas settings,
  representada por `device_id` no comando. Comparação exata, sensível a caixa;
  sem hash, truncamento, UUID por dispositivo ou dependência de `localName`,
  endereço MAC, nome mostrado pelo sistema ou identificador de `CBPeripheral`.
- **BLE-003:** perfil V1 legado aceita identidade ASCII de 1 a 13 bytes. O
  exemplo `esp32c6-FFFE19` tem 13 bytes. ID vazio, não ASCII ou maior que 13
  impede iniciar BLE com erro de configuração observável; os demais
  transportes continuam. Não encurtar a identidade para fazê-la caber.
- **BLE-004:** o pacote principal conectável e escaneável contém Flags
  (3 bytes) e lista completa com o UUID Control de 128 bits (18 bytes).
  A resposta ao scan contém exclusivamente Service Data de 128 bits:
  comprimento (1), tipo AD `0x21` (1), UUID (16), ID ASCII integral (1–13).
  Total máximo: 31 bytes em cada payload; o ID não leva NUL, JSON, prefixo ou
  versão. Não adicionar nome local que desloque esse campo. O UUID no campo AD
  segue a ordem de bytes BLE; o app usa a chave `CBUUID` do dicionário de
  Service Data, cujo valor já exclui o UUID.

O Service Data integra a descoberta sem conexão, embora esteja na scan
response. O app deve aguardar esse dado: receber apenas a lista de serviços
não identifica o dispositivo. Este perfil não exige advertising estendido.
IDs maiores exigem revisão explícita do perfil; não são silenciosamente
substituídos por Manufacturer Data ou UUID de 16 bits não atribuído.

### 4.2 Serviço e characteristics

Valores de UUID propostos por esta versão, fixos para todos os dispositivos,
a reservar como contrato comum firmware/app antes da implementação:

| Elemento | UUID de 128 bits | Propriedade / conteúdo |
|---|---|---|
| IoTSmartSys Control | `9d8f1000-6f4b-4c65-9d63-4d8a7b210001` | Serviço primário |
| DeviceInfo | `9d8f1001-6f4b-4c65-9d63-4d8a7b210001` | READ; ID ASCII completo |
| Command | `9d8f1002-6f4b-4c65-9d63-4d8a7b210001` | WRITE com resposta ATT |
| Response | `9d8f1003-6f4b-4c65-9d63-4d8a7b210001` | NOTIFY e CCCD padrão `0x2902` |
| Auth | `9d8f1004-6f4b-4c65-9d63-4d8a7b210001` | READ, WRITE com resposta, NOTIFY e CCCD próprio |

- **BLE-005:** Command, Response e Auth são obrigatórias. DeviceInfo é opcional,
  recomendada e habilitada por padrão na proposta. Se presente, o app lê e
  compara com o ID anunciado e esperado; divergência ou falha de leitura
  encerra a sessão antes de escrever. Ausência é permitida e registrada como
  confirmação indisponível. Identidade pública não autentica o equipamento.
- **BLE-006:** não existem characteristics por capability ou por dispositivo.
  O CCCD deve estar habilitado e confirmado antes do primeiro byte de Command.
  Write Without Response não integra a V1. ACK ATT confirma somente a escrita
  do fragmento; a conclusão da transação exige Response. Command permanece
  bloqueada por autorização de aplicação até Auth concluir nesta conexão.
  Ambos os CCCDs devem estar habilitados antes de ler o desafio Auth.

## 5. Comando, framing e resposta

### 5.1 Comando lógico preservado

- **BLE-007:** transmitir o JSON lógico atual, sem envelope BLE, novos opcodes
  ou renomeação de campos. Exemplo para capability que já aceite `ON`:

```json
{"device_id":"esp32c6-FFFE19","capability_name":"luz_sala","type":"CAPABILITY","value":"ON","args":[]}
```

`device_id`, `capability_name` e `value` são strings requeridas pelo parser
vigente. `type` omitido conserva o default `CAPABILITY`; `args` conserva o
array de objetos `{"key":"...","value":"..."}` com strings. O app não deve
criar suporte a `args1`/`args1value`: o parser observado lê esses campos para
log, mas só transfere `args` ao comando resultante.

- **BLE-008:** antes do despacho, exigir `device_id` igual à identidade local
  e capability registrada como `ICommandCapability`. Aceitar somente comandos
  de capability; rejeitar SYSTEM, tipo desconhecido ou valor reconhecido como
  comando de sistema, inclusive com tipo omitido/CAPABILITY. Isso evita a
  promoção automática para SYSTEM existente no dispatcher comum. Essas
  restrições são de entrada BLE; não modificam MQTT/Serial.
- **BLE-009:** não traduzir nomes, valores, args, persistência ou estados. O
  toggle Swift escolhe o comando suportado pela capability, usando o mesmo
  mapeamento do controle atual. O gesto de UI não inventa um opcode `toggle`.

### 5.2 Framing de transporte

- **BLE-010:** uma sessão aceita exatamente um documento de comando UTF-8 em
  uma linha, terminado por LF (`0x0A`). LF interno em string deve estar escapado
  como no JSON. O terminador não pertence ao JSON entregue ao parser.
  O limite proposto é 1024 bytes de JSON, excluído LF; excesso é rejeitado
  sem encaminhar prefixo. Documento incompleto nunca é executado.
- **BLE-011:** dividir os bytes em writes sequenciais com resposta, respeitando
  o limite efetivo da conexão e `maximumWriteValueLength(for: .withResponse)`;
  o perfil usa fragmentos de no máximo `ATT_MTU - 3`. Deve funcionar com MTU 23,
  sem exigir MTU grande ou Prepare/Execute Write. Fragmentos podem cortar um
  caractere UTF-8; decodificar/validar somente após remontagem completa.
- **BLE-012:** após LF, bloquear novos comandos nessa conexão, inclusive linhas
  adicionais no mesmo write. Rejeitar o write que trouxer múltiplas linhas
  antes de despachar qualquer uma. Não há fila de comandos, reordenação ou
  reexecução. Buffers e identificação de sessão são descartados ao desconectar.

### 5.3 ACK/resposta V1

- **BLE-013:** Response usa JSON em linha terminado por LF, máximo 128 bytes
  excluído LF, fragmentado em notificações de no máximo `ATT_MTU - 3`, em ordem.
  O app remonta até LF antes de interpretar. Uma sessão e um comando pendente
  correlacionam a resposta; não se adiciona request ID ao `DeviceCommand`.
- **BLE-014:** resposta positiva: `{"ack":true}`. Significa que o caminho comum
  encaminhou o comando à capability, não que o relé mudou, a garagem abriu ou
  o estado foi publicado. Resposta negativa: `{"ack":false,"error":"CODE"}`.
  Códigos: `INVALID_COMMAND`, `DEVICE_MISMATCH`, `UNSUPPORTED_COMMAND`,
  `CAPABILITY_NOT_FOUND`, `PAYLOAD_TOO_LARGE`, `RECEIVE_TIMEOUT`,
  `DISPATCH_FAILED`, `BUSY` e `INTERNAL_ERROR`. Emitir código específico só
  quando a camada o conhece; `false` do dispatcher comum vira `DISPATCH_FAILED`.
- **BLE-015:** este ACK é novo contrato de resposta do transporte BLE, pois a
  baseline não tem schema comum. Não é um segundo protocolo de comandos e não
  impõe ACK ao MQTT/Serial. NOTIFY não garante entrega à aplicação; ausência de
  resposta completa gera resultado indeterminado se o comando já pode ter sido
  enviado. Nunca apresentar timeout pós-envio como prova de não execução.

## 6. Responsabilidades e integração

- **BLE-016:** adapter BLE da plataforma cuida de rádio, GAP/GATT, segurança,
  fragmentos, CCCD, bonding e eventos de conexão. `BluetoothDispatcher` cuida
  da sessão, autenticação Auth, limites e validação de entrada, invoca uma
  única vez o caminho comum e
  converte seu retorno na resposta. Capabilities e hardware adapters não
  conhecem BLE. Não duplicar parser, factory ou lógica de `applyCommand()`.
- **BLE-017:** manter separação equivalente a canal/dispatcher de MQTT e Serial.
  O despacho BLE deve capturar diretamente o retorno do dispatcher comum numa
  rota exclusiva de entrada BLE. Não registrar simplesmente dois dispatchers
  consumidores no mesmo hub: o hub atual chamaria ambos e poderia executar
  duas vezes. Não presumir que `setOnMessage`, de retorno `void`, devolva ACK.
  O encaixe local pode delegar ao mesmo objeto comum; não exige redesenhar o
  hub nem mudar a interface pública existente para request/reply genérico.
- **BLE-018:** metadados locais, quando usados: `origin="ble"`,
  `kind=Command`, `retain=false`, `hops=0`, tópico lógico
  `device/<device_id>/command`. Respostas são locais à conexão; não circulam
  como comandos. Nenhuma mensagem BLE entra no forwarding Raw, inclusive erro.
- **BLE-019:** copiar dados de callbacks antes de seu lifetime terminar e
  processar comandos pelo ciclo de transporte existente, incluindo fallback
  cooperativo quando a task não existir. Não executar capability em callback
  GAP/GATT nem criar execução concorrente adicional do processador comum.
  Preservar a política de concorrência já vigente para MQTT/Serial.
- **BLE-020:** BLE funciona no ramo operacional sem exigir conexão ao broker
  ou Wi-Fi. Durante provisioning, Control não anuncia nem aceita comandos.
  A V1 não mantém simultaneamente Control e BLE provisioning ativos. Recursos
  BLE de controle são pertencentes à composição da aplicação; start/stop
  repetidos são seguros e não reinicializam stack pertencente a outro serviço.
  Falha de inicialização é local e observável; sem erase global de NVS.
- **BLE-021:** BLE não é capability, não consome slot, não altera catálogo,
  registro, estados ou persistência das capabilities e preserva a capacidade
  configurável vigente
  (default oito, perfil de doze conforme autoridade existente).

## 7. Fluxo e estados

1. Toque no segundo toggle inicia transação explícita; bloqueia novo toque
   enquanto pendente. Toggle MQTT mantém seu fluxo, sem fallback automático.
2. Verificar permissão e rádio no app. Scan em foreground filtrado pelo UUID
   Control; aguardar Service Data e comparar ID completo. Não conectar por nome.
3. Ao encontrar, parar scan e conectar. Descobrir serviço e characteristics,
   confirmar DeviceInfo quando existir e habilitar CCCDs Auth/Response. Ler Auth
   estabelece criptografia/bonding quando necessário. Central nova exige janela
   física aberta; central conhecida reutiliza o vínculo. Ler desafio, escrever
   prova e aguardar Auth positivo antes de Command. Erro em qualquer etapa
   impede o envio. O primeiro vínculo é uma etapa de configuração local, não
   um pareamento automaticamente admitido pelo toque no toggle.
4. Enviar fragmentos em ordem, aguardando conclusão ATT de cada write.
   Aceitar Response válida mesmo se chegar antes do callback do último write;
   mantê-la associada à geração atual da sessão.
5. Receber resposta completa, distinguir encaminhado, rejeitado e indeterminado.
   ACK não atualiza estado físico como confirmado. Desconectar e liberar recursos.
6. Após desconexão, dispositivo volta a anunciar se operacional e habilitado.

| Lado | Estados e transições principais |
|---|---|
| App | Idle → Scanning → Connecting → Discovering → Identifying → Subscribing → Securing → Authenticating → Writing → AwaitingResponse → Disconnecting → Idle |
| Dispositivo | Disabled / Unavailable → Advertising → Connected → Securing → Authenticating → Authorized → Receiving → Dispatching → Responding → AwaitingDisconnect → Advertising |

Estados opcionais sem trabalho são atravessados sem espera. Qualquer falha ou
cancelamento entra em limpeza e desconexão; preservar o resultado observado.
Eventos tardios de sessão anterior são ignorados. Antes de LF, cancelar não
executa comando; depois de possível entrega de LF, cancelamento não desfaz o
comando nem garante que ele não execute. Dois dispositivos com mesmo ID
observados no scan geram identidade ambígua; não selecionar por RSSI como prova.
Uma conexão ativa por peripheral; outro central não interrompe o dono atual.

## 8. Timeouts e condições de borda

Defaults propostos, medidos com relógio monotônico e sem espera bloqueante:

| Etapa | Limite | Resultado |
|---|---|---|
| Scan | 8 s | Dispositivo não encontrado; nenhum comando enviado |
| Conexão | 8 s | Erro de conexão; cancelar tentativa |
| Descoberta, leitura DeviceInfo, assinatura | 5 s por etapa | Erro GATT/identidade; desconectar |
| Segurança de conexão | 15 s | Não autorizado; nenhum comando enviado |
| Auth | 5 s desde leitura do desafio | Falha de autenticação, sem Command |
| Janela de novo vínculo | 60 s desde ação física | Não aprovar candidata após expiração |
| Write | 3 s por fragmento | Erro ATT; pós-envio pode ser indeterminado |
| Remontagem no dispositivo | 5 s desde primeiro byte, sem renovar | RECEIVE_TIMEOUT; descartar e encerrar |
| Espera de resposta | 5 s desde envio de LF | Indeterminado se não recebeu resposta completa |
| Sessão no dispositivo | 30 s desde conexão | Encerrar sessão ociosa/incompleta; não interromper handler em execução |
| Desconexão no app | 2 s | Liberar estado da UI e ignorar callbacks obsoletos; não reutilizar conexão pendente |

- **BLE-022:** limites finitos são configuráveis antes da inicialização;
  não renovar indefinidamente por tráfego inválido. Depois de enfileirar
  resposta, servidor aguarda até 2 s para desconexão do app e então desconecta.
- **BLE-023:** timeout não preempta `applyCommand()` síncrono e não permite
  despachar de novo. Handler que bloquear o ciclo além do limite é limitação
  observável, não sucesso. ACK atrasado não autoriza reenvio.
- **BLE-024:** rádio indisponível, permissão negada, serviço ausente, CCCD
  ausente/negado, falha de segurança ou de notify têm diagnóstico distinto no
  app/log. Sem CCCD ou autorização, rejeitar write por erro ATT antes de
  executar; quando não puder notificar, não inventar resposta recebida.
- **BLE-025:** não repetir automaticamente comandos após desconexão/timeout;
  eles podem ter efeitos não idempotentes. Retry requer nova ação explícita.
  Limpar buffer parcial, timer e assinatura em toda saída, sem vazar conteúdo
  entre centrais. Reboot perde contexto e não reproduz comando anterior.

## 9. Segurança e autorização da sessão

### 9.1 Política confirmada e integração local

- **BLE-026:** exigir, cumulativamente, vínculo BLE autorizado para Control,
  conexão criptografada e prova de conhecimento do segredo nesta conexão.
  Bonding isolado, ID anunciado ou CCCD habilitado não liberam Command.
  Usar LE Secure Connections com Just Works, sem PIN digitado ou programado no
  iOS. O sistema pode apresentar confirmação de pareamento; não se promete
  suprimir diálogos de consentimento ou permissão do sistema operacional.
- **BLE-027:** novos vínculos só são admitidos em janela de 60 s aberta por
  ação física local. Fechada por padrão, não persiste após reboot e fecha no
  primeiro vínculo autorizado ou ao expirar. Uma tentativa nova deve concluir
  bonding e autenticação dentro da janela; expiração impede sua aprovação.
  Centrais já autorizadas podem reconectar fora da janela, sempre com Auth.
- **BLE-028:** a biblioteca fornece operações locais `openPairingWindow()` e
  `revokeBleBonds()`, acionáveis pelo firmware após setup, sem alterar o conjunto
  de capabilities. A configuração pré-setup declara explicitamente integração
  de admissão/revogação e fornece a chave; sem isso, BLE permanece desabilitado
  com diagnóstico. O firmware é dono do botão e traduz gestos físicos distintos
  em chamadas; a biblioteca não escolhe GPIO nem reutiliza o gesto de factory
  reset. Essas operações não são comandos MQTT, Serial ou GATT. A janela
  pode ser aberta novamente apenas por nova ação física deliberada.
- **BLE-029:** validar comprimentos antes de copiar e rejeitar JSON malformado
  ou campos duplicados/ambíguos antes do parser comum. Validação sintática pode
  ser guard local BLE, sem duplicar interpretação das capabilities.
  Não registrar chave, material de
  bonding, prova HMAC ou buffers de Auth em logs, dumps de diagnóstico ou
  mensagens de erro. Registrar somente etapa e código de resultado.

A composição mínima de referência é um firmware que já gerencia um botão e
chama as duas operações a partir de eventos locais distintos. Os gestos e
pinagem pertencem ao firmware consumidor, como integração explícita, sem
mudança em `FactoryResetButtonController`. A biblioteca fornece consulta do
estado de janela/autorização e resultado das operações; a indicação visual é
opcional. Na V1, há **um slot de central autorizada**, além de no máximo uma
candidata em admissão; substituir a central exige revogação local anterior.
Abrir janela com slot ocupado não substitui nem apaga o vínculo: retorna
`ALREADY_BOUND`. Candidata não é uma segunda conexão simultânea.

### 9.2 Segredo compartilhado

- **BLE-030:** usar uma chave aleatória de 32 bytes (256 bits), igual no app e
  em todos os dispositivos deste perfil, fornecida por configuração privada
  antes de setup. Não usar PIN numérico, `deviceId`, senha MQTT ou API token.
  Ausência ou comprimento inválido impede habilitar Control. O runtime deve
  possuir uma cópia válida pelo seu lifetime, sem depender de buffer temporário.
- **BLE-031:** não incluir valor real ou chave de demonstração funcional em
  código versionado, especificação, exemplos, testes, logs ou advertising.
  Injeção privada na distribuição de firmware/app pertence ao responsável do
  produto; esta autoria não gera nem instala chave. O app não mostra a chave
  na UI. Não transmitir a chave, mesmo em link criptografado.

A chave comum comprova posse do segredo do produto, não identidade de usuário,
conta, aplicativo oficial ou dispositivo individual. Ocultá-la na UI não
impede extração por engenharia reversa de app/firmware. Extração em uma cópia
compromete a autenticação de toda a linha. Ação física e vínculo permanecem
barreiras adicionais. Revogar vínculos não troca a chave global; rotação exige
atualização coordenada de firmware/app por seus mecanismos existentes, fora
desta V1. Não há atualização da chave via BLE nem fallback para outra chave.

### 9.3 Troca Auth e formato interoperável

A characteristic Auth é obrigatória e possui READ, WRITE com resposta e NOTIFY
com CCCD próprio. Seu UUID fixo é
`9d8f1004-6f4b-4c65-9d63-4d8a7b210001`. O JSON de Command/Response não muda;
Auth não passa pelo parser ou dispatcher de comandos da biblioteca.

- **BLE-032:** antes de acessar o valor Auth, exigir link criptografado e
  bonding válido: central já autorizada ou candidata admitida pela janela
  física. As permissões GATT de Auth permitem iniciar a negociação de segurança
  pelo iOS; descobrir serviços e habilitar CCCDs não autentica a aplicação.
  Sem janela aberta, novo pareamento é recusado, mesmo conhecendo a chave.
- **BLE-033:** depois de habilitar o CCCD Auth e Response, o app lê Auth. A
  resposta é exatamente `0x01 || nonce`, sendo `nonce` 16 bytes de CSPRNG,
  novo por conexão. Leituras repetidas devolvem o mesmo desafio enquanto
  pendente; não renovam seu prazo. O valor de 17 bytes cabe com MTU 23.
  Não gerar desafio a partir de millis, MAC ou PRNG previsível. Falha da fonte
  aleatória impede autenticação; observar as precondições de entropia da stack.
- **BLE-034:** o app calcula o tag completo de 32 bytes:

```text
HMAC-SHA-256(K,
  ASCII("IoTSmartSys-Control-Auth-v1") || 0x00 ||
  uint8(tamanho do deviceId ASCII) || ASCII(deviceId) || nonce)
```

`K` é a chave de 32 bytes; `||` concatena bytes, sem hexadecimal, Base64, JSON,
NUL extra ou conversão de caixa. O app usa o ID esperado da tela, já conferido
no scan; o servidor usa a identidade local. Isso impede reutilizar o mesmo tag
em desafio diferente ou em outro ID, sem criar identidade criptográfica única.

- **BLE-035:** enviar o tag bruto por writes sequenciais com resposta em Auth,
  no máximo `ATT_MTU - 3` bytes cada; MTU 23 usa 20 + 12 bytes. Remontar
  exatamente 32 bytes, sem LF. Rejeitar excesso antes de verificar e nunca
  aceitar prefixo. Uma prova por conexão; comparação em tempo constante pela
  biblioteca criptográfica. Chave incorreta, tag incompleto, repetido ou
  inválido não autoriza nenhum comando. Não aceitar tag após expirar o desafio.
- **BLE-036:** resultado Auth é uma notificação única de dois bytes:
  `0x01 0x00` para sucesso e `0x01 0x01` para falha genérica. Não confundir esse
  resultado com ACK de Command. O app espera sucesso antes de enviar Command;
  versão ou tamanho inesperado encerra a sessão. O servidor só informa sucesso
  depois de validar o HMAC e, para candidata, persistir sua autorização.
  O app deve preservar o resultado se chegar antes do callback ATT do último
  fragmento, vinculando-o à geração atual da sessão.
- **BLE-037:** Auth tem prazo de 5 s desde a primeira leitura do desafio,
  incluindo remontagem/verificação; não renovar por fragmento ou releitura.
  Falha, cancelamento, perda de criptografia ou desconexão invalida nonce, tag
  parcial e autorização volátil. Nova conexão exige novo desafio, inclusive
  para a mesma central já vinculada. Falha notificada encerra em até 2 s;
  sem CCCD ou possibilidade de notificar, encerra diretamente. Erros locais
  distinguem `AUTH_FAILED`, `AUTH_TIMEOUT`, `AUTH_PROTOCOL_ERROR` e
  `AUTH_STORAGE_ERROR`; falhas ATT anteriores não revelam material secreto.

A identidade integral anunciada continua pública e correlacionável; Auth não
a oculta. Não há garantia de execução exatamente uma vez entre conexões.

O challenge-response não substitui a criptografia BLE nem acrescenta assinatura
por comando. Just Works não oferece proteção MITM no primeiro pareamento;
essa troca não promete impedir relay em tempo real, nem autentica o servidor
perante o app. Essas limitações fazem parte do perfil escolhido; não apresentar
HMAC de sessão como proteção integral contra interceptação ativa. O risco da
chave comum e o uso de Just Works foram explicitados na decisão conversacional.

### 9.4 Bond provisório, persistência e revogação

- **BLE-038:** distinguir bond criado pela stack de autorização persistente
  para Control. Candidata só ocupa o slot autorizado após Auth válido dentro
  da janela. Manter registro local privado de identidade do peer, usando a
  identidade resolvida pelo bonding, não seu endereço aleatório transitório.
  Esse registro é exclusivo do transporte; não altera settings, capabilities
  ou o snapshot binário. Chaves BLE permanecem no armazenamento da stack.
- **BLE-039:** registrar/recuperar a admissão de modo que reboot entre bonding
  e Auth não transforme candidata em autorizada. Bond sem registro autorizado
  nunca dá acesso fora da janela; rejeitar conexão e limpar somente vínculo
  provisório pertencente a Control. Falha de Auth remove a candidata e seus
  recursos, sem remover vínculo já autorizado por mera prova incorreta.
  Sem persistência confirmada, não enviar sucesso de admissão. Falhas de
  armazenamento mantêm Control indisponível, sem erase global nem reparação
  silenciosa de namespaces alheios.
- **BLE-040:** `revokeBleBonds()` fecha janela, bloqueia novos comandos e
  invalida autorização/nonce em memória imediatamente; cancela comando ainda
  não despachado e desconecta. Handler já iniciado não é desfeito. Revogar
  duravelmente o registro Control e remover seus bonds usando a API da stack;
  só informar conclusão após ambos terminarem. Remoção incompleta mantém
  controle bloqueado e exige recuperação antes de nova admissão. Reboot depois
  da revogação persistida não restaura acesso por bond residual. Falha em gravar
  revogação deve ser reportada como falha, sem prometer revogação durável.
  Não apagar Wi-Fi, API, identidade, estados de capabilities ou bonds de
  serviços alheios. Não remover vínculo no iOS por API privada; eventual bond
  obsoleto no telefone pode exigir a recuperação oferecida pelo sistema.

A persistência descrita limita-se à autorização BLE Control e seus candidatos;
não cria serviço genérico de credenciais, arbitragem de stacks ou autenticação
para MQTT/Serial. Não há migração de autorização de versão implementada anterior,
pois 0.1 não foi implementada. Revogar, reabrir e parar o serviço são operações
cooperativas com resultado observável, sem bloqueio por chamadas de rádio.

## 10. Critérios de aceite e validações

Todos são critérios para a futura implementação, sem alegação de execução nesta
autoria. Ausência de evidência permanece `Not Executed`.

| Critério | Cenário, ação e resultado observável | Meio / requisitos |
|---|---|---|
| BLE-AC-001 | BLE desabilitado: baseline compila, não inicia rádio de controle e MQTT/Serial mantêm comportamento | Build e inspeção; 001, 020–021 |
| BLE-AC-002 | ID exemplo: scan iOS encontra UUID e 13 bytes exatos sem conexão; captura confirma dois payloads ≤31 bytes | iPhone + ESP32 + captura; 002–004 |
| BLE-AC-003 | ID longo/inválido não é truncado; BLE falha localmente e outros transportes continuam | Inspeção e execução instrumentada; 003 |
| BLE-AC-004 | DeviceInfo ausente permite fluxo; divergente bloqueia; Command/Response/Auth ou CCCD ausente impede envio | Integração GATT; 005–006, 024 |
| BLE-AC-005 | Mesmo JSON via BLE/MQTT/Serial alcança a mesma capability/args; BLE encaminha uma única vez e nenhum pacote é reenviado ao broker/UART | Instrumentação do dispatcher; 007–009, 016–019 |
| BLE-AC-006 | MTU 23: writes e notificações fragmentados recompõem exatamente mensagem; limites 1024/1025, LF, timeout e duas linhas não executam prefixos inválidos | Exercício instrumentado; 010–013, 022, 029 |
| BLE-AC-007 | Capability existente gera ACK de encaminhamento; ausente, ID divergente, tipo SYSTEM e valor promovível são rejeitados sem efeito | Execução instrumentada; 008, 014 |
| BLE-AC-008 | ACK ATT isolado não significa sucesso; perda da resposta após execução gera indeterminado e nenhum retry | Integração com interrupção controlada; 006, 015, 023–025 |
| BLE-AC-009 | Fluxo Swift termina em desconexão e UI disponível; cancelamentos e callbacks tardios não enviam novo comando | Integração foreground; seções 7–8 |
| BLE-AC-010 | Sem bond, criptografia ou Auth positivo, escrita não executa; janela fechada recusa nova central; operações locais permitem vínculo e revogação sem alterar factory reset/Wi-Fi | Hardware e integração local; 026–032, 038–040 |
| BLE-AC-011 | Sem Wi-Fi/broker, BLE comanda; provisioning não expõe Control; repetir start/stop e fallback sem task não duplica execução nem danifica stack/settings | Hardware e instrumentação; 019–020 |
| BLE-AC-012 | ACK não confirma movimento físico e não altera semântica de estado/persistência; BLE não consome slot | Confronto das fontes e execução de capability representativa; 009, 014, 021 |
| BLE-AC-013 | Com chave privada correta, desafio/prova interoperam entre Swift e firmware com MTU 23; tag errado, replay de outra conexão/ID, versão/tamanho inválidos e timeout não liberam Command | Integração Auth e confronto de bytes/HMAC; 030–037 |
| BLE-AC-014 | Duas conexões da mesma central exigem desafios distintos e duas provas; perder link invalida autorização volátil; callbacks antigos não autorizam nova sessão | Instrumentação por sessão; 033–037 |
| BLE-AC-015 | Reboot após bonding e antes de Auth não autoriza candidata; falha de persistência não produz sucesso; revogação concluída continua válida após reboot; falha de remoção não reabre acesso | Falhas controladas de armazenamento e hardware; 038–040 |
| BLE-AC-016 | Chave não é mostrada, logada ou transmitida; falta/chave de tamanho inválido desabilita Control; slot ocupado não é substituído por abertura da janela | Inspeção de configuração, UI/logs e captura; 027–031 |

Nenhum artefato de teste automatizado integra o recorte desta versão. Os meios
acima são inspeções, builds e cenários instrumentados/manuais, cuja execução
futura requer a autorização operacional aplicável. Build habilitado precisa
nomear environment Arduino/ESP32 efetivamente compatível com BLE; não inferir
suporte ESP32-C6 pela string de identidade ou pelo suporte do chip isoladamente.

## 11. Relações, decisões e lacunas

### Decisões confirmadas pelo pedido

BLE apenas para comandos; segundo toggle Swift; conexão sob demanda; identidade
integral em Service Data de UUID fixo; serviço Control e Command/Response fixos;
DeviceInfo opcional/recomendada; preservação do comando lógico e de capabilities;
somente especificação nesta atuação.

Na revisão 0.2, o Arquiteto escolheu janela física com bonding Just Works e
segredo compartilhado entre app e dispositivos, sem PIN digitado; confirmou
a proposta de desafio criptográfico por conexão, chave longa não transmitida
e revogação local. A limitação de extração de uma chave comum foi apresentada.
A autenticação acrescenta Auth ao GATT, preservando Command/Response.

### Propostas desta versão

UUIDs, limite de ID de 13 bytes no perfil legado, JSON/LF fragmentado, limites
de buffers/timeouts, ACK de encaminhamento, exclusão de SYSTEM e ativação opt-in
permanecem o desenho desta especificação. Na 0.2, tamanho da chave, HMAC-SHA-256,
formato binário Auth, um slot de central, integração por operações locais e
regras de persistência concretizam a política escolhida. Não são mecanismos
já implementados nem uma promessa de segredo inextragível. Não atribuir
`Ready` ou autorização de implementação ao pedido de escrita documental.

### Autoridades preservadas e fronteiras a analisar

- API e runtime: extensão aditiva, configuração pré-setup, mesmo processamento
  cooperativo e nenhuma mudança aos defaults de MQTT/Serial.
- Capacidade: prevalece `RUNTIME-CAPABILITY-CAPACITY@0.2` e `ADR-0001` sobre as
  menções históricas a oito slots fixos. BLE não muda essa regra.
- Capabilities: preservam-se comandos, estados, persistência e adapters,
  particularmente `BINARY-COMMAND-STATE-PERSISTENCE` e `GARAGE-CONTROL-STATE`.
- `EKM-GAP-0001` (evidência de compatibilidade), `EKM-GAP-0003` (domínios de
  transporte/provisioning pouco especificados) e `EKM-GAP-0004` (plataformas)
  permanecem abertos; esta autoria não os encerra nem cria débito aceito.
- Analisar o encaixe exclusivo de `BluetoothDispatcher`, start sem broker e
  transição provisioning/operacional. Se exigir proprietário global novo da
  stack, alterações de callbacks ou arbitragem que afetem outros serviços,
  registrar pré-requisito arquitetural separado/ADR; não ampliar esta V1.
- Vínculo e revogação passam a ser contratados por BLE-026 a BLE-040, com
  responsabilidade do firmware pelos eventos físicos e operações locais da
  biblioteca. Auth e registro de autorização são exclusivos do transporte.
  Reavaliar implementabilidade dessa extensão na baseline, sem presumir que o
  provisioning a oferece e sem criar infraestrutura transversal por inferência.
- Confirmar target, stack e exposição de Service Data da scan response no iOS
  suportado. BLE de provisioning usa Bluedroid; sua presença não qualifica
  automaticamente todos os modelos ESP32 nem compatibilidade de stacks.
- Os perfis centrais consultados declaram EKOM 5.0, enquanto AGENTS declara 4.7
  e diretrizes locais 4.6. Esta autoria segue o padrão documental local e não
  migra governança nem declara Repository Readiness inexistente como aprovada.

## 12. Conhecimento afetado e estado

Registrar esta fonte no índice, árvore e diagrama de `KNOWLEDGE-MAP.md`, com
estado `Draft`, e abrir transação em `EKOM-CHANGELOG.md`. Não alterar fontes
vigentes de capabilities, API, runtime ou relatórios históricos.

Versão 0.2 incorpora a decisão do Arquiteto e trata a pendência de vínculo e
revogação apontada como BLE-AN-001 na análise 0.1. O relatório histórico
`docs/reports/2026-09-12T021149Z-0.1-7b7674aa-implementability-analysis.md`
permanece imutável e aplicável somente à revisão confrontada. A disposição
formal do bloqueador cabe à reanálise da 0.2, ainda pendente.

Estado permanece `Draft`/`Pending`/`Not Started`: nenhuma prontidão,
qualificação de hardware, validação de segurança ou ordem de implementação é
inferida da confirmação de desenho. Nenhum código, teste, build, geração de
chave ou operação física integra esta revisão documental.

## 13. Fontes técnicas consultadas

Fontes locais: `src/Contracts/Transports/`, `src/Core/Transports/TransportHub.cpp`,
`src/App/Managers/TransportController.cpp`, `src/Core/Commands/`,
`src/Contracts/Events/DeviceCommand.h`,
`src/Platform/Espressif/Parsers/EspIdfCommandParser.cpp`,
`src/Platform/Arduino/Transports/ArduinoSerialTransportChannel.cpp`,
`src/Platform/Espressif/Provisioning/BleProvisioningChannel.cpp` e
`src/SmartSysApp.cpp`. `docs/REPO_DOSSIER.md` é informativo, subordinado às specs.

Referências externas, consultadas em 12/09/2026:

- [Bluetooth SIG — Service Data, UUID de 128 bits e tipos AD](https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/CSS_v14/out/en/core-supplementary-features/data-types-specification.html).
- [Silicon Labs — limite e composição do advertising legado](https://docs.silabs.com/bluetooth/5.0/bluetooth-fundamentals-advertising-scanning/).
- [Apple — CBAdvertisementDataServiceDataKey](https://developer.apple.com/documentation/corebluetooth/cbadvertisementdataservicedatakey).
- [Apple — maximumWriteValueLength(for:)](https://developer.apple.com/documentation/corebluetooth/cbperipheral/maximumwritevaluelength(for:)).

- [RFC 2104 — HMAC](https://www.rfc-editor.org/info/rfc2104/).
- [Espressif — GAP e gerenciamento de bonds](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_gap_ble.html).
- [Espressif — entropia e geração aleatória](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/system/random.html).
