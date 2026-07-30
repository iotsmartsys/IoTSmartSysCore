# Especificação — Persistência do estado de comandos binários

**ID:** IOTSSC-BINARY-COMMAND-STATE

**Classe da fonte:** Normativa

**Versão:** 0.1

**Estado normativo:** Proposta [`Proposed`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Não pronta [`Not Ready`]

**Revisão de implementabilidade:** Implementável [`Implementable`]

**Relação normativa:** Nova [`New`]

## 1. Objetivo e contexto

Fazer com que toda capability baseada em `BinaryCommandCapability` restaure no
boot o último estado binário registrado e o aplique ao hardware, em vez de
permanecer no valor default após cada reinicialização.

Hoje, `BinaryCommandCapability` começa com o valor de desligado definido pelo
tipo concreto. Durante o `setup()`, o adapter de comando é inicializado e seu
estado corrente passa a ser o estado lógico da capability. Como não existe
persistência própria para esse domínio, uma reinicialização perde a última
intenção efetivamente aplicada.

A intenção confirmada pelo Arquiteto é:

- registrar o estado em NVS sempre que o estado binário mudar;
- manter a leitura de boot leve;
- aplicar, durante a inicialização, o último estado validamente registrado para
  cada capability abrangida.

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
- a implementação Espressif vigente já usa NVS, mas não existe um contrato de
  persistência de estado de capability que possa ser reutilizado sem misturar
  esse estado com settings do dispositivo.

Esses fatos descrevem a implementação atual e sustentam a proposta; não
constituem por si mesmos requisitos legados.

## 3. Escopo

- todas as instâncias cujo comportamento concreto deriva de
  `BinaryCommandCapability`;
- identificação estável do registro por capability;
- representação persistida dos dois estados lógicos suportados;
- leitura, validação e cache do snapshot durante o boot;
- aplicação do estado restaurado ao hardware durante `setup()`;
- gravação e commit em NVS após cada mudança lógica confirmada;
- tratamento de ausência, corrupção, incompatibilidade e falha de storage;
- logs diagnósticos sem dados privados;
- testes automatizados do Core e do provedor NVS;
- build do runtime Arduino sobre ESP32.

## 4. Fora de escopo

- persistir estados de capabilities que não derivem de
  `BinaryCommandCapability`;
- persistir comandos transitórios como o texto `toggle`;
- alterar nomes públicos de capabilities, comandos, tipos ou estados;
- alterar defaults públicos das capabilities;
- alterar o limite de oito capabilities;
- sincronizar o estado persistido com API remota, MQTT ou settings do
  dispositivo;
- criar histórico de estados, contador de acionamentos ou telemetria;
- adicionar debounce, atraso ou agrupamento de gravações;
- suportar ESP8266;
- promover ESP-IDF a runtime suportado;
- upload, release ou deploy.

## 5. Solução arquitetural proposta

### 5.1 Fronteira de persistência

Deve ser criado um contrato de storage específico para estados binários no
Core e uma implementação Espressif baseada em NVS. A implementação de
`BinaryCommandCapability` não pode incluir nem chamar diretamente APIs
`Preferences`, `nvs_*` ou headers específicos de plataforma.

O serviço deve ser registrado no bootstrap de plataforma e disponibilizado às
capabilities pelo mecanismo de composição já usado pelo projeto. Construtores
e formas públicas de registro existentes devem permanecer compatíveis.

Essa proposta adiciona uma nova fronteira de persistência porque não existe
precedente equivalente para estado de capabilities. Ela preserva o padrão
vigente de contrato no Core, implementação em `Platform/Espressif` e composição
no registro de serviços; não autoriza uma camada estrutural adicional fora
dessas responsabilidades.

### 5.2 Modelo de armazenamento

O provedor Espressif deve usar namespace NVS próprio, sem compartilhar nem
reescrever o blob de settings. O conteúdo deve ser um snapshot compacto e
versionado, com no máximo oito registros ativos.

Cada registro deve conter:

- identidade composta pelo `capability_name` definitivo e pelo `type`;
- estado semântico binário `off` ou `on`;
- informação suficiente para rejeitar formato truncado, incompatível ou
  corrompido.

O estado semântico deve ser convertido para `_offValue` ou `_onValue` pela
própria `BinaryCommandCapability`. Assim, o mesmo storage atende vocabulários
como `off`/`on` e `closed`/`open` sem persistir comandos transitórios.

O snapshot deve ser obtido por uma única leitura de dados da NVS durante a
inicialização do serviço. Consultas posteriores no mesmo boot devem ocorrer
somente no cache em memória. O ciclo cooperativo de `handle()` não pode fazer
leituras NVS.

Uma gravação deve atualizar o snapshot em memória, persistir o blob e executar
o commit NVS. Gravação e leitura não podem tocar o namespace ou a chave de
settings.

### 5.3 Identidade e ciclo de vida

A identidade somente pode ser consultada depois que o builder tiver definido o
nome definitivo da capability. Alterar `capability_name` ou `type` entre boots
cria outra identidade e não pode aplicar automaticamente o registro da
identidade anterior.

O fluxo de inicialização proposto é:

```text
bootstrap da plataforma
→ inicialização do storage e leitura única do snapshot NVS
→ construção e nomeação das capabilities
→ setup do adapter de comando
→ consulta em cache por (capability_name, type)
→ registro válido encontrado
  → conversão para o valor binário concreto
  → aplicação no adapter
  → leitura de confirmação do adapter
  → atualização/publicação do estado confirmado
→ registro ausente ou inválido
  → preservação do fluxo e do default vigentes
```

O estado persistido não pode ser anunciado como aplicado antes de o adapter
aceitar o comando e sua leitura confirmar o valor correspondente.

### 5.4 Registro de mudanças

Toda transição lógica confirmada entre os dois valores binários deve solicitar
imediatamente a atualização e o commit do snapshot. Isso inclui transições
originadas por:

- comandos remotos;
- chamadas públicas como `turnOn`, `turnOff`, `power` e `toggle`;
- sincronização com mudança observada no adapter;
- comportamento automático de uma classe derivada, inclusive `blink`.

Repetir o mesmo valor não constitui mudança e não pode gerar nova gravação. O
texto `toggle` nunca deve ser persistido; deve ser registrado apenas o estado
resultante confirmado.

O caminho comum de `BinaryCommandCapability` deve centralizar a detecção da
transição e a persistência, para que classes derivadas não precisem duplicar o
protocolo. Uma falha de gravação deve ser registrada em log, mas não pode
reverter um comando já aplicado nem impedir o processamento cooperativo.

## 6. Requisitos

- **BCS-001:** toda capability derivada de `BinaryCommandCapability` deve
  participar automaticamente da restauração e persistência, sem configuração
  opt-in por tipo concreto.
- **BCS-002:** a identidade persistente deve combinar o
  `capability_name` definitivo e o `type`.
- **BCS-003:** o storage deve representar somente os estados semânticos
  binários `off` e `on`.
- **BCS-004:** cada estado semântico restaurado deve ser convertido para o
  `_offValue` ou `_onValue` do tipo concreto antes de ser aplicado.
- **BCS-005:** o provedor deve manter os estados em namespace NVS exclusivo e
  não pode alterar o blob de settings.
- **BCS-006:** o formato persistido deve ser versionado, limitado a oito
  registros ativos e capaz de rejeitar conteúdo incompleto, incompatível ou
  corrompido.
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
- **BCS-012:** versão desconhecida, conteúdo inválido, corrupção ou identidade
  não correspondente devem ser tratados como ausência de registro para a
  capability afetada.
- **BCS-013:** toda transição confirmada de `off` para `on` ou de `on` para
  `off` deve atualizar o cache, gravar o snapshot e executar commit NVS.
- **BCS-014:** uma tentativa de atribuir novamente o estado lógico corrente não
  pode gerar gravação nem commit.
- **BCS-015:** `toggle` deve persistir somente o valor final confirmado, nunca
  o comando transitório.
- **BCS-016:** mudanças originadas por comandos, API pública, leitura do
  adapter ou automação de classe derivada devem seguir o mesmo protocolo de
  persistência.
- **BCS-017:** falha ao abrir, gravar ou executar commit na NVS não pode
  bloquear o loop, reiniciar o dispositivo nem desfazer o estado de hardware
  já confirmado.
- **BCS-018:** após falha de persistência, o estado lógico e sua publicação
  devem continuar refletindo o hardware; o último estado registrado continua
  sendo o último commit concluído com sucesso.
- **BCS-019:** a restauração de uma capability não pode alterar outra
  capability, mesmo quando ambas possuem o mesmo estado semântico.
- **BCS-020:** mudança de nome ou tipo entre boots não pode reutilizar
  silenciosamente o registro da identidade anterior.
- **BCS-021:** logs devem permitir distinguir ausência de registro, registro
  inválido, falha de aplicação e falha de persistência, sem expor settings,
  credenciais ou conteúdo privado.
- **BCS-022:** APIs públicas, defaults, ordem de configuração antes de
  `SmartSysApp::setup()`, processamento cooperativo e limite de oito
  capabilities devem ser preservados.
- **BCS-023:** a implementação Espressif deve operar no runtime Arduino sobre
  ESP32 e não pode degradar o código preparatório para ESP-IDF.

## 7. Falhas e condições de borda

- Primeiro boot ou NVS apagada: cada capability usa o default vigente; a mera
  inicialização no default não cria uma mudança a persistir.
- Snapshot parcialmente escrito, corrompido ou de versão desconhecida: nenhum
  valor não validado é aplicado; o dispositivo continua com os defaults.
- Registro de outra identidade: o registro é ignorado para a capability atual.
- Comando rejeitado pelo adapter: o estado solicitado não é persistido.
- Aplicação aceita, mas leitura não confirma o valor: o valor solicitado não é
  anunciado nem persistido como estado confirmado.
- Falha de commit: o runtime mantém o estado confirmado em memória e hardware,
  registra a falha e tenta persistir novamente apenas quando ocorrer outra
  transição; esta especificação não cria fila de retry.
- Reinicialização após commit bem-sucedido: o valor confirmado é restaurado.
- Reinicialização após falha de commit: restaura-se o último snapshot cujo
  commit foi concluído com sucesso.
- Duas capabilities no mesmo estado: cada registro permanece isolado pela
  identidade composta.
- `LEDCapability` em `blink`: cada alternância confirmada é uma mudança e,
  portanto, produz commit. A frequência de escrita resultante é consequência
  direta do requisito de persistir toda mudança; otimização por atraso ou
  agrupamento está fora do escopo.

## 8. Critérios de aceite e evidências esperadas

| Requisito | Evidência esperada |
|---|---|
| BCS-001 a BCS-004 | Testes com switch, light, LED, switch plug e valve, comprovando conversão dos dois vocabulários |
| BCS-005 e BCS-006 | Inspeção do namespace/formato e testes de versão, limite, truncamento e corrupção |
| BCS-007 e BCS-008 | Provedor instrumentado comprovando uma leitura de dados no boot e zero leituras durante consultas e `handle()` |
| BCS-009 e BCS-010 | Testes da ordem `adapter setup → restore → apply → read-back → state event` |
| BCS-011 e BCS-012 | Testes de primeiro boot, identidade ausente, versão desconhecida e snapshot inválido |
| BCS-013 a BCS-016 | Testes de mudança por comando, chamadas públicas, toggle, adapter e blink, com uma gravação/commit por transição e nenhuma por repetição |
| BCS-017 e BCS-018 | Injeção de falhas de open, write e commit, comprovando continuidade do runtime e retenção do último commit válido |
| BCS-019 e BCS-020 | Testes com múltiplas capabilities, nomes iguais com tipos distintos e mudança de identidade entre boots |
| BCS-021 | Inspeção dos logs de cada classe de resultado, sem dados privados |
| BCS-022 e BCS-023 | Inspeção de API pública e build `esp32_dev` |

Também são obrigatórios:

- testes PlatformIO/Unity do protocolo de restauração e persistência;
- teste do provedor contra NVS real ou emulação equivalente que preserve
  semântica de commit;
- `pio run -e esp32_dev`;
- `pio test -e esp32s3_test`;
- `git diff --check`.

Build e testes automatizados aprovados permitem promover a implementação para
`Implemented`. Validação em hardware deve comprovar ao menos duas capabilities
binárias com identidades distintas, desligamento completo, novo boot e aplicação
dos dois últimos estados antes da promoção para `Validated`.

## 9. Conhecimento afetado

- `src/Core/Capabilities/CapabilityHelpers.h`;
- `src/Contracts/Capabilities/ICapability.h` e/ou o contrato de consulta segura
  necessário para identificar o comportamento binário sem RTTI;
- novo contrato de storage no domínio de capabilities;
- `src/Contracts/Providers/ServiceProvider.*`;
- `src/Platform/Espressif/Providers/EspressifPlatformServiceRegistrar.*`;
- novo provedor Espressif de estado binário em NVS;
- `src/App/Builders/Builders/CapabilitiesBuilder.*` e/ou
  `CapabilityManager`, conforme a composição aprovada na revisão;
- testes, mocks e configuração de teste aplicáveis;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

A lista identifica responsabilidades provavelmente afetadas, não autoriza
reorganização estrutural nem obriga a alterar todos os arquivos citados.

## 10. Relações

- `IOTSSC-PUBLIC-API`;
- `IOTSSC-RUNTIME`;
- `EKM-CHG-0009`.

## 11. Decisão pendente do Arquiteto

**BCS-DEC-001 — Factory reset:** a intenção recebida não determina se o factory
reset deve apagar também o snapshot de estados binários.

A solução recomendada é apagar o namespace desta funcionalidade junto com os
demais dados persistentes de dispositivo, para que o primeiro boot após factory
reset use os defaults. Essa recomendação ainda não é uma decisão confirmada e
não autoriza alteração do fluxo de factory reset.

A revisão de implementabilidade deve classificar o impacto dessa decisão sobre
o recorte. O comportamento de reboot comum, queda de energia e atualização de
firmware permanece completamente especificado independentemente dela.

**Classificação da revisão:** não bloqueante para esta especificação. Factory
reset não integra o escopo funcional autorizado e seu fluxo não deve ser
alterado pela implementação. `BCS-DEC-001` permanece pendente para uma mudança
futura que toque esse fluxo, sem impedir a persistência em reboot comum, queda
de energia ou atualização de firmware.

## 12. Estado da autoria

A intenção, o comportamento observável, a solução proposta, as falhas e as
evidências esperadas foram registrados. Ao fim da autoria, a especificação
permanecia `Proposed`, a implementação `Not Started`, a entrega `Not Ready` e a
revisão de implementabilidade `Pending Review`.

O Autor da Especificação não executou análise de implementabilidade independente,
implementação, build nem testes funcionais.

## 13. Revisão de implementabilidade

**Resultado:** Implementável [`Implementable`]

A solução pode ser implementada sem decisão normativa, de produto ou
arquitetura ausente:

- `BinaryCommandCapability` já centraliza comandos e sincronização com o
  adapter, oferecendo o ponto comum requerido para restauração, confirmação e
  persistência de todas as classes derivadas;
- `ICommandHardwareAdapter` já informa aceitação do comando e permite leitura
  posterior do estado, sustentando aplicação seguida de read-back sem alteração
  de sua API pública;
- o builder define o `capability_name` definitivo antes da construção e do
  `setup()` do `CapabilityManager`, tornando disponível a identidade composta
  no momento exigido;
- `ServiceProvider` e `EspressifPlatformServiceRegistrar` constituem o
  precedente vigente para contrato no Core, implementação de plataforma e
  composição no bootstrap;
- o provedor de settings demonstra o uso vigente de blob versionado, namespace,
  chave e commit NVS, enquanto o novo contrato e namespace determinados nesta
  especificação preservam a separação entre os domínios;
- o limite fixo de oito slots no runtime coincide com o máximo normativo do
  snapshot;
- o environment `esp32s3_test` e o teste de settings existente oferecem
  precedente para testes PlatformIO/Unity contra NVS real.

Versão do formato, serialização compacta, verificação de integridade e detalhes
internos do cache são decisões de implementação delimitadas pelos requisitos
BCS-003, BCS-005 a BCS-008 e BCS-019 a BCS-020; não alteram comportamento
público nem exigem nova decisão arquitetural.

A análise não alterou código, testes ou configuração e preserva a implementação
como `Not Started`. Uma nova ordem do Arquiteto é necessária para iniciar a
implementação.
