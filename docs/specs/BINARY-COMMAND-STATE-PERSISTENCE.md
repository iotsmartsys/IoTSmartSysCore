# Especificação — Persistência do estado de comandos binários

**ID:** IOTSSC-BINARY-COMMAND-STATE

**Classe da fonte:** Normativa

**Versão:** 0.2

**Estado normativo:** Proposta [`Proposed`]

**Estado da implementação:** Em andamento [`In Progress`]

**Estado da entrega:** Não pronta [`Not Ready`]

**Revisão de implementabilidade:** Implementável [`Implementable`]

**Relação normativa:** Corrige [`Corrects`]
`IOTSSC-BINARY-COMMAND-STATE@0.1`

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
- a execução experimental da versão 0.1 demonstrou que testes estruturalmente
  presentes ainda podiam aceitar valve sem interpreter, LED fora do protocolo
  comum, snapshot sem integridade de conteúdo, falhas NVS indistintas de
  ausência, identidades truncadas e compilação com zero casos executados.

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

A identidade deve ser preservada integralmente para todo nome e tipo aceito
pela configuração pública vigente. O formato interno não pode introduzir um
limite menor, truncamento, colisão por prefixo ou gravação parcial.

O formato deve possuir verificação de integridade que cubra o cabeçalho e todos
os registros ativos. Tamanho e versão compatíveis, isoladamente, não comprovam
integridade.

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
  → aplicação pelo mesmo caminho de interpretação usado por um comando normal
  → leitura de confirmação pelo mesmo caminho de interpretação do estado
  → atualização/publicação do estado confirmado
→ registro ausente ou inválido
  → preservação do fluxo e do default vigentes
