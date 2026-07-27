# Especificação — Exemplos executáveis e validação em hardware

**ID:** IOTSSC-HW-EXAMPLES

**Tipo:** Normativo

**Estado normativo:** Active

**Estado da implementação:** Implemented

**Estado da entrega:** Not Ready

**Technical readiness:** Implementable

**Versão:** 1.1

**Última atualização:** 23/07/2026

## 1. Objetivo

Disponibilizar exemplos executáveis das capabilities e dos principais fluxos públicos do IoTSmartSysCore, selecionáveis por ambientes do PlatformIO e diretamente graváveis em placas ESP32.

Os exemplos devem cumprir simultaneamente três funções:

1. ensinar o consumo correto da API pública;
2. permitir evolução e experimentação rápida em hardware;
3. fornecer cenários manuais e automatizáveis de regressão.

## 2. Problema

O repositório possui `src/main.cpp` e exemplos com estruturas diferentes, mas não oferece um catálogo uniforme que possa ser escolhido, compilado e gravado apenas pela seleção de um ambiente PlatformIO.

Código meramente informativo pode ficar desatualizado sem ser percebido. Os exemplos desta especificação devem ser consumidores reais da biblioteca e permanecer compiláveis.

## 3. Terminologia

- **Exemplo:** aplicação Arduino completa com `setup()` e `loop()`.
- **Perfil de placa:** configuração reutilizável de board, plataforma, upload, monitor e propriedades comuns.
- **Ambiente de exemplo:** environment PlatformIO que combina um exemplo com um perfil de placa.
- **Validação de hardware:** procedimento observável executado após upload, com resultado esperado documentado.

## 4. Arquitetura proposta

```text
perfil de placa
      +
seleção do exemplo pelo environment
      ↓
runner único de exemplos
      ↓
aplicação Arduino do exemplo
      ↓
API pública do IoTSmartSysCore
```

A seleção deve ocorrer em build time. O environment informa um identificador de exemplo; um runner dedicado inclui exatamente a aplicação correspondente.

Essa abordagem evita copiar arquivos antes do build, evita múltiplos `setup()`/`loop()` e preserva `src/main.cpp` como aplicação padrão do projeto.

Os nomes finais de arquivos e macros próprios da infraestrutura de seleção podem ser ajustados na implementação, desde que o contrato permaneça equivalente e não dependa de edição manual de fontes. Símbolos oficiais de pinout da placa não podem ser substituídos, redefinidos ou inferidos pelo implementador.

## 5. Organização esperada

Estrutura conceitual:

```text
examples/
├── executable/
│   ├── <example-id>/
│   │   ├── example.hpp ou example.cpp
│   │   └── README.md
│   └── ...
└── README.md

src/
├── main.cpp
└── <runner de exemplos>

configs/
├── <perfis de placa>.ini
└── <ambientes de exemplos>.ini
```

## 6. Requisitos funcionais

