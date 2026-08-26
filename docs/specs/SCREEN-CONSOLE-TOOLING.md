# Especificação — Console de tela como ferramenta de diagnóstico

**ID:** IOTSSC-SCREEN-CONSOLE

**Classe da fonte:** Normativa

**Versão:** 0.2

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Em andamento [`In Progress`]

**Estado da entrega:** Não aplicável [`Not Applicable`]

**Revisão de implementabilidade:** Implementável [`Implementable`]

**Relação normativa:** Nova [`New`], com aposentadoria [`Retires`] do componente
inerte `src/Infra/display/Display_ST7789_170_320.{h,cpp}` e preservação
[`Preserves`] de `IOTSSC-HW-EXAMPLES@1.1`

## 1. Objetivo e contexto

Incorporar ao core um **console de tela** como ferramenta de diagnóstico de
primeira classe, construído com o mesmo padrão arquitetural já vigente para
logging.

O logging do projeto tem quatro peças: o contrato `iotsmartsys::core::ILogger`,
a fachada estática `iotsmartsys::core::Log` com implementação nula por default
(`DefaultLogger`), a implementação de plataforma
`platform::arduino::ArduinoSerialLogger` e o registro no `ServiceProvider`
executado pelo registrar de plataforma. Os níveis são eliminados em tempo de
compilação por `IOTSMARTSYS_LOG_LEVEL`, com custo zero quando desligados.

Esta especificação replica esse padrão para uma saída visual: contrato em
`Contracts`, implementação em `Platform`, fachada estática com implementação
nula, ativação opt-in por build e custo zero quando desativada.

O console apresenta mensagens em linhas coloridas ancoradas na base da tela,
rolando as anteriores para cima como um terminal, de modo que a mensagem mais
recente esteja sempre visível na parte inferior.

## 2. Escopo

- contrato de console de tela (`IScreenConsole`) e sua paleta abstrata
  (`ScreenColor`);
- fachada estática de acesso (`Screen`) com implementação nula por default;
- implementação para painéis ST7789 sobre SPI (`ST7789ScreenConsole`);
- decorador de logger que espelha o diagnóstico no console
  (`ScreenMirrorLogger`);
- registro do console no grafo de serviços, em forma aditiva;
- ativação por flag de build, com custo nulo quando desativada;
- exemplo executável de uso do console e do `ScreenMirrorLogger`, selecionado
  pelo runner versionado e por environment próprio;
- aposentadoria do componente inerte `Display_ST7789_170_320`.

## 3. Fora de escopo

- interface gráfica, ícones, gráficos, telas de status ou navegação;
- entrada por toque ou qualquer interação do usuário com a tela;
- controladores de painel diferentes de ST7789 sobre SPI;
- persistência do histórico de linhas;
- transporte remoto do console por MQTT, HTTP ou portal web;
- alteração do contrato `ILogger`, dos níveis existentes ou do formato da saída
  serial;
- alteração do ciclo cooperativo do `SmartSysApp` ou do limite de oito
  capabilities.

## 4. Componentes e responsabilidades

| Componente | Local | Papel análogo no logging |
|---|---|---|
| `ScreenColor` | `src/Contracts/Display/` | `LogLevel` |
| `IScreenConsole` | `src/Contracts/Display/` | `ILogger` |
| `Screen` e `NoOpScreenConsole` | `src/Contracts/Display/` | `Log` e `DefaultLogger` |
| `ST7789ScreenConsole` | `src/Platform/Arduino/Display/` | `ArduinoSerialLogger` |
| `ScreenMirrorLogger` | `src/Platform/Arduino/Logging/` | — (novo, decorador) |
| Registro do console | `src/Contracts/Providers/ServiceProvider.*`, `src/Contracts/Providers/IServiceProvider.h`, `src/Core/Providers/ServiceManager.*` | registro do logger |
| Exemplo executável | `examples/executable/screen_console/`, `src/ExecutableExampleRunner.cpp`, `configs/executable_examples.ini` | padrão de `basic_light` |