```

O estado persistido não pode ser anunciado como aplicado antes de o adapter
aceitar o comando e sua leitura confirmar o valor correspondente.

Para `ValveCapability`, restaurar `on` significa solicitar `open` à capability,
o interpreter deve entregar `on` ao adapter, o adapter deve reportar `on` e o
interpreter deve converter a leitura confirmada em `open`. O fluxo equivalente
de `off` usa `closed` na capability e `off` no adapter. Enviar diretamente
`open` ou `closed` ao adapter não satisfaz esta especificação.

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
transição e a persistência. Um override de classe derivada, inclusive
`LEDCapability::handle()`, não pode contornar aplicação, read-back, atualização
do estado lógico, publicação e persistência. Uma falha de gravação deve ser
registrada em log, mas não pode reverter um comando já aplicado nem impedir o
processamento cooperativo.

## 6. Requisitos

- **BCS-001:** toda capability derivada de `BinaryCommandCapability` deve
  participar automaticamente da restauração e persistência, sem configuração
  opt-in por tipo concreto.
- **BCS-002:** a identidade persistente deve combinar o
  `capability_name` definitivo e o `type`, sem truncamento silencioso nem
  colisão por prefixo.
- **BCS-003:** o storage deve representar somente os estados semânticos
  binários `off` e `on`.
- **BCS-004:** cada estado semântico restaurado deve ser convertido para o
  `_offValue` ou `_onValue` do tipo concreto e percorrer o interpreter
  configurado antes de alcançar o adapter.
- **BCS-005:** o provedor deve manter os estados em namespace NVS exclusivo e
  não pode alterar o blob de settings.
- **BCS-006:** o formato persistido deve ser versionado, limitado a oito
  registros ativos e capaz de rejeitar conteúdo incompleto, incompatível ou
  corrompido por meio de validação de integridade do cabeçalho e dos registros
  ativos.
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
  persistência, mesmo quando a classe derivada sobrescrever `handle()` ou outro
  ponto do ciclo.
- **BCS-017:** falha ao inicializar, abrir, ler, gravar ou executar commit na
  NVS não pode bloquear o loop, abortar ou reiniciar o dispositivo nem desfazer
  o estado de hardware já confirmado.
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

## 8. Critérios de aceite e validações

Um critério comportamental só é aprovado quando o teste indicado executa ao
menos um caso, termina sem falha nem erro e todas as asserções descritas são
satisfeitas. Compilação, descoberta sem execução, zero casos, erro de upload,
erro de infraestrutura ou resultado desconhecido classificam o critério como
**não verificado**, nunca como aprovado.

### 8.1 Matriz assertável

| Critério | Requisito | Cenário e ação | Resultado observável | Evidência terminal |
|---|---|---|---|---|
| BCS-AC-001 | BCS-001 | Para cada tipo concreto `SwitchCapability`, `SwitchPlugCapability`, `LightCapability`, `LEDCapability` e `ValveCapability`, partir de snapshot válido com estado oposto ao default, executar `setup()` e depois uma transição confirmada. | Cada um dos cinco tipos restaura o estado correto e persiste exatamente uma vez a transição posterior, sem opt-in específico do tipo. Se qualquer tipo não restaurar ou não persistir, o critério reprova. | Suíte parametrizada ou cinco casos nomeados, todos executados e aprovados, com adapter e storage observáveis. |
| BCS-AC-002 | BCS-002, BCS-019 | Salvar duas capabilities cujas identidades diferem apenas pelo nome, duas que compartilham o nome mas diferem pelo tipo e duas identidades válidas com prefixo longo comum e sufixos distintos; reinicializar o provedor e consultar cada identidade completa. | Cada consulta retorna somente o próprio estado. Nenhuma identidade é truncada, rejeitada por limite interno menor que o público, colide, consome o registro de outra ou altera outra capability. | Teste de round-trip e isolamento que compara nome e tipo integrais antes e depois do reboot simulado. |
| BCS-AC-003 | BCS-003, BCS-015 | Partir de `off`, executar `toggle`, confirmar `on`, persistir e decodificar o snapshot; repetir partindo de `on`. | O snapshot contém somente o estado semântico final `on` ou `off`; nunca contém `toggle`, `open`, `closed` ou outro comando transitório/concreto. | Teste executado que observa a entrada do contrato de storage e decodifica o blob persistido nos dois sentidos. |
| BCS-AC-004 | BCS-004, BCS-009, BCS-010 | Configurar uma `ValveCapability` com o `ValveHardwareCommandInterpreter` e um double fiel ao `OutputHardwareAdapter`, que aceita somente `on`, `off` e `toggle`. Restaurar semanticamente `on` e depois `off`. | Para `on`, a capability solicita `open`, o interpreter envia `on` ao adapter, o read-back `on` é interpretado como `open` e somente então o estado lógico/publicado se torna `open`. Para `off`, ocorre `closed → off → off → closed`. Envio direto de `open` ou `closed` ao adapter reprova. | Dois casos executados com sequência de chamadas e valores observáveis; o double rejeita o vocabulário da valve quando recebido diretamente. |
| BCS-AC-005 | BCS-004, BCS-009, BCS-010 | Para switch, switch plug, light e LED, restaurar `on` e `off` após o `setup()` do adapter, registrando ordem, aceitação, read-back e eventos. | A ordem é `adapter setup → comando interpretado/aplicado → read-back → atualização/publicação`. Estado rejeitado ou não confirmado não é assumido nem publicado. | Casos executados para os quatro tipos, com spy de ordem e asserções de ausência de atualização antes do read-back. |
| BCS-AC-006 | BCS-005 | Gravar e restaurar um snapshot enquanto existe um blob sentinela no namespace/chave de settings. | Somente o namespace e a chave exclusivos da funcionalidade são abertos para escrita; o blob sentinela permanece byte a byte idêntico. | Teste contra NVS real ou emulação fiel que enumera operações por namespace/chave e compara o blob de settings antes e depois. |
| BCS-AC-007 | BCS-006, BCS-012 | Criar snapshots válidos com zero e oito registros; tentar salvar o nono; fornecer separadamente blob truncado, versão desconhecida, quantidade maior que oito, estado fora de `off`/`on` e registro estruturalmente inválido. | Zero e oito são aceitos; o nono é recusado sem alterar o último snapshot válido. Cada blob inválido é rejeitado e nenhum de seus valores é aplicado. | Casos executados por condição, com inspeção do resultado do provedor, cache e ausência de aplicação nas capabilities. |
| BCS-AC-008 | BCS-006, BCS-012 | Partir de snapshot válido e alterar separadamente um byte do cabeçalho e um byte de cada região de registro ativo, preservando tamanho e versão externos. | Toda alteração coberta é detectada pela verificação de integridade; o snapshot é rejeitado integralmente e nenhum valor corrompido é aplicado. Aceitar conteúdo apenas porque tamanho e versão coincidem reprova. | Teste de corrupção por mutação de bytes, executado sobre formato real, que comprova rejeição e zero chamadas de aplicação. |
| BCS-AC-009 | BCS-007 | Instrumentar as operações NVS, iniciar o serviço uma vez com snapshot presente e consultar todas as oito identidades repetidamente. | Durante o boot ocorre no máximo uma chamada que copia dados do blob para memória. Todas as consultas posteriores retornam do cache e não acrescentam leitura de dados; consulta apenas de tamanho é contabilizada separadamente. | Contadores executados por operação NVS antes e depois do boot e das consultas, com asserção `data_reads <= 1` e delta posterior igual a zero. |
| BCS-AC-010 | BCS-008 | Após o boot, chamar `handle()` repetidamente nos cinco tipos concretos, incluindo LED parado e piscando. | O contador de leituras NVS permanece inalterado em todas as chamadas. Gravações decorrentes de transição confirmada do blink não contam como leitura e continuam permitidas. | Teste executado com instrumentação NVS e asserção de delta zero para chamadas de leitura. |
| BCS-AC-011 | BCS-011 | Simular primeiro boot, namespace ausente e identidade ausente em snapshot válido; executar `setup()` de cada tipo concreto. | Cada capability mantém o default vigente obtido do adapter, conclui a inicialização e não cria gravação ou commit apenas por estar no default. | Casos executados com estado lógico, eventos, chamadas de storage e conclusão do setup observáveis. |
| BCS-AC-012 | BCS-009, BCS-010, BCS-012 | Restaurar registro válido e provocar separadamente rejeição do comando e read-back diferente do valor solicitado. | Em ambos os casos o valor solicitado não vira estado lógico, não é publicado e não é persistido como confirmado; a inicialização continua com o estado efetivamente lido ou o default vigente. | Dois casos executados com adapter programável e spies de estado, publicação e save. |
| BCS-AC-013 | BCS-013, BCS-014 | Para cada sentido `off → on` e `on → off`, confirmar a mudança e depois atribuir novamente o mesmo valor. | Cada mudança efetiva atualiza o cache, grava um snapshot e conclui um commit exatamente uma vez. A repetição produz zero gravações e zero commits adicionais. | Teste executado com contadores separados de atualização de cache, write e commit. |
| BCS-AC-014 | BCS-016 | Produzir transições separadamente por comando remoto, `turnOn`, `turnOff`, `power`, `toggle` e mudança externa observada no adapter. | Cada origem que resulta em mudança confirmada percorre read-back, atualização/publicação e exatamente um save/commit; origem rejeitada ou sem mudança produz zero save/commit. | Casos executados e nomeados por origem, com adapter, eventos e storage observáveis. |
| BCS-AC-015 | BCS-001, BCS-013, BCS-016 | Em `LEDCapability` fora de blink, executar comando aceito e depois `handle()`; em blink, avançar o relógio por duas alternâncias completas. | Fora de blink, a transição é confirmada, publicada e persistida uma vez. Em blink, cada alternância é aplicada, confirmada por read-back, publicada e persistida uma vez. O override de `handle()` não pode omitir sincronização nem duplicar commit. | Teste com LED concreto, relógio controlado, adapter e storage observáveis; contadores após cada alternância devem crescer exatamente em um. |
| BCS-AC-016 | BCS-017, BCS-018, BCS-021 | Injetar separadamente falha de inicialização NVS, open, leitura do blob, write e commit; após cada falha, executar o próximo ciclo cooperativo. | Nenhuma falha aborta, reinicia, bloqueia ou escapa do ciclo esperado. Falhas de init/open/read preservam o fluxo/default; falhas de write/commit preservam o estado já confirmado de hardware, lógico e publicado. Ausência e falha possuem resultados/logs distintos, e o ciclo posterior termina. | Um caso executado por operação, usando seam que devolve os mesmos códigos e preserva a semântica de commit da NVS; spy comprova retorno ao chamador, ciclo posterior e classe diagnóstica. |
| BCS-AC-017 | BCS-017, BCS-018 | Persistir `off`, confirmar transição de hardware para `on` e provocar falha de write; repetir provocando falha de commit; reinicializar após cada cenário. | Antes do reboot, hardware, estado lógico e publicação permanecem `on` apesar da falha. Após o reboot, restaura-se `off`, que foi o último commit bem-sucedido. Não há rollback imediato nem promoção do snapshot falho a commit válido. | Casos executados com emulação transacional fiel ou NVS real com injeção equivalente, observando estado antes e depois do reboot. |
| BCS-AC-018 | BCS-019 | Persistir estados opostos para duas capabilities e restaurá-las no mesmo boot, variando a ordem de consulta e setup. | Cada capability recebe somente o próprio estado; restaurar ou alterar uma não muda cache, hardware, estado lógico ou publicação da outra. | Teste executado com duas instâncias, dois adapters, dois sinks e inspeção independente dos registros. |
| BCS-AC-019 | BCS-020 | Persistir uma identidade, reinicializar primeiro com nome diferente e depois com tipo diferente. | Nenhuma das identidades alteradas reutiliza o registro anterior; ambas seguem o fluxo de ausência e preservam o default. A identidade original ainda recupera seu próprio registro. | Teste executado cobrindo mudança de nome e de tipo separadamente. |
| BCS-AC-020 | BCS-021 | Executar ausência, snapshot inválido, falha de aplicação e falha de init/open/read/write/commit. | O diagnóstico identifica a classe correta sem imprimir valor do blob, settings, credenciais ou conteúdo privado. Falha de storage nunca é registrada como ausência. | Testes com logger capturado e asserções positivas da classe e negativas para sentinelas privadas. |
| BCS-AC-021 | BCS-022 | Comparar as APIs públicas e defaults com a base anterior à funcionalidade; construir oito capabilities antes de `SmartSysApp::setup()` e executar ciclos cooperativos. | Assinaturas e defaults públicos permanecem compatíveis, oito capabilities continuam aceitas, uma nona continua sujeita ao limite vigente e `handle()` retorna cooperativamente sem espera por storage. | Inspeção de diff das APIs públicas mais build e teste executado do limite/configuração/ciclo. |
| BCS-AC-022 | BCS-023 | Compilar o environment Arduino ESP32 suportado e inspecionar as dependências do contrato Core e do código preparatório ESP-IDF afetado. | `pio run -e esp32_dev` termina com sucesso; o Core não inclui APIs NVS/Arduino; o provedor Espressif permanece atrás da fronteira de plataforma e nenhuma fonte preparatória ESP-IDF é removida ou tornada dependente do runtime Arduino. | Build terminal aprovado e inspeção estática registrada sobre os arquivos alterados. |

### 8.2 Fidelidade obrigatória dos doubles

- O double do adapter de saída aceita somente `on`, `off` e `toggle`, como o
  `OutputHardwareAdapter`; ele deve rejeitar `open` e `closed`.
- Testes de valve usam o `ValveHardwareCommandInterpreter` real ou um double
  que comprove explicitamente as conversões `open ↔ on` e `closed ↔ off`.
- O seam NVS permite falhar individualmente init, open, read, write e commit e
  preserva a propriedade material de que somente commit bem-sucedido sobrevive
  ao reboot.
- Spies de storage mantêm contadores distintos para consulta de tamanho,
  leitura que copia dados, atualização de cache, write e commit.
- Relógio usado no teste de blink é controlável e avança cada alternância sem
  espera real.

Um double que aceite um vocabulário rejeitado pela integração real, trate write
como commit ou não permita observar uma operação exigida torna o critério
correspondente não verificável.

### 8.3 Checklist de autoria

- [x] BCS-001 a BCS-023 estão relacionados a pelo menos um critério.
- [x] Cada critério identifica cenário, ação, resultado observável e evidência
  terminal.
- [x] Os resultados podem ser convertidos em asserções sem decisão funcional ou
  arquitetural adicional.
- [x] Os critérios reprovam os desvios plausíveis observados na execução
  anterior: valve sem interpreter, override de LED fora do protocolo, corrupção
  validada apenas por tamanho/versão, falhas NVS confundidas com ausência,
  identidade truncada e testes compilados com zero casos executados.
- [x] Validações automatizáveis estão separadas da validação física posterior.
- [x] `BCS-DEC-001` permanece explicitamente fora do recorte e não bloqueante.

### 8.4 Gate da implementação

Para promover a implementação a `Implemented`, todos os critérios
BCS-AC-001 a BCS-AC-022 devem estar aprovados ou possuir evidência automatizada
equivalente que demonstre exatamente o mesmo oráculo. São obrigatórios:

- `pio run -e esp32_dev` com estado terminal `SUCCESS`;
- `pio test -e esp32s3_test` com estado terminal aprovado, quantidade total de
  casos executados maior que zero e os casos desta especificação efetivamente
  executados;
- `git diff --check` sem erros;
- matriz BCS-AC preenchida com resultado terminal e referência à evidência de
  cada critério.

Critério falho, não executado ou não verificável mantém a implementação
`In Progress`. Compilar testes com `--without-testing`, obter zero casos ou
falhar antes da execução não satisfaz o gate.

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
- `EKM-CHG-0013`.

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

A versão 0.2 corrige os oráculos insuficientes da versão 0.1 sem alterar a
intenção funcional. Todos os requisitos obrigatórios possuem rastreabilidade
para cenário, ação, resultado observável e evidência terminal. Os critérios
explicitam as propriedades que reprovam as incompatibilidades plausíveis
observadas no experimento anterior e separam o gate automatizável da validação
física posterior.

Ao fim desta autoria, a especificação está `Proposed`, a implementação desta
versão está `Not Started`, a entrega está `Not Ready` e a revisão de
implementabilidade está `Pending Review`.

O Autor da Especificação não executou análise de implementabilidade independente,
implementação, build nem testes funcionais.

## 13. Revisão de implementabilidade

**Resultado:** Implementável [`Implementable`]

**Resumo da análise:** revisão independente da versão 0.2, sem reutilizar a
conclusão de implementabilidade da versão 0.1. Todo o recorte (BCS-001 a
BCS-023, BCS-AC-001 a BCS-AC-022) pode ser executado sem decisão normativa, de
produto ou arquitetura ausente, usando exclusivamente os padrões já vigentes no
repositório.

Fontes técnicas confrontadas nesta análise:

- `src/Core/Capabilities/CapabilityHelpers.h` (`BinaryCommandCapability`) já
  concentra `toggle`/`turnOn`/`turnOff`/`power`, restauração no `setup()` e
  sincronização com persistência via `syncFromHardware()`/
  `persistIfTransition()`, confirmando o ponto comum exigido por BCS-001 e
  BCS-016;
- `ICommandCapability::applyCommand` já percorre `command_interpreter` quando
  presente, e `command_hardware_adapter.applyCommand` já retorna aceitação;
  isso sustenta aplicação seguida de read-back interpretados (BCS-004,
  BCS-009, BCS-010) sem alterar API pública. Para `ValveCapability`, o caminho
  correto (5.3/BCS-AC-004) exige que a restauração reutilize esse mesmo caminho
  interpretado — hoje `restoreFromStorage()` chama
  `command_hardware_adapter.applyCommand()`/`getStateValue()` diretamente,
  contornando `command_interpreter`. Isso é um desvio de implementação
  corrigível dentro da classe existente (membros protegidos já acessíveis),
  não uma lacuna de decisão;
- `LEDCapability::handle()` hoje substitui integralmente
  `BinaryCommandCapability::handle()` sem chamar `syncFromHardware()`, o que
  reproduz exatamente o desvio "LED fora do protocolo comum" registrado nos
  fatos observados (seção 2). `syncFromHardware()` é protegido e herdável, o
  que torna a correção (BCS-016, BCS-AC-015) uma alteração local, sem novo
  contrato;
- `EspNvsBinaryCapabilityStateProvider` (namespace `iotbcs`, chave `state`)
  hoje valida apenas tamanho e versão do blob, e `copyField()` trunca
  silenciosamente nomes que excedam `NAME_LEN`/`TYPE_LEN`. Ambos reproduzem os
  desvios "snapshot sem integridade de conteúdo" e "identidades truncadas" dos
  fatos observados. BCS-006/BCS-012/BCS-AC-008 exigem verificação de
  integridade sobre cabeçalho e registros ativos (ex.: checksum), e
  BCS-002/BCS-AC-002 exigem rejeitar em vez de truncar; ambos são alcançáveis
  no mesmo formato de blob fixo já em uso, sem novo contrato de storage;
- `type` das cinco capabilities do escopo (`Switch`, `Switch Plug`,
  `Light Actuator`, `LED Actuator`, `Valve Actuator`) tem no máximo 14
  caracteres, dentro de `TYPE_LEN=24`; `capability_name` não possui limite
  público documentado (é `device_id + "_" + type` ou string arbitrária vinda
  da configuração), portanto o requisito de "não introduzir limite menor que o
  público" (5.2) é satisfeito por um buffer generoso combinado com rejeição
  explícita (não silenciosa) de identidades que excedam a capacidade interna —
  decisão de formato interno, não decisão normativa ausente;
- `ServiceProvider`/`EspressifPlatformServiceRegistrar` já implementam
  exatamente o padrão de composição exigido (contrato no Core, implementação
  em `Platform/Espressif`, registro via setter singleton), e
  `registerPlatformServices()` já chama `loadSnapshot()` antes da construção
  das capabilities, confirmando a ordem de boot da seção 5.3;
- `common::StateResult` já distingue `NotFound`, `StorageCorrupt`,
  `StorageVersionMismatch`, `StorageReadFail`, `StorageWriteFail`,
  `InvalidArg` e `Overflow`, suficiente para os diagnósticos distintos exigidos
  por BCS-021 sem novo tipo;
- `pio run -e esp32_dev` compila com sucesso no estado atual do branch
  (`SUCCESS`, Flash 89.8% / RAM 23.8%), confirmando que a fronteira de
  plataforma já integra sem quebrar o build; a margem de Flash é estreita e
  deve ser observada pelo Implementador ao adicionar verificação de
  integridade e diagnósticos mais granulares, mas isso é risco de engenharia,
  não decisão ausente;
- `pio test -e esp32s3_test` exige upload para hardware físico
  (`upload_port` fixo em `configs/esp32s3-test.ini`); sem um ESP32-S3 conectado
  nesta sessão, a suíte inteira retorna `ERRORED` na etapa de upload — inclusive
  os testes já existentes de `test_binary_command_capability_state` e
  `test_binary_capability_state_storage`, que chegam a compilar com sucesso
  antes de falhar apenas no upload. Essa é uma pré-condição ambiental
  preexistente do projeto (mesma modalidade de teste usada pelas specs
  anteriores), não uma lacuna desta especificação; o Implementador precisa de
  hardware conectado para produzir a evidência terminal exigida pelo gate 8.4;
- os artefatos da versão 0.1 ainda presentes na branch (`IBinaryCapabilityStateProvider`,
  `EspNvsBinaryCapabilityStateProvider`, testes e mocks) não constituem
  implementação aprovada da versão 0.2 e reproduzem, ponto a ponto, os desvios
  que os fatos observados da seção 2 registraram; permanecem como material de
  partida sujeito a nova implementação controlada, não como evidência aceita.

**Decisões ausentes:** `BCS-DEC-001`, permanece pendente e não bloqueante
(fora do escopo funcional autorizado; factory reset não deve ser alterado por
esta implementação). Nenhuma outra decisão normativa, de produto ou
arquitetura está ausente.

**Evidências consultadas:** leitura de
`src/Core/Capabilities/CapabilityHelpers.h`, `src/Contracts/Capabilities/ValveCapability.*`,
`src/Contracts/Capabilities/LEDCapability.*`,
`src/Platform/Arduino/Interpreters/ValveHardwareCommandInterpreter.*`,
`src/Platform/Arduino/Adapters/OutputHardwareAdapter.h`,
`src/Contracts/Providers/IBinaryCapabilityStateProvider.h`,
`src/Contracts/Providers/ServiceProvider.*`,
`src/Platform/Espressif/Capabilities/Providers/EspNvsBinaryCapabilityStateProvider.*`,
`src/Platform/Espressif/Providers/EspressifPlatformServiceRegistrar.*`,
`src/Contracts/Common/StateResult.h`, `src/App/Builders/Builders/CapabilitiesBuilder.cpp`
e `src/App/Builders/Configs/HardwareConfig.h`; execução de `pio run -e esp32_dev`
(`SUCCESS`) e `pio test -e esp32s3_test` (`ERRORED` na etapa de upload, sem
hardware conectado) nesta sessão, apenas para verificação de fatos — nenhum
código, teste, configuração ou build foi alterado por esta análise.

A análise não alterou código, testes ou configuração e preserva a
implementação como `Not Started` e a entrega como `Not Ready`. Uma nova ordem
do Arquiteto é necessária para iniciar a implementação.

## 14. Implementação (Engenheiro Implementador)

**Ordem recebida:** autorização explícita do Arquiteto para implementar todo
o recorte BCS-001 a BCS-023, corrigindo os artefatos experimentais 0.1 ainda
presentes na branch conforme os desvios apontados na seção 13.

**Estado desta transação:** Em andamento [`In Progress`]. Registrada em
`EKM-CHG-0015`.

### 14.1 Correções aplicadas

- `src/Core/Capabilities/CapabilityHelpers.h`: `restoreFromStorage()` agora
  usa `command_interpreter->interpretCommand()`/`interpretState()` quando
  presente, em vez de chamar `command_hardware_adapter` diretamente — corrige
  o desvio "valve sem interpreter" (BCS-004/BCS-009/BCS-010, 5.3).
- `src/Core/Capabilities/LEDCapability.cpp`: `handle()` chama
  `syncFromHardware()` incondicionalmente — corrige o desvio "LED fora do
  protocolo comum" (BCS-016).
- `src/Platform/Espressif/Capabilities/Providers/EspNvsBinaryCapabilityStateProvider.*`:
  checksum (cabeçalho + todos os registros) validado em `loadSnapshot()`;
  `copyField()`/`save()` rejeitam identidade que excede o buffer interno em
  vez de truncar — corrige "snapshot sem integridade de conteúdo" e
  "identidades truncadas" (BCS-002/BCS-006/BCS-012).
- Doubles e testes alinhados à fidelidade exigida em 8.2 (ver `EKM-CHG-0015`
  para a lista completa de arquivos de teste alterados).

### 14.2 Matriz BCS-AC — resultado desta transação

Cobertura automatizada aplicada nesta transação; estado terminal por hardware
real não obtido (ver 14.3). Critérios marcados **compilado, não executado**
têm caso de teste presente e compilando, mas sem evidência terminal de
execução em hardware — permanecem não verificados pelo gate 8.4 até essa
evidência existir.

| Critério | Cobertura nesta transação | Estado |
|---|---|---|
| BCS-AC-001 | Parcial: switch coberto por `test_transition_persists_once_and_repeats_do_not`; valve por `test_valve_vocabulary_conversion_on_restore_*`; LED por `test_led_*`. Switch Plug e Light não têm caso dedicado nesta transação. | Compilado, não executado (parcial) |
| BCS-AC-002 | `test_identity_isolation_between_capabilities` (capability) e `test_identity_isolation` (storage); prefixo longo coberto por `test_oversized_identity_is_rejected_not_truncated`. | Compilado, não executado |
| BCS-AC-003 | `test_toggle_persists_only_final_confirmed_value`. | Compilado, não executado |
| BCS-AC-004 | `test_valve_vocabulary_conversion_on_restore_open`/`_closed`, com `ValveHardwareCommandInterpreter` real e double que rejeita `open`/`closed`. | Compilado, não executado |
| BCS-AC-005 | Coberto indiretamente pelos testes de restore/transição existentes; sem spy de ordem dedicado nesta transação. | Compilado, não executado (parcial) |
| BCS-AC-006 | Não coberto por caso dedicado nesta transação (namespace exclusivo é garantido pelo design, não observado por teste de sentinela). | Não verificado |
| BCS-AC-007 | Não coberto por caso dedicado nesta transação (contador de leituras NVS não instrumentado). | Não verificado |
| BCS-AC-008 | `test_header_byte_corruption_is_rejected`, `test_active_record_byte_corruption_is_rejected`. | Compilado, não executado |
| BCS-AC-009 | Não coberto por caso dedicado nesta transação. | Não verificado |
| BCS-AC-010 | Não coberto por caso dedicado nesta transação. | Não verificado |
| BCS-AC-011 | `test_setup_without_record_uses_default`, `test_first_boot_is_absent`. | Compilado, não executado |
| BCS-AC-012 | `test_setup_restore_rejected_by_adapter_keeps_default`, `test_setup_restore_unconfirmed_keeps_default`. | Compilado, não executado |
| BCS-AC-013 | `test_transition_persists_once_and_repeats_do_not`. | Compilado, não executado |
| BCS-AC-014 | Parcial: `turnOn`/`toggle`/adapter-observed cobertos; comando remoto e `power` sem caso dedicado nesta transação. | Compilado, não executado (parcial) |
| BCS-AC-015 | `test_led_handle_outside_blink_persists_confirmed_command`, `test_led_blink_persists_each_alternation`. | Compilado, não executado |
| BCS-AC-016 | Não coberto por caso dedicado nesta transação (seam de falha por operação NVS não construído). | Não verificado |
| BCS-AC-017 | `test_persist_failure_keeps_hardware_state_and_last_successful_record` cobre falha de save() a nível de capability; não cobre reboot real com falha de write/commit isolada a nível do provedor NVS. | Compilado, não executado (parcial) |
| BCS-AC-018 | `test_identity_isolation_between_capabilities`. | Compilado, não executado |
| BCS-AC-019 | Não coberto por caso dedicado nesta transação (mudança de identidade entre boots simulados). | Não verificado |
| BCS-AC-020 | Não coberto por caso dedicado nesta transação (captura de log). | Não verificado |
| BCS-AC-021 | Não coberto por caso dedicado nesta transação. | Não verificado |
| BCS-AC-022 | `pio run -e esp32_dev` `SUCCESS`; inspeção estática confirma que o Core não inclui APIs NVS/Arduino e que o provedor Espressif permanece atrás de `#ifdef ESP32`. | Aprovado (build) / restante não verificado |