- **HWEX-001:** cada exemplo deve ser uma aplicação Arduino completa e executável em ESP32.
- **HWEX-002:** o usuário deve selecionar, compilar, gravar e monitorar um exemplo usando um environment PlatformIO identificado de forma estável.
- **HWEX-003:** selecionar um exemplo não pode exigir copiar, renomear ou editar arquivos-fonte.
- **HWEX-004:** exatamente um par `setup()`/`loop()` deve ser compilado em cada environment.
- **HWEX-005:** o build padrão atual deve continuar disponível e não pode ser transformado implicitamente em exemplo.
- **HWEX-006:** cada exemplo deve consumir somente APIs públicas destinadas aos usuários da biblioteca, salvo quando seu objetivo explícito for demonstrar uma API de baixo nível.
- **HWEX-007:** toda configuração e capability deve ser adicionada antes de `SmartSysApp::setup()`.
- **HWEX-008:** nenhum exemplo pode exceder o limite intencional de oito capabilities.
- **HWEX-009:** recursos que exigem chamadas periódicas próprias devem ser conduzidos no `loop()` antes ou junto de `app.handle()`, conforme seus contratos públicos.
- **HWEX-010:** cada environment deve declarar ou herdar explicitamente placa, framework, flags e dependências necessárias.
- **HWEX-011:** pinos e periféricos devem vir do pinout oficial da board selecionada. Somente placas sem pinout normativo podem usar configuração explícita pelo environment ou por configuração de hardware claramente localizada.
- **HWEX-012:** credenciais, tokens e endpoints privados não podem ser versionados nos exemplos.
- **HWEX-013:** a ausência de uma configuração obrigatória deve causar erro de build compreensível, em vez de produzir firmware com valores perigosos ou silenciosos.
- **HWEX-014:** o firmware deve registrar no boot o identificador do exemplo e as configurações de hardware relevantes, sem revelar segredos.
- **HWEX-015:** cada exemplo deve possuir um procedimento de hardware com montagem, estímulo, comportamento esperado e evidência mínima.
- **HWEX-016:** os exemplos devem poder ser compilados sem upload para detectar regressões de API.
- **HWEX-017:** alteração de API pública deve atualizar os exemplos afetados na mesma transação EKM.
- **HWEX-018:** environments `iotsmartsys_mcb_r1` devem importar automaticamente o pinout definido em `src/Platform/Espressif/Pinouts/SmartSys_MCB01_Pinouts.h` por meio da seleção da board; exemplos não podem incluir uma cópia ou uma definição paralela do pinout.
- **HWEX-019:** código e configuração dos exemplos MCB R1 devem referenciar os símbolos semânticos oficiais do pinout, sem literais numéricos de GPIO para funções cobertas pelo arquivo.
- **HWEX-020:** `basic_light` deve controlar a saída binária pelo símbolo `ITS_MCB01_RELAY_PIN`. `LED_BUILTIN` representa o LED da placa e não pode substituir o pino do relé.
- **HWEX-021:** `environment_dht` deve usar `ITS_MCB01_TEMPERATURE_SENSOR_PIN` para o sinal do DHT11.
- **HWEX-022:** environments baseados em `iotsmartsys_mcb_r1` e seus `build_flags` não podem redefinir `LED_BUILTIN`, `ITS_MCB01_RELAY_PIN`, `ITS_MCB01_TEMPERATURE_SENSOR_PIN` ou outro símbolo fornecido pelo pinout selecionado. Essa restrição não define o pinout do environment genérico `esp32_dev`.
- **HWEX-023:** se a board selecionada não importar o pinout esperado ou não houver um símbolo inequívoco para a função demonstrada, a Technical Readiness Review deve resultar em `Needs Clarification`; o executor não pode escolher um GPIO por equivalência numérica ou conveniência.

## 7. Contrato de nomenclatura

Identificadores devem ser curtos, estáveis e orientados ao comportamento.

Formato recomendado do environment:

```text
example_<example-id>_<board-profile>
```

Exemplos:

```text
example_basic_light_mcb_r1
example_environment_dht_mcb_r1
```

O nome não deve depender de porta serial, credencial ou máquina do desenvolvedor.

## 8. Conteúdo obrigatório de cada exemplo

Cada diretório deve conter:

- objetivo;
- APIs e capabilities demonstradas;
- placas e periféricos compatíveis;
- tabela de pinos;
- esquema textual de ligação, quando necessário;
- configurações obrigatórias;
- comandos de build, upload e monitor;
- sequência do teste manual;
- resultado esperado;
- limitações e riscos do hardware.

O código deve privilegiar clareza de consumo da API, sem duplicar a implementação interna da biblioteca.

## 9. Categorias do catálogo

O catálogo poderá evoluir incrementalmente nas seguintes categorias:

| Categoria | Exemplos possíveis |
|---|---|
| Bootstrap | aplicação mínima, LED e factory reset |
| Saídas | light, LED, switch, switch plug, valve, garage control |
| Entradas | push button, touch, door e PIR |
| Ambiente | temperatura, umidade e luminosidade |
| Medição | distância, fluxo e nível de água, GLP |
| Conectividade | provisioning, settings, MQTT e UART |
| Operação | registration, OTA e recuperação |