A separação é normativa: `Contracts` define o console em termos abstratos e não
conhece biblioteca gráfica; `Platform` implementa a renderização; nenhum
componente de negócio conhece o display.

## 5. Requisitos

### 5.1 Paleta abstrata

- **SCR-001:** `ScreenColor` deve ser um enum com, no mínimo, os valores
  `Default`, `White`, `Red`, `Green`, `Blue`, `Yellow`, `Cyan` e `Magenta`.
- **SCR-002:** a tradução de `ScreenColor` para o formato nativo do painel é
  exclusiva da implementação de plataforma. Nenhum identificador de biblioteca
  gráfica pode atravessar a fronteira de `Contracts`.
- **SCR-003:** `Default` representa a cor de frente configurada na
  implementação, e não um valor fixo.

### 5.2 Contrato do console

- **SCR-004:** `IScreenConsole` deve declarar destrutor virtual e as operações
  `begin()`, `writef(ScreenColor color, const char *fmt, va_list args)`,
  `clear()` e `isReady() const`.
- **SCR-005:** `writef` é a primitiva única de escrita, no estilo `printf`, sem
  alocação dinâmica de memória, espelhando a assinatura de `ILogger::logf`.
- **SCR-006:** o contrato deve oferecer conveniências variádicas não virtuais
  construídas sobre `writef`: `write(ScreenColor, const char *fmt, ...)` e os
  atalhos por severidade `info(...)`, `warn(...)` e `error(...)`, com cor
  implícita `White`, `Yellow` e `Red` respectivamente.
- **SCR-007:** `begin()` inicializa o painel e limpa a tela; `clear()` limpa a
  tela e descarta o histórico de linhas; `isReady()` informa se o console está
  operante.
- **SCR-008:** `IScreenConsole` não declara `handle()` e não participa do ciclo
  cooperativo do runtime.

### 5.3 Fachada e implementação nula

- **SCR-009:** `Screen` deve oferecer `static void setConsole(IScreenConsole *)`
  e `static IScreenConsole &get()`, seguindo o precedente de `Log`.
- **SCR-010:** enquanto nenhum console for registrado, `Screen::get()` deve
  resolver para uma instância de `NoOpScreenConsole`, que descarta toda escrita
  sem efeito colateral e devolve `false` em `isReady()`.
- **SCR-011:** `Screen::get()` nunca pode devolver referência inválida, mesmo
  antes da inicialização do runtime.

### 5.4 Comportamento do console

- **SCR-012:** cada escrita ocupa a linha imediatamente acima do rodapé, e as
  linhas anteriores sobem uma posição.
- **SCR-013:** a mensagem escrita mais recentemente deve estar sempre encostada
  na base da área útil da tela.
- **SCR-014:** quando o histórico excede a capacidade visível, a linha mais
  antiga é descartada.
- **SCR-015:** texto mais largo que a área útil deve ocupar linhas consecutivas,
  preservando todo o conteúdo. Truncamento é proibido.
- **SCR-016:** cada linha preserva a cor com que foi escrita enquanto permanecer
  visível.
- **SCR-017:** o redesenho deve repintar apenas as faixas ocupadas pelas linhas,
  sem limpar a tela inteira a cada escrita.
- **SCR-018:** a capacidade visível deve ser derivada da altura da área útil, do
  tamanho da fonte e do espaçamento configurados.
- **SCR-019:** o histórico deve usar capacidade fixa de **24 linhas**, sem
  alocação dinâmica crescente. O número efetivamente utilizado é o menor entre a
  capacidade visível calculada e essa capacidade fixa.

### 5.5 Implementação ST7789

- **SCR-020:** `ST7789ScreenConsole` deve implementar `IScreenConsole` para
  painéis ST7789 sobre SPI e receber sua configuração por estrutura própria.
- **SCR-021:** a configuração deve oferecer pinos `CS`, `DC`, `RST`, `SCLK`,
  `MOSI` e backlight opcional; largura e altura nativas do painel; rotação;
  tamanho da fonte; margem horizontal; espaçamento entre linhas; cor de fundo e
  cor de frente default.
