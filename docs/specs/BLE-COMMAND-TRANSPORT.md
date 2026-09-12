# Especificação — Transporte BLE de comandos V1

**ID:** `IOTSSC-BLE-COMMAND-TRANSPORT`

**Classe da fonte:** Normativa

**Versão:** 0.1

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Pendente [`Pending`]

**Revisão de implementabilidade:** Pendente [`Pending`]

**Bloqueio arquitetural:** Não avaliado formalmente; fronteiras na seção 11.

**Relações normativas e de dependência:** Novo [`New`]. Preserva
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

- **BLE-005:** Command e Response são obrigatórias. DeviceInfo é opcional,
  recomendada e habilitada por padrão na proposta. Se presente, o app lê e
  compara com o ID anunciado e esperado; divergência ou falha de leitura
  encerra a sessão antes de escrever. Ausência é permitida e registrada como
  confirmação indisponível. Identidade pública não autentica o equipamento.
- **BLE-006:** não existem characteristics por capability ou por dispositivo.
  O CCCD deve estar habilitado e confirmado antes do primeiro byte de Command.
  Write Without Response não integra a V1. ACK ATT confirma somente a escrita
  do fragmento; a conclusão da transação exige Response.

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
  fragmentos, CCCD e eventos de conexão. `BluetoothDispatcher` cuida da sessão,
  limites e validação de entrada, invoca uma única vez o caminho comum e
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
  registro, estados ou persistência e preserva capacidade configurável vigente
  (default oito, perfil de doze conforme autoridade existente).

## 7. Fluxo e estados

1. Toque no segundo toggle inicia transação explícita; bloqueia novo toque
   enquanto pendente. Toggle MQTT mantém seu fluxo, sem fallback automático.
2. Verificar permissão e rádio no app. Scan em foreground filtrado pelo UUID
   Control; aguardar Service Data e comparar ID completo. Não conectar por nome.
3. Ao encontrar, parar scan e conectar. Descobrir serviço e characteristics,
   estabelecer segurança, confirmar DeviceInfo quando existir e habilitar
   Response. Erro em qualquer etapa impede Command.
4. Enviar fragmentos em ordem, aguardando conclusão ATT de cada write.
   Aceitar Response válida mesmo se chegar antes do callback do último write;
   mantê-la associada à geração atual da sessão.
5. Receber resposta completa, distinguir encaminhado, rejeitado e indeterminado.
   ACK não atualiza estado físico como confirmado. Desconectar e liberar recursos.
6. Após desconexão, dispositivo volta a anunciar se operacional e habilitado.

| Lado | Estados e transições principais |
|---|---|
| App | Idle → Scanning → Connecting → Discovering → Securing → Identifying → Subscribing → Writing → AwaitingResponse → Disconnecting → Idle |
| Dispositivo | Disabled / Unavailable → Advertising → Connected → Authorized → Receiving → Dispatching → Responding → AwaitingDisconnect → Advertising |

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

## 9. Segurança mínima proposta

- **BLE-026:** canal desabilitado por padrão. Para controle, exigir conexão
  criptografada e central previamente autorizada por bonding. ID, UUID e
  proximidade são descoberta, nunca credenciais. Sem segurança, falhar fechado.
- **BLE-027:** novos bonds só podem ser admitidos após ação física local
  explícita, em janela proposta de 60 s; janela fechada por padrão e após um
  vínculo. Não reutilizar senha MQTT, API token ou identidade como segredo BLE.
  Adotar pareamento autenticado quando o produto dispõe de I/O/OOB; Just Works
  não oferece proteção MITM e só pode ser escolhido mediante aceitação
  explícita desse risco pelo responsável do produto.
- **BLE-028:** firmware consumidor deve fornecer procedimento local de
  revogação de autorização sem mudar implicitamente o factory reset existente.
  Sem fluxo definido de vínculo/revogação, BLE Control permanece desabilitado.
  Bonding desta proposta é restrito ao controle; não autoriza redesenhar
  provisioning, persistência geral ou política de autenticação da biblioteca.