A tabela é um roadmap, não autorização para implementar todos os exemplos no primeiro recorte.

## 10. Perfis de placa

Configurações comuns de placa devem ser reutilizadas por mais de um exemplo. Propriedades específicas do computador, como `upload_port`, não devem fazer parte do contrato do exemplo e devem permanecer opcionais ou locais.

Um mesmo exemplo pode possuir environments para diferentes placas, desde que sua compatibilidade de pinos e periféricos seja comprovada.

### 10.1 Contrato de pinout da MCB R1

Para `iotsmartsys_mcb_r1`, a fonte normativa de GPIOs é:

```text
src/Platform/Espressif/Pinouts/SmartSys_MCB01_Pinouts.h
```

O arquivo é importado automaticamente pela infraestrutura da board selecionada. Exemplos devem consumir seus símbolos semânticos, mantendo separadas as funções físicas da placa:

| Função | Símbolo obrigatório |
|---|---|
| LED integrado | `LED_BUILTIN` |
| Saída binária/relé | `ITS_MCB01_RELAY_PIN` |
| Sinal do DHT11 | `ITS_MCB01_TEMPERATURE_SENSOR_PIN` |

Os valores numéricos resolvidos podem ser apresentados no README para montagem e diagnóstico, mas não substituem os símbolos no código ou na configuração. Alterar o arquivo oficial de pinout está fora do escopo desta especificação.

## 11. Segurança e operação

- Saídas devem iniciar em estado seguro definido pelo exemplo.
- O README deve alertar sobre relés, válvulas, motores ou cargas externas.
- Exemplos destrutivos, de factory reset ou OTA devem declarar claramente seus efeitos.
- Serviços externos devem usar configuração local ou ambiente de teste explicitamente escolhido.
- Logs não podem expor senha Wi-Fi, token, API key ou credencial MQTT.

## 12. Validação

### Validação automatizável

- resolução de todos os environments de exemplo;
- build individual de cada environment suportado;
- verificação de que somente uma aplicação foi vinculada;
- verificação de ausência de segredos versionados;
- `git diff --check`.

### Validação em hardware

Para cada combinação declarada como validada:

1. compilar;
2. gravar a placa;
3. confirmar o identificador no boot;
4. executar os estímulos documentados;
5. observar saídas, logs e integrações esperadas;
6. registrar placa, exemplo, resultado e limitações.

Build aprovado permite estado `Implemented`; somente evidência suficiente do comportamento permite `Validated`.

## 13. Primeiro recorte aprovado

O primeiro recorte de implementação deve validar a infraestrutura, não cobrir todo o catálogo:

1. criar o runner e a seleção por environment;
2. usar `iotsmartsys_mcb_r1` como perfil canônico;
3. criar `basic_light`, demonstrando uma capability de saída simples no hardware da MCB R1;
4. criar `environment_dht`, demonstrando temperatura e umidade com DHT11 e chamada periódica de `handle()`;
5. criar os environments `example_basic_light_mcb_r1` e `example_environment_dht_mcb_r1`;
6. manter o build padrão existente;
7. compilar ambos os environments sem upload na validação automatizada;
8. validar pelo menos um exemplo em hardware antes de declarar a infraestrutura `Validated`.

## 14. Decisões aprovadas

- **HWEX-DEC-001:** a placa canônica do primeiro recorte é `iotsmartsys_mcb_r1`.
- **HWEX-DEC-002:** os primeiros exemplos são `basic_light` e `environment_dht`.
- **HWEX-DEC-003:** os exemplos podem usar a infraestrutura externa real atual somente por configuração privada já existente; nenhum segredo pode ser incorporado ao código ou à configuração versionada.
- **HWEX-DEC-004:** a matriz inicial de CI deve compilar, sem upload, `example_basic_light_mcb_r1` e `example_environment_dht_mcb_r1`.
- **HWEX-DEC-005:** o pinout oficial importado pela board é a única autoridade para GPIOs da MCB R1; exemplos não podem redefini-lo ou inferir pinos alternativos.

