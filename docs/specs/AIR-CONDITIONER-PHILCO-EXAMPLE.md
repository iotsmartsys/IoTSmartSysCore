# Exemplo executável da AirConditionerCapability para aparelhos Philco

**Identificador:** AIR-CONDITIONER-PHILCO-EXAMPLE  
**Status normativo:** Draft  
**Estado da implementação:** Not Started  
**Estado da entrega:** Not Ready  
**Technical Readiness Review:** Pending Review  
**Relação:** Amends `EXECUTABLE-HARDWARE-EXAMPLES`  
**Plataforma:** Arduino sobre ESP32  
**Board de referência:** `iotsmartsys_mcb_r1`

## 1. Objetivo

Adicionar ao catálogo da IoTSmartSysCore um exemplo executável que demonstre a utilização da `AirConditionerCapability` para interpretar comandos infravermelhos de um aparelho de ar-condicionado Philco.

O exemplo deve permitir compilação, gravação, monitoramento serial e validação física por meio de um environment PlatformIO estável.

## 2. Contexto

A IoTSmartSysCore possui uma `AirConditionerCapability`, um sensor de comandos infravermelhos e um interpretador específico para aparelhos Philco.

O catálogo de exemplos executáveis deve demonstrar a composição desses elementos em hardware real, seguindo o modelo de seleção em build time definido por `EXECUTABLE-HARDWARE-EXAMPLES`.

## 3. Premissas normativas

- Existe uma forma pública e suportada de registrar uma `AirConditionerCapability` em `SmartSysApp`.
- Essa forma de registro integra a capability ao ciclo de vida, ao gerenciamento de estado e ao mecanismo de publicação de eventos da aplicação.
- O sensor infravermelho e o interpretador Philco podem ser fornecidos ou selecionados por essa forma de registro.
- O mecanismo existente respeita o limite vigente de oito capabilities por aplicação.

Estas premissas constituem precondições do exemplo. Criar, corrigir ou ampliar o mecanismo de registro da `AirConditionerCapability` não faz parte desta especificação.

## 4. Escopo

Esta especificação inclui:

- criação do exemplo executável `air_conditioner_philco`;
- criação do environment `example_air_conditioner_philco_mcb_r1`;
- uso da board `iotsmartsys_mcb_r1`;
- recepção de comandos infravermelhos pelo conector `EXT_IO33`;
- uso do interpretador Philco existente;
- integração do exemplo ao runner de exemplos executáveis;
- documentação de montagem, execução e validação física;
- compilação do novo environment na matriz de integração contínua.

## 5. Fora de escopo

Não fazem parte desta especificação:

- criação ou alteração da `AirConditionerCapability`;
- criação ou alteração da API usada para registrar a capability;
- correções no ciclo de vida da capability;
- criação de novos interpretadores infravermelhos;
- suporte a outras marcas ou modelos;
- transmissão de comandos infravermelhos;
- controle ativo do aparelho de ar-condicionado;
- alteração dos códigos reconhecidos pelo interpretador Philco;
- alteração do protocolo de eventos ou transporte;
- alteração do pinout oficial da MCB R1;
- inclusão de credenciais, endpoints ou configurações privadas no exemplo.

## 6. Requisitos funcionais

- **ACPH-001:** deve existir um exemplo executável identificado de forma estável como `air_conditioner_philco`.
- **ACPH-002:** deve existir um environment PlatformIO denominado `example_air_conditioner_philco_mcb_r1`.
- **ACPH-003:** o environment deve selecionar a board `iotsmartsys_mcb_r1`.
- **ACPH-004:** o exemplo deve criar ou obter um sensor de comandos infravermelhos associado ao pino oficial `ITS_MCB01_J4_EXT_IO33`.
- **ACPH-005:** o exemplo não pode usar o literal `33` como substituto do símbolo oficial do pinout.
- **ACPH-006:** o exemplo deve utilizar `PhilcoAirConditionerInterpreter`.
- **ACPH-007:** o exemplo deve registrar uma `AirConditionerCapability` por meio da forma pública e suportada disponibilizada por `SmartSysApp`.
- **ACPH-008:** o exemplo deve chamar `SmartSysApp::setup()` uma única vez durante a inicialização.
- **ACPH-009:** o exemplo deve chamar `SmartSysApp::handle()` continuamente no loop principal.
- **ACPH-010:** o sensor infravermelho deve permanecer válido durante toda a execução da aplicação.
- **ACPH-011:** o interpretador Philco deve permanecer válido durante toda a execução da aplicação.
- **ACPH-012:** comandos reconhecidos pelo interpretador devem atualizar o estado da capability e seguir o mecanismo normal de publicação de eventos.
- **ACPH-013:** comandos não reconhecidos devem preservar o comportamento vigente da `AirConditionerCapability` e do `PhilcoAirConditionerInterpreter`.
- **ACPH-014:** o boot deve registrar no monitor serial, no mínimo, o identificador do exemplo, a board, o símbolo lógico do conector, o GPIO resolvido e o interpretador selecionado.
- **ACPH-015:** o exemplo deve ser selecionado exclusivamente em build time pelo runner de exemplos executáveis.
- **ACPH-016:** cada build do environment deve conter exatamente um par `setup()`/`loop()`.

## 7. Configuração de build

O environment `example_air_conditioner_philco_mcb_r1` deve:

- herdar o perfil ESP32 usado pelos demais exemplos;
- selecionar `iotsmartsys_mcb_r1`;
- excluir o `main.cpp` regular;
- habilitar o runner de exemplos;
- selecionar exclusivamente `air_conditioner_philco`;
- declarar explicitamente a dependência necessária para recepção IR;
- habilitar as flags necessárias ao sensor infravermelho;
- reutilizar as configurações privadas já suportadas pelo projeto;
- não redefinir símbolos fornecidos pelo pinout oficial da board.

## 8. Hardware de referência

### Componentes

- uma board IoTSmartSys MCB R1;
- um receptor infravermelho compatível com o nível elétrico da board;
- um controle remoto Philco compatível com os códigos reconhecidos pelo interpretador existente;
- alimentação e conexão de programação adequadas.

### Ligação do sinal

O sinal do receptor infravermelho deve ser conectado ao `EXT_IO33` da MCB R1.

No firmware, o exemplo deve referenciar esse sinal exclusivamente por:

`ITS_MCB01_J4_EXT_IO33`

Alimentação e aterramento devem seguir as características elétricas do receptor utilizado e da MCB R1.

## 9. Estados observáveis

A especificação não altera o vocabulário existente do interpretador Philco.

A validação deve considerar os estados atualmente produzidos pelo interpretador, incluindo:

- desligado;
- temperaturas reconhecidas;
- modo seco;
- ventilação;
- aquecimento;
- automático;
- comando desconhecido.

Os valores exatos publicados devem permanecer os definidos pelo comportamento vigente da capability e do interpretador.

## 10. Falhas e condições de borda

- A ausência de comando IR não deve produzir atualização espúria de estado.
- Repetições geradas pela manutenção de uma tecla pressionada devem preservar o tratamento vigente do sensor infravermelho.
- Um comando não reconhecido não pode causar reinicialização, bloqueio ou corrupção da aplicação.
- Falha no registro da capability deve impedir que o exemplo seja considerado inicializado com sucesso.
- A indisponibilidade da forma pública de registro presumida por esta especificação deve resultar em `Needs Clarification` durante a Technical Readiness Review; ela não autoriza criação ou modificação dessa API.
- A ausência de alguma dependência ou flag necessária ao sensor IR deve ser tratada como falha de build, não por fallback silencioso.

## 11. Documentação do exemplo

O README do exemplo deve informar:

- objetivo;
- hardware necessário;
- ligação ao `EXT_IO33`;
- símbolo oficial utilizado;
- nome do environment;
- comandos de build, upload e monitoramento;
- dependências relevantes;
- procedimento de validação física;
- estados esperados;
- restrição a controles Philco compatíveis;
- orientação para manter credenciais e endpoints fora do exemplo.

## 12. Validações obrigatórias

### Validações automatizáveis

- resolução do environment pelo PlatformIO;
- build de `example_air_conditioner_philco_mcb_r1`;
- build dos exemplos executáveis já existentes;
- confirmação de exatamente um `setup()` e um `loop()` no artefato;
- confirmação da seleção exclusiva do exemplo;
- confirmação do uso de `ITS_MCB01_J4_EXT_IO33`;
- ausência do GPIO `33` como configuração paralela do pino IR;
- ausência de redefinição de símbolos do pinout da MCB R1;
- inclusão do environment na matriz de CI;
- busca por credenciais ou endpoints privados adicionados pelo exemplo;
- verificação de formatação do diff.

### Validação física

1. Montar o receptor IR no `EXT_IO33`.
2. Compilar e gravar `example_air_conditioner_philco_mcb_r1`.
3. Abrir o monitor serial.
4. Confirmar a identificação do exemplo, da board, do GPIO e do interpretador.
5. Enviar um comando Philco reconhecido para desligamento.
6. Enviar ao menos um comando reconhecido de temperatura.
7. Enviar ao menos um comando reconhecido de modo.
8. Confirmar a atualização correspondente da capability.
9. Confirmar a publicação dos eventos pela integração configurada.
10. Enviar um comando não reconhecido e confirmar o comportamento vigente sem falha da aplicação.
11. Manter uma tecla pressionada e verificar que o tratamento de repetição permanece estável.

## 13. Critérios de aceite

A implementação será aceita quando:

- todos os requisitos `ACPH-001` a `ACPH-016` forem atendidos;
- o environment compilar em integração contínua;
- os exemplos anteriores continuarem compilando;
- o pino IR for obtido exclusivamente do pinout oficial;
- a documentação permitir reproduzir a montagem e a execução;
- a validação física comprovar recepção, interpretação, atualização de estado e publicação de eventos;
- nenhuma API pública ou comportamento fora do escopo tiver sido criado ou alterado;
- conhecimento, implementação e evidências estiverem reconciliados conforme a EKM.

## 14. Technical Readiness Review

Antes de qualquer alteração de código, build, testes, automação ou documentação de implementação, a IA Executora deve confrontar integralmente esta especificação com o baseline real.

Em particular, deve confirmar a existência e a adequação da forma pública de registro presumida na seção 3.

Se essa precondição não estiver comprovada, o resultado obrigatório será `Needs Clarification`, sem implementação parcial do exemplo.

## 15. Decisões

- **ACPH-DEC-001:** o exemplo é específico para aparelhos Philco.
- **ACPH-DEC-002:** a board de referência é `iotsmartsys_mcb_r1`.
- **ACPH-DEC-003:** o receptor IR utiliza `ITS_MCB01_J4_EXT_IO33`.
- **ACPH-DEC-004:** a API de registro da capability é tratada como preexistente e fica fora do escopo.
- **ACPH-DEC-005:** a validação em hardware é obrigatória para declarar a implementação `Validated`.