- **BLE-029:** limitar recursos a uma sessão/um comando, validar comprimentos
  antes de copiar e rejeitar JSON malformado ou campos duplicados/ambíguos antes
  de invocar parser comum. Validação sintática pode ser guard local BLE, sem
  reimplementar interpretação de capabilities. Registrar etapa, erro e ID;
  não registrar chaves de vínculo nem credenciais.

O identificador completo anunciado é público e permite correlação por
observadores. Isso é consequência do requisito de descoberta desta V1;
criptografia da conexão não esconde o advertising. Não se promete autenticação
por DeviceInfo nem execução exatamente uma vez entre conexões.

## 10. Critérios de aceite e validações

Todos são critérios para a futura implementação, sem alegação de execução nesta
autoria. Ausência de evidência permanece `Not Executed`.

| Critério | Cenário, ação e resultado observável | Meio / requisitos |
|---|---|---|
| BLE-AC-001 | BLE desabilitado: baseline compila, não inicia rádio de controle e MQTT/Serial mantêm comportamento | Build e inspeção; 001, 020–021 |
| BLE-AC-002 | ID exemplo: scan iOS encontra UUID e 13 bytes exatos sem conexão; captura confirma dois payloads ≤31 bytes | iPhone + ESP32 + captura; 002–004 |
| BLE-AC-003 | ID longo/inválido não é truncado; BLE falha localmente e outros transportes continuam | Inspeção e execução instrumentada; 003 |
| BLE-AC-004 | DeviceInfo ausente permite fluxo; divergente bloqueia; Command/Response ou CCCD ausente impede envio | Integração GATT; 005–006, 024 |
| BLE-AC-005 | Mesmo JSON via BLE/MQTT/Serial alcança a mesma capability/args; BLE encaminha uma única vez e nenhum pacote é reenviado ao broker/UART | Instrumentação do dispatcher; 007–009, 016–019 |
| BLE-AC-006 | MTU 23: writes e notificações fragmentados recompõem exatamente mensagem; limites 1024/1025, LF, timeout e duas linhas não executam prefixos inválidos | Exercício instrumentado; 010–013, 022, 029 |
| BLE-AC-007 | Capability existente gera ACK de encaminhamento; ausente, ID divergente, tipo SYSTEM e valor promovível são rejeitados sem efeito | Execução instrumentada; 008, 014 |
| BLE-AC-008 | ACK ATT isolado não significa sucesso; perda da resposta após execução gera indeterminado e nenhum retry | Integração com interrupção controlada; 006, 015, 023–025 |
| BLE-AC-009 | Fluxo Swift termina em desconexão e UI disponível; cancelamentos e callbacks tardios não enviam novo comando | Integração foreground; seções 7–8 |
| BLE-AC-010 | Sem bond/link seguro, escrita não executa; janela fechada recusa novo bond; revogação retira acesso | Hardware e procedimento de produto; 026–029 |
| BLE-AC-011 | Sem Wi-Fi/broker, BLE comanda; provisioning não expõe Control; repetir start/stop e fallback sem task não duplica execução nem danifica stack/settings | Hardware e instrumentação; 019–020 |
| BLE-AC-012 | ACK não confirma movimento físico e não altera semântica de estado/persistência; BLE não consome slot | Confronto das fontes e execução de capability representativa; 009, 014, 021 |

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

### Propostas desta versão

UUIDs, limite de ID de 13 bytes no perfil legado, JSON/LF fragmentado, limites
de buffers/timeouts, ACK de encaminhamento, exclusão de SYSTEM, ativação opt-in
e segurança das seções 4–9 são proposta técnica do Autor, não decisões humanas
anteriores nem comportamento já implementado. Não atribuir aprovação de risco,
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
- Vínculo seguro, ação física e revogação precisam de definição de produto e
  análise na baseline. Não foi localizado contrato transversal vigente que os
  forneça para Control. Se exigirem infraestrutura independente, mantê-la como
  dependência preparatória, sem presumir que o provisioning já a oferece.
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

Versão 0.1 escrita por solicitação explícita do Arquiteto. Análise formal de
implementabilidade pendente; implementação não iniciada. A proposta é
reviewable e não constitui qualificação de hardware, validação de segurança ou
ordem para implementar. Nenhum código, teste, build ou operação física integra
a presente autoria.

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