`HWEX-DEC-001` a `HWEX-DEC-004` foram aprovadas pelo responsável humano em 22/07/2026. `HWEX-DEC-005` foi aprovada em 23/07/2026 durante a reabertura normativa da especificação.

## 15. Fora de escopo

- implementar todos os exemplos de capabilities em uma única mudança;
- substituir testes unitários;
- automatizar interação física sem infraestrutura específica;
- alterar APIs públicas para facilitar exemplos;
- corrigir comportamentos da biblioteca encontrados durante os testes;
- incorporar credenciais ao repositório;
- declarar suporte ESP8266 ou ESP-IDF nativo.

## 16. Critérios de aceite da infraestrutura

- um environment seleciona um exemplo sem edição manual;
- o build padrão permanece funcional;
- exemplos usam a API pública e compilam contra o código do worktree;
- documentação permite reproduzir montagem e teste;
- pelo menos um exemplo é gravado e validado em ESP32;
- inclusão de novo exemplo segue um padrão simples e documentado;
- mapa, changelog e estados da especificação são reconciliados.
- `basic_light` e `environment_dht` usam os símbolos oficiais exigidos por `HWEX-020` e `HWEX-021`;
- nenhum environment baseado em `iotsmartsys_mcb_r1` ou respectivo `build_flag` redefine símbolos do pinout da MCB R1;
- a seleção de `iotsmartsys_mcb_r1` torna os símbolos necessários disponíveis sem inclusão ou configuração paralela feita pelo exemplo.

## 17. Relações