- **SCR-022:** quando o pino de backlight estiver configurado, `begin()` deve
  acioná-lo; quando não estiver, o backlight não é manipulado.
- **SCR-023:** escritas recebidas antes de `begin()` não podem produzir acesso
  ao painel; a implementação deve inicializar sob demanda ou descartar a
  escrita, sem comportamento indefinido.
- **SCR-024:** nenhum pino, dimensão, rotação ou cor pode ser fixado em código;
  todos são parâmetros de configuração.

### 5.6 Espelhamento do diagnóstico

- **SCR-025:** `ScreenMirrorLogger` deve implementar `ILogger` decorando um
  `ILogger` existente: toda chamada é repassada ao logger decorado e também
  escrita no console.
- **SCR-026:** o mapeamento de nível para cor é `Error` → `Red`, `Warn` →
  `Yellow`, `Info` → `White`, `Debug` e `Trace` → `Cyan`.
- **SCR-027:** a linha escrita no console deve preservar a mensagem formatada e
  a tag, quando houver; o formato exato de prefixo é escolha local da
  implementação.
- **SCR-028:** `setMinLevel` deve ser propagado ao logger decorado e respeitado
  pelo espelhamento.
- **SCR-029:** o decorador não é instalado por default. Sua instalação é ato
  explícito da aplicação ou do registrar de plataforma.

### 5.7 Ativação e custo

- **SCR-030:** a ativação é controlada pela flag de build
  `IOTSMARTSYS_SCREEN_CONSOLE_ENABLED`, com default **`0`**.
- **SCR-031:** com a flag em `0`, nenhuma implementação de painel é compilada
  nem linkada, e nenhuma dependência de biblioteca gráfica é exigida do
  consumidor.
- **SCR-032:** com a flag em `0`, o contrato e a fachada permanecem
  compiláveis, e todo uso resolve para a implementação nula.
- **SCR-033:** a ativação não pode alterar comportamento observável de qualquer
  funcionalidade existente além da própria saída visual.

### 5.8 Registro no grafo de serviços

- **SCR-034:** `ServiceProvider` deve receber `setScreenConsole(IScreenConsole *)`
  e `screenConsole() const`, e `ServiceManager` deve expor o acesso
  correspondente, em forma aditiva.
- **SCR-035:** o registro deve também alimentar a fachada `Screen`, como o
  registro do logger alimenta `Log`.
- **SCR-036:** a ausência de console registrado é estado válido e não pode
  impedir a inicialização do runtime nem produzir erro.
- **SCR-037:** nenhuma assinatura, default ou comportamento público existente
  pode ser alterado por esta especificação.

### 5.9 Aposentadoria do componente inerte

- **SCR-038:** `src/Infra/display/Display_ST7789_170_320.h` e
  `src/Infra/display/Display_ST7789_170_320.cpp` devem ser removidos.
- **SCR-039:** a remoção não pode alterar comportamento observável: o
  componente é guardado por `ST7789_170x320_ENABLED`, que nenhum environment
  define, e referencia identificadores inexistentes no projeto (`LCD_CS`,
  `LCD_DC`, `LCD_RST`, `LCD_WIDTH`, `LCD_HEIGHT`, `LCD_ROTATION`,
  `LCD_TEXTSIZE`), de modo que não compila se ativado.
- **SCR-040:** nenhum consumidor do projeto pode referenciar os símbolos
  removidos após a implementação.

### 5.10 Exemplo executável

- **SCR-041:** deve existir o exemplo executável
  `examples/executable/screen_console/example.hpp`, acompanhado de README com
  objetivo, configuração, pinout, comandos, montagem, validação manual,
  resultado esperado, limitações e riscos, conforme o padrão de
  `examples/executable/basic_light`.
