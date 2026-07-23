# Especificação — Exemplos executáveis e validação em hardware

**ID:** IOTSSC-HW-EXAMPLES

**Tipo:** Normativo

**Estado normativo:** Active

**Estado da implementação:** Not Started

**Versão:** 1.0

**Última atualização:** 22/07/2026

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

Os nomes finais de arquivos e macros podem ser ajustados na implementação, desde que o contrato de seleção permaneça equivalente e não dependa de edição manual de fontes.

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
- **HWEX-011:** pinos e periféricos devem ser configuráveis pelo environment ou por uma configuração de hardware claramente localizada; não podem ficar ocultos em código sem documentação.
- **HWEX-012:** credenciais, tokens e endpoints privados não podem ser versionados nos exemplos.
- **HWEX-013:** a ausência de uma configuração obrigatória deve causar erro de build compreensível, em vez de produzir firmware com valores perigosos ou silenciosos.
- **HWEX-014:** o firmware deve registrar no boot o identificador do exemplo e as configurações de hardware relevantes, sem revelar segredos.
- **HWEX-015:** cada exemplo deve possuir um procedimento de hardware com montagem, estímulo, comportamento esperado e evidência mínima.
- **HWEX-016:** os exemplos devem poder ser compilados sem upload para detectar regressões de API.
- **HWEX-017:** alteração de API pública deve atualizar os exemplos afetados na mesma transação EKM.

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

As decisões foram aprovadas pelo responsável humano em 22/07/2026.

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

## 17. Relações

- `PUBLIC-API-COMPATIBILITY.md`;
- `CORE-RUNTIME-LIFECYCLE.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `EKM-CHG-0002`.