### 14.3 Gate 8.4 — não satisfeito nesta transação

`pio test -e esp32s3_test` exige um ESP32-S3 físico conectado
(`upload_port` fixo em `configs/esp32s3-test.ini`); nesta sessão a suíte
compilou e terminou `ERRORED` apenas na etapa de upload, para as duas suítes
desta funcionalidade — mesma pré-condição ambiental registrada em
`EKM-CHG-0014`. Sem essa evidência terminal, o gate de `Implemented` não é
satisfeito e a implementação permanece `In Progress`, mesmo com `pio run`
aprovado e `git diff --check` sem erros. Os critérios BCS-AC-006, BCS-AC-007,
BCS-AC-009, BCS-AC-010, BCS-AC-016, BCS-AC-019, BCS-AC-020 e BCS-AC-021, além
das lacunas parciais indicadas em 14.2, também não possuem caso dedicado
nesta transação e permanecem não verificados independentemente do hardware.

### 14.4 Lacunas conhecidas e próximos passos

- Construir seam de falha por operação NVS (init/open/read/write/commit) no
  provedor Espressif para viabilizar BCS-AC-016/BCS-AC-017 completos.
- Instrumentar contadores de leitura NVS (tamanho vs. dados) para
  BCS-AC-007/BCS-AC-009/BCS-AC-010.
- Adicionar casos dedicados para Switch Plug e Light em BCS-AC-001/BCS-AC-005,
  para comando remoto e `power` em BCS-AC-014, para mudança de identidade
  entre boots em BCS-AC-019, para captura de log em BCS-AC-020/BCS-AC-021 e
  para sentinela de namespace de settings em BCS-AC-006.
- Executar `pio test -e esp32s3_test` com ESP32-S3 físico conectado para
  produzir a evidência terminal exigida pelo gate 8.4.
- `BCS-DEC-001` permanece pendente e não bloqueante; nenhuma ação desta
  transação depende dela.