- **SCR-042:** o exemplo deve ser selecionado pelo runner versionado
  `src/ExecutableExampleRunner.cpp` por meio da macro exclusiva
  `IOTSMARTSYS_EXAMPLE_SCREEN_CONSOLE`; `src/main.cpp` não integra o recorte e
  continua sendo a aplicação padrão.
- **SCR-043:** deve existir o environment estável
  `example_screen_console_esp32_dev`, herdando `env:esp32_dev`, excluindo
  `src/main.cpp`, habilitando `APP_EXAMPLE_RUNNER=1` e
  `IOTSMARTSYS_SCREEN_CONSOLE_ENABLED=1`, selecionando exclusivamente o exemplo
  e declarando as dependências Adafruit ST7789/GFX necessárias.
- **SCR-044:** o exemplo governa a placa Ideaspark ESP32 1.9 inch TFT LCD
  Display Board com controlador ST7789 e área nativa de 170 × 320 pixels. Sua
  configuração deve usar `CS=GPIO15`, `DC=GPIO2`, `RST=GPIO4`, `SCLK=GPIO18`,
  `MOSI=GPIO23` e backlight `GPIO32`; esses valores pertencem ao exemplo e não
  se tornam defaults da implementação de plataforma.
- **SCR-045:** o `setup()` do exemplo deve construir o console com configuração
  completa, registrá-lo no `ServiceProvider`, chamar `begin()` e instalar
  explicitamente um `ScreenMirrorLogger` decorando o logger corrente antes de
  emitir a mensagem diagnóstica demonstrativa.
- **SCR-046:** o `loop()` do exemplo deve preservar o processamento cooperativo
  por `SmartSysApp::handle()` e não deve escrever continuamente no display em
  caminho quente.
- **SCR-047:** o exemplo deve produzir no boot ao menos uma mensagem de
  diagnóstico pelo logger decorado, observável simultaneamente na saída serial
  e no console, e deve identificar o example id, a placa, as dimensões e a
  pinagem sem revelar segredo.
- **SCR-048:** o catálogo `examples/README.md` deve referenciar o novo exemplo e
  seu environment sem alterar os contratos dos exemplos existentes.

## 6. Fluxo esperado

1. O environment do consumidor define `IOTSMARTSYS_SCREEN_CONSOLE_ENABLED=1`.
2. A aplicação constrói `ST7789ScreenConsole` com a configuração do seu painel.
3. A aplicação registra o console no grafo de serviços, o que também alimenta a
   fachada `Screen`.
4. A aplicação chama `begin()` do console durante a inicialização.
5. Opcionalmente, a aplicação instala `ScreenMirrorLogger` decorando o logger
   corrente, e o diagnóstico do runtime passa a aparecer na tela.
6. Qualquer código pode escrever no console por `Screen::get()`, sem conhecer o
   painel.
7. Sem a flag, ou sem registro, toda escrita resolve para a implementação nula.
8. Para o uso de referência, o environment
   `example_screen_console_esp32_dev` seleciona o exemplo pelo runner, constrói
   o console Ideaspark com sua pinagem, instala `ScreenMirrorLogger` e preserva
   `app.handle()` no loop.

## 7. Falhas e condições de borda

- **Console não registrado:** toda escrita é descartada silenciosamente;
  `isReady()` devolve `false`; nenhuma mensagem de erro é emitida por isso.
- **Escrita antes de `begin()`:** conforme SCR-023, sem acesso ao painel e sem
  comportamento indefinido.
- **Mensagem maior que o buffer de formatação:** o conteúdo é truncado pela
  formatação, como já ocorre no logger serial; a quebra em linhas de SCR-015 se
  aplica ao texto formatado resultante.
- **Área útil menor que uma linha:** o console permanece operante e apresenta
  ao menos uma linha.
- **Escrita muito frequente:** a renderização é síncrona e bloqueante durante a
  transferência SPI. Esta é característica contratada, não defeito: o console é
  ferramenta de diagnóstico e não deve ser usado em caminho quente do ciclo
  cooperativo.
- **Exemplo fora do environment próprio:** a ausência das macros obrigatórias
  de seleção ou configuração deve causar erro de build compreensível; o exemplo
  não pode assumir silenciosamente outro painel ou pinout.