- `PUBLIC-API-COMPATIBILITY.md`;
- `CORE-RUNTIME-LIFECYCLE.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `EKM-CHG-0002`.

## 18. Technical Readiness Review

**Resultado:** Implementable

**Revisão vigente:** 23/07/2026, executada integralmente contra o baseline `implement_ekm` / `b90fe872ed70a6769bd278d3fa76b18f9d9b968a`, com worktree inicial limpo.

**Revisão anterior:** a análise de 23/07/2026 que declarou `Implementable` foi invalidada após a identificação de uma contradição entre sua regra de retomada e `AGENTS.md`/`EKM-GUIDELINES.md`. As evidências técnicas abaixo permanecem históricas e úteis, mas não autorizam implementação.

**Data:** 23/07/2026

**Motivo:** a versão 1.1 adicionou o contrato normativo de pinout da MCB R1 após a primeira implementação. Esta análise confrontou integralmente `HWEX-018` a `HWEX-023` e `HWEX-DEC-005` com o worktree atual (`src/pins.h`, `src/SmartSysApp.h`, `src/Platform/Espressif/Pinouts/SmartSys_MCB01_Pinouts.h`, `boards/iotsmartsys_mcb_r1.json`, `configs/executable_examples.ini`, `examples/executable/basic_light/example.hpp`, `examples/executable/environment_dht/example.hpp`).

**Evidência por requisito:**

- `HWEX-018` — Conforme. `boards/iotsmartsys_mcb_r1.json` define `IOTSMARTSYS_MCB01=1` e `IOTSMARTSYS_BOARD_REV=1` via `extra_flags`; `src/pins.h` inclui `SmartSys_MCB01_Pinouts.h` sob essas macros; `src/SmartSysApp.h` inclui `pins.h`; ambos os exemplos incluem `SmartSysApp.h`. O pinout é importado automaticamente pela seleção da board, sem cópia paralela nos exemplos.
- `HWEX-019` e `HWEX-020` — Não conforme. `configs/executable_examples.ini` define `-DEXAMPLE_LIGHT_PIN=26` e `examples/executable/basic_light/example.hpp` consome apenas `EXAMPLE_LIGHT_PIN`; o código e a configuração não referenciam `ITS_MCB01_RELAY_PIN`, usando um literal numérico para uma função coberta pelo pinout oficial.
- `HWEX-021` — Não conforme. `configs/executable_examples.ini` define `-DEXAMPLE_DHT_PIN=23` e `examples/executable/environment_dht/example.hpp` consome apenas `EXAMPLE_DHT_PIN`; o código e a configuração não referenciam `ITS_MCB01_TEMPERATURE_SENSOR_PIN`.
- `HWEX-022` — Conforme. Nenhum environment ou `build_flags` baseado em `iotsmartsys_mcb_r1` redefine `LED_BUILTIN`, `ITS_MCB01_RELAY_PIN`, `ITS_MCB01_TEMPERATURE_SENSOR_PIN` ou outro símbolo do pinout selecionado; a definição de `-DLED_BUILTIN=23` permanece isolada no environment genérico `esp32_dev`, conforme exceção prevista.
- `HWEX-023` — Não acionado. A board importa o pinout esperado e existe símbolo inequívoco para cada função demonstrada (`ITS_MCB01_RELAY_PIN`, `ITS_MCB01_TEMPERATURE_SENSOR_PIN`); não há lacuna decisória a esclarecer.
- `HWEX-DEC-005` — Não conforme na implementação atual pelo mesmo motivo de `HWEX-019` a `HWEX-021`: os exemplos não consomem o pinout oficial como autoridade única, embora também não o redefinam.

**Conclusão:** os desvios identificados (`HWEX-019`, `HWEX-020`, `HWEX-021`, `HWEX-DEC-005`) não dependem de decisão ausente, contraditória ou insuficientemente especificada — o símbolo oficial exigido já existe e seu valor coincide com o literal hoje utilizado (26 e 23). A correção é mecânica: substituir os literais/macros próprios dos exemplos pelos símbolos oficiais no código e na configuração, sem alterar comportamento observável, API pública ou critério de aceite. Nenhum requisito obrigatório resultou em `Needs Clarification`.

**Verificações da revisão vigente:** requisitos `HWEX-001` a `HWEX-023`, decisões `HWEX-DEC-001` a `HWEX-DEC-005`, critérios de aceite, condições de borda, fora de escopo, contratos de API e runtime e cadeia de importação do pinout foram confrontados com o baseline. Não foi encontrada ambiguidade ou mudança necessária fora da autorização normativa.

**Autorização de implementação:** a revisão vigente confirma o resultado `Implementable` antes da primeira alteração de código, build, exemplos ou documentação de implementação. A correção autorizada é limitada a remover `EXAMPLE_LIGHT_PIN` e `EXAMPLE_DHT_PIN`, consumir diretamente `ITS_MCB01_RELAY_PIN` e `ITS_MCB01_TEMPERATURE_SENSOR_PIN` e reconciliar a documentação e as evidências afetadas.

### 18.1 Reconciliação da implementação

Após a autorização acima:

- `basic_light` passou a validar e consumir diretamente `ITS_MCB01_RELAY_PIN`;
- `environment_dht` passou a validar e consumir diretamente `ITS_MCB01_TEMPERATURE_SENSOR_PIN`;
- `EXAMPLE_LIGHT_PIN` e `EXAMPLE_DHT_PIN`, juntamente com seus GPIOs literais, foram removidos dos environments;
- os READMEs foram reconciliados com os símbolos oficiais, preservando os valores numéricos apenas como informação de montagem e diagnóstico;
- `HWEX-011`, `HWEX-019`, `HWEX-020`, `HWEX-021` e `HWEX-DEC-005` estão conformes por inspeção e build.

Validações automatizáveis aprovadas:

- resolução da configuração por `pio project config --json-output`;
- `pio run -e esp32_dev`;
- `pio run -e example_basic_light_mcb_r1`;
- `pio run -e example_environment_dht_mcb_r1`;
- exatamente um símbolo `setup()` e um `loop()` em cada firmware de exemplo;
- ausência das macros locais de pino e de redefinições dos símbolos oficiais nos examples/environments;
- busca por indicadores de segredos no catálogo executável;
- `git diff --check`.

A validação física obrigatória permanece pendente. Por isso o estado é `Implemented`, não `Validated`, e a entrega permanece `Not Ready`.