## 8. Critérios de aceite e validações

- **SCR-AC-001:** sem registrar console, `Screen::get()` devolve referência
  válida e escritas não produzem efeito nem falha. Meio: inspeção e execução
  instrumentada.
- **SCR-AC-002:** com `IOTSMARTSYS_SCREEN_CONSOLE_ENABLED=0`, o build canônico
  `pio run -e esp32_dev` alcança estado terminal com sucesso e o binário não
  linka implementação de painel. Meio: build canônico e inspeção do mapa de
  link.
- **SCR-AC-003:** com a flag em `1` e o console registrado, escritas sucessivas
  aparecem em linhas distintas, a mais recente sempre na base da área útil.
  Meio: hardware.
- **SCR-AC-004:** ultrapassada a capacidade visível, a linha mais antiga
  desaparece e nenhuma linha intermediária é perdida. Meio: hardware.
- **SCR-AC-005:** texto mais largo que a área útil ocupa linhas consecutivas com
  todo o conteúdo preservado. Meio: hardware.
- **SCR-AC-006:** cada linha mantém sua cor original ao rolar. Meio: hardware,
  escrevendo quatro linhas de cores distintas e forçando a rolagem.
- **SCR-AC-007:** com `ScreenMirrorLogger` instalado, uma chamada de log produz
  a mesma mensagem na saída serial e no console, com a cor do nível conforme
  SCR-026. Meio: hardware, comparando serial e tela.
- **SCR-AC-008:** sem `ScreenMirrorLogger` instalado, nenhuma mensagem de log
  aparece na tela. Meio: hardware.
- **SCR-AC-009:** nenhuma assinatura pública preexistente foi alterada. Meio:
  inspeção do delta contra `PUBLIC-API-COMPATIBILITY`.
- **SCR-AC-010:** após a remoção de SCR-038, nenhuma referência aos símbolos
  retirados permanece no repositório e o build canônico continua com sucesso.
  Meio: busca textual e build canônico.
- **SCR-AC-011:** `pio run -e example_screen_console_esp32_dev` alcança estado
  terminal com sucesso, vincula exatamente um par `setup()`/`loop()` e contém a
  implementação ST7789 habilitada. Meio: build e inspeção de símbolos do ELF.
- **SCR-AC-012:** o delta do exemplo demonstra construção, registro, `begin()`
  e instalação explícita de `ScreenMirrorLogger`, preserva `app.handle()` e
  referencia a pinagem de SCR-044 sem criar defaults de plataforma. Meio:
  inspeção do delta e build do environment.
- **SCR-AC-013:** após upload do environment próprio na placa Ideaspark, a
  mensagem diagnóstica de boot aparece na serial e na última linha visível do
  display, com a cor correspondente ao nível emitido. Meio: hardware,
  comparando serial e tela.
- **SCR-AC-014:** o README do exemplo e o catálogo documentam environment,
  controlador, dimensões, pinagem, montagem, comandos e resultado esperado sem
  credenciais. Meio: inspeção documental.

A validação física de SCR-AC-003 a SCR-AC-008 e SCR-AC-013 exige hardware e
permissão operacional explícita do Arquiteto. Enquanto não executada, permanece
`Not Executed` e não pode ser convertida em evidência aprovada.

## 9. Testes

**Nenhum artefato de teste integra o recorte desta versão.** Esta especificação
não exige criar, ampliar, reestruturar ou executar suítes automatizadas, e
nenhuma execução de teste é condição de aceite. O exemplo executável, seu
README, sua seleção no runner e seu environment integram explicitamente o
recorte, mas não constituem uma suíte de teste.

As evidências previstas são o build canônico (SCR-AC-002, SCR-AC-010), o build
do exemplo (SCR-AC-011, SCR-AC-012), a inspeção do delta (SCR-AC-001,
SCR-AC-009, SCR-AC-010, SCR-AC-012, SCR-AC-014) e a validação física sob ordem
explícita (SCR-AC-003 a SCR-AC-008, SCR-AC-013).

## 10. Conhecimento afetado

- `docs/rfc/KNOWLEDGE-MAP.md`: nova fonte normativa e cobertura de plataforma;
- `docs/rfc/EKM-CHANGELOG.md`: transação de autoria desta especificação.
- `examples/README.md`: catálogo do novo exemplo executável;
- `examples/executable/screen_console/README.md`: uso, montagem e validação da
  placa Ideaspark.

## 11. Relações, decisões e lacunas

### Relações normativas

- `docs/specs/PUBLIC-API-COMPATIBILITY.md` — preservada. As adições ao
  `ServiceProvider` e ao `ServiceManager` são aditivas e não alteram assinatura,
  default ou comportamento público existente.
- `docs/specs/CORE-RUNTIME-LIFECYCLE.md` — preservada. O console não é
  capability nem hardware adapter, não possui `handle()` e não participa do
  ciclo cooperativo.
- `docs/specs/RELEASE-AND-DISTRIBUTION.md` — sem impacto. As bibliotecas
  `adafruit/Adafruit ST7735 and ST7789 Library` e
  `adafruit/Adafruit GFX Library` já constam de `library.json`; nenhuma
  dependência publicada é acrescentada.
- `docs/specs/CURRENT-SENSING-CAPABILITY.md` — independente. Aquela
  especificação contrata diagnóstico por `ILogger` e é beneficiada pelo
  espelhamento de 5.6 sem ser emendada.
- `docs/specs/EXECUTABLE-HARDWARE-EXAMPLES.md` — preservada. O novo exemplo usa
  o runner e o contrato de catálogo vigentes, mantém `src/main.cpp` como
  aplicação padrão e acrescenta um environment executável sem redefinir a
  infraestrutura geral de exemplos.
- `src/Infra/display/Display_ST7789_170_320.{h,cpp}` — **aposentado**
  [`Retires`] conforme 5.9. Nenhuma fonte normativa vigente governa esse
  componente; ele é código inerte remanescente.

### Decisões do Arquiteto

- **SCR-DEC-001:** a primitiva de escrita é por cor abstrata, com atalhos por
  severidade construídos sobre ela.
- **SCR-DEC-002:** o console é registrado no `ServiceProvider` e no
  `ServiceManager`, com fachada estática `Screen` como conveniência.
- **SCR-DEC-003:** `ScreenMirrorLogger` integra o recorte, opcional e não
  instalado por default.
- **SCR-DEC-004:** `Display_ST7789_170_320.{h,cpp}` é retirado na implementação
  desta especificação.
- **SCR-DEC-005:** somente ST7789 sobre SPI é suportado, com pinos, dimensões e
  rotação configuráveis.
- **SCR-DEC-006:** texto longo é quebrado em linhas consecutivas, nunca
  truncado.
- **SCR-DEC-007:** o histórico tem capacidade fixa de 24 linhas.
- **SCR-DEC-008:** os nomes contratados são `IScreenConsole`, `ScreenColor`,
  `Screen`, `NoOpScreenConsole`, `ST7789ScreenConsole` e `ScreenMirrorLogger`.
- **SCR-DEC-009:** nenhuma criação ou execução de teste integra o recorte.
- **SCR-DEC-010:** a flag de ativação é `IOTSMARTSYS_SCREEN_CONSOLE_ENABLED`,
  com default `0`.
- **SCR-DEC-011:** o exemplo de uso é versionado no catálogo executável e
  referenciado por `src/ExecutableExampleRunner.cpp`; `src/main.cpp` não é
  alterado.
- **SCR-DEC-012:** o environment do exemplo é
  `example_screen_console_esp32_dev`, derivado de `env:esp32_dev`, e governa a
  placa Ideaspark e a pinagem registrada em SCR-044.
- **SCR-DEC-013:** o exemplo demonstra obrigatoriamente `ScreenMirrorLogger`,
  além da construção e do registro do console.

### Lacunas

Nenhuma lacuna bloqueante registrada na autoria da versão 0.2. A suficiência
técnica e a confrontação da implementação existente pertencem à nova Análise de
Implementabilidade.

## 12. Estado da especificação

Versão 0.2 registrada em `Draft`, com as decisões `SCR-DEC-001` a
`SCR-DEC-013` incorporadas.

A Análise de Implementabilidade da versão 0.1 foi executada e classificou
somente aquela versão como Pronta [`Ready`], sem bloqueador normativo,
arquitetural ou de evidência prévia. O relatório histórico é
`docs/reports/2026-08-26T012514Z-0.1-5cc6e5eb-implementability-analysis.md` e
registra cinco restrições materiais não bloqueantes para o handoff.

A implementação foi iniciada por ordem explícita do Arquiteto para a versão
0.1 e permanece Em andamento [`In Progress`]. Nenhuma validação foi executada
até o registro desta transição.

### Implementação da versão 0.1

O código do recorte foi implementado e o build canônico com
`IOTSMARTSYS_SCREEN_CONSOLE_ENABLED=0` alcançou `SUCCESS`. A inspeção do ELF
não encontrou símbolos da implementação ST7789 nem das bibliotecas gráficas, e
a busca textual confirmou a ausência de consumidores dos símbolos aposentados.

A execução está registrada no relatório
`docs/reports/2026-08-26T141607Z-0.1-de05f6a6-implementation-report.md`.
O estado permanece Em andamento [`In Progress`]: SCR-AC-003 a SCR-AC-008 e a
parcela instrumentada de SCR-AC-001 permanecem `Not Executed`, sem autorização
operacional para hardware. Nenhum teste integra ou foi executado no recorte.

### Autoria da versão 0.2

Por decisão do Arquiteto, a versão 0.2 acrescenta ao recorte o exemplo
executável `screen_console`, selecionado pelo runner versionado, com environment
derivado de `env:esp32_dev`, configuração da placa Ideaspark registrada em
SCR-044 e demonstração obrigatória de `ScreenMirrorLogger`.

A alteração normativa posterior ao `Ready` de 0.1 invalida sua aplicabilidade à
versão corrente. A implementação permanece Em andamento [`In Progress`], pois
os artefatos e evidências acrescentados por SCR-041 a SCR-048 e SCR-AC-011 a
SCR-AC-014 ainda não foram implementados ou executados.

### Análise de implementabilidade da versão 0.2

A versão 0.2 foi analisada integralmente e classificada como Pronta [`Ready`],
sem bloqueador normativo, arquitetural, de impacto ou de evidência prévia. O
relatório é
`docs/reports/2026-08-26T153635Z-0.2-e827ebd6-implementability-analysis.md`.
A revisão passa a Implementável [`Implementable`]; a implementação permanece
Em andamento [`In Progress`] e nenhuma implementação, build, teste, upload ou
validação física da versão 0.2 foi iniciada por esta análise.

### Implementação da versão 0.2

A implementação foi iniciada por ordem explícita do Arquiteto, sob a transação
`EKM-CHG-0040`, após a classificação `Ready` da versão corrente. O recorte
autorizado compreende os artefatos e as evidências de SCR-041 a SCR-048 e
SCR-AC-011 a SCR-AC-014; upload e validação física permanecem não autorizados.

O exemplo, o runner, o environment e a documentação foram implementados. Os
builds canônico e habilitado alcançaram `SUCCESS`; a inspeção do ELF confirmou
um único par `setup()`/`loop()`, presença de ST7789 no exemplo e ausência da
implementação gráfica na baseline. O relatório é
`docs/reports/2026-08-26T154955Z-0.2-d49f8216-implementation-report.md`.

O estado permanece Em andamento [`In Progress`]: SCR-AC-003 a SCR-AC-008,
SCR-AC-013 e a parcela instrumentada de SCR-AC-001 permanecem `Not Executed`
por exigirem hardware e ordem operacional explícita. Nenhum teste ou upload foi
executado.
