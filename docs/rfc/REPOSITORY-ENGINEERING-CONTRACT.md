# Repository Engineering Contract — recorte AVANT

**Classe da fonte:** Normativa

**Estado:** Aprovado [`Approved`]

**Versão:** 0.1

**Repositório:** `iotsmartsys/IoTSmartSysCore`

**Escopo:** `AVIR-INTERPRETER`, definido na seção 1

**Responsável técnico:** Marcelo Miranda — Arquiteto do IoTSmartSysCore

**Aprovação:** Versão 0.1 aprovada em 21/10/2026 UTC

**Decisão humana:** `AVIR-ENG-DEC-001`, resposta explícita de Marcelo Miranda
nesta sessão, registrada em 2026-10-21T02:56:42Z: "Aprovo o contrato v0.1
integralmente para o escopo AVIR-INTERPRETER, incluindo precedentes e itens
não aplicáveis." Identificação informada: "Marcelo Miranda e sou o arquiteto".

**Baseline técnica:** `b72778c7e07819a0d212d8237cbd35078a03e161`

## 1. Alcance e autoridade

Este contrato governa somente a construção da extensão definida em
[AVANT-LS963231305-IR-COMMAND.md](../specs/AVANT-LS963231305-IR-COMMAND.md),
versão 0.1: interpreter concreto, constantes públicas, acréscimo ao enum
existente e documentação de uso. Cobre as dependências de interface, tipos
compartilhados e build necessárias para essa extensão.

Não qualifica todos os interpreters, capabilities, plataformas ou serviços do
repositório. Sensor, processor e consumidores existentes são fronteiras a
preservar, não componentes autorizados a evoluir.

A qualificação segue o
[EKOM 5.0, revisão e049856](https://github.com/iotsmartsys/EKOM-guidelines/blob/e0498569169a46257a0f7a278cb48ff17774ded0/docs/REPOSITORY-READINESS.md).
Sua aprovação aplica esse pré-requisito ao recorte AVANT, sem migração global
do roteamento local, sem reinterpretação de decisões anteriores e sem dispensa
das restrições de implementação, testes, hardware ou entrega Git.

As fontes locais referenciadas correspondem à baseline técnica acima.
Aplicam-se a especificação da tarefa, este contrato quando aprovado,
precedentes oficiais, demais código existente e escolhas locais, nessa ordem
e dentro da autoridade de cada fonte. A especificação governa comportamento;
este contrato não altera seus nove requisitos ou cinco critérios.

## 2. Arquitetura e limites — EC-AVIR-01

- Manter a interface e o enum em `src/Contracts/Interpreters/`.
- Colocar o interpreter concreto em `src/Platform/Arduino/Interpreters/`,
  derivado de `iotsmartsys::core::IRCommandInterpreter`.
- Preservar a separação entre recepção, interpretação e consumo do resultado.
  Não registrar serviços, criar capability, acionar hardware ou conduzir o
  lifecycle de outro objeto.
- Não modificar `SmartSysApp`, `main.cpp`, sensor IR, processor, interpreters
  anteriores, protocolos de eventos ou infraestrutura compartilhada.

## 3. Organização de arquivos — EC-AVIR-02

- Usar par `.h`/`.cpp` com mesmo nome-base para o novo tipo concreto.
- Manter as constantes públicas junto à declaração do novo interpreter.
- Alterar `IRCommandInterpreter.h` somente pelo acréscimo ao final do enum.
- Registrar o guia em `docs/IR-COMMAND-INTERPRETERS.md`; incluir consumo
  AVANT e referência à especificação, sem alterar exemplos executáveis.
- Usar `docs/reports/` para evidências, `docs/rfc/KNOWLEDGE-MAP.md` para
  localização e `docs/rfc/EKOM-CHANGELOG.md` para estado. Não criar camada,
  diretório estrutural de código ou factory.

## 4. Nomenclatura e estilo — EC-AVIR-03

- Seguir o precedente IoTSM1541: tipo em PascalCase com sufixo
  `ControlInterpreter`, agrupamento de constantes com sufixo `Constants`,
  constantes em `UPPER_SNAKE_CASE` e métodos em camelCase.
- Usar namespace `iotsmartsys::platform::arduino` no novo tipo, `#pragma once`
  no header, indentação de quatro espaços e chaves em linha própria.
- Manter `interpret(const iotsmartsys::core::IRCommand &command)` como
  `override`, retornando `std::string`; não criar API paralela.
- Usar identificadores ASCII. Comentários devem explicar somente lógica não
  evidente; documentação em português pode usar acentuação.
- Tornar os novos headers autocontidos mediante includes necessários, sem
  corrigir incidentalmente headers legados ou depender da ordem de includes
  da aplicação.

## 5. Dependências — EC-AVIR-04

- O novo interpreter pode depender da interface em Contracts e dos tipos
  padrão C++ necessários. Não incluir implementação concreta de sensor,
  application facade, serviços de rede ou armazenamento.
- Contracts não deve passar a incluir o novo interpreter de Platform.
- Não adicionar bibliotecas, flags, environments ou dependência direta de
  `IRremoteESP8266`: a interpretação recebe um comando já formado.
- Preservar a cadeia de includes existente e seus consumidores, sem
  refatoração transversal para introduzir a extensão.

## 6. Padrões de implementação — EC-AVIR-05

- Comparar o valor integral de `IRCommand::code`, sem estreitamento, máscara,
  transformação, filtro de protocolo ou condição de `triggered`.
- Implementar os retornos e as constantes conforme a especificação. Manter
  o caminho desconhecido observável como `"unknown"`.
- Acrescentar o identificador de modelo ao final de `IRControlModel`, sem
  renumerar, renomear ou mudar o significado dos membros anteriores.
- Não introduzir cache, singleton, estado entre chamadas, task, atraso,
  temporizador, callback ou seleção automática.
- Novos serviços, DTOs e adapters não se aplicam: o contrato `IRCommand`
  existente é suficiente e não deve ser ampliado.

## 7. Persistência e dados — EC-AVIR-06

- O argumento é referência constante emprestada durante a chamada. Não
  armazenar sua referência, alterar seus campos ou assumir sua propriedade.
- Devolver o texto por valor. Não criar ownership de sensor, alocação manual
  ou recursos que precisem de lifecycle próprio; o retorno `std::string`
  segue a interface existente.
- Persistência, banco de dados e migração não se aplicam: esta extensão é
  sem estado e não escreve em NVS, arquivos ou serviços externos.

## 8. Erros e observabilidade — EC-AVIR-07

- Código não reconhecido é um resultado de interpretação, não sucesso de
  execução. Representá-lo por `"unknown"`, sem aplicar comando substituto,
  lançar exceção ou ocultar a entrada com retorno vazio.
- Não exigir logger, serial ou sensor para interpretar. Não criar logs por
  tecla; o retorno é a observabilidade contratada, conforme o precedente.
- Falha de compilação, verificação não executada ou ausência de evidência
  deve permanecer explícita no relatório, sem fallback de sucesso.

## 9. Tecnologias e abstrações — EC-AVIR-08

- Usar C++17 e Arduino sobre ESP32 conforme `configs/base_esp.ini`.
- Preservar as flags vigentes, incluindo ausência de exceções e RTTI.
- Não ampliar suporte a ESP8266 ou ESP-IDF nativo, atualizar toolchain ou
  introduzir novos mecanismos de injeção, reflexão ou geração de código.

## 10. Build, testes e conformidade — EC-AVIR-09

- Na implementação autorizada, executar `pio run -e esp32_dev`, sem upload,
  monitor, testes ou deploy. Registrar resultado terminal e código de saída.
- Confirmar compilação do novo componente, além do sucesso global. O filtro
  existente inclui fontes e exclui `main.cpp`; não alterar essa configuração.
- Confrontar o delta com as regras EC-AVIR-01 a EC-AVIR-11 e os critérios
  AVIR-AC-001 a AVIR-AC-005. Inspeção deve cobrir todos os oito códigos,
  entradas desconhecidas, comparação integral e preservação dos consumidores.
- Não criar nem executar testes: a versão 0.1 contrata inspeção estática e
  build. Compilação não demonstra comportamento em execução nem recepção
  física. Nenhuma validação física é autorizada por este contrato.
- Executar `git diff --check` e a guarda documental EKOM nos documentos
  alterados. Validação estrutural não aprova arquitetura ou contrato.
- Nesta qualificação documental, não executar build nem testes. Dependência
  ou falha posterior de construção deve ser tratada conforme o escopo, sem
  alteração de configuração privada ou remediação alheia ao recorte.

## 11. Precedentes oficiais — EC-AVIR-10

Após aprovação, os exemplos abaixo são oficiais apenas para as regras citadas.
Não homologam integralmente código legado ou comportamentos fora do recorte.

| Alteração | Fonte e símbolo na baseline | Regra exemplificada | Limite |
|---|---|---|---|
| Interface e enum aditivo | `src/Contracts/Interpreters/IRCommandInterpreter.h`: `IRCommandInterpreter`, `IRControlModel` | EC-AVIR-01, 04, 05 | Preservar assinatura e valores existentes. |
| Classe concreta e constantes | `src/Platform/Arduino/Interpreters/IoTSM1541ControlInterpreter.h`: classe e `IoTSM1541ControlInterpreterConstants` | EC-AVIR-02, 03, 10 | Seguir organização, não copiar nomes de teclas ou includes desnecessários. |
| Interpretação por código | `src/Platform/Arduino/Interpreters/IoTSM1541ControlInterpreter.cpp`: `interpret` | EC-AVIR-05, 07 | Preservar AVANT sem modificar o mapeamento IoTSM1541. |
| Entrada e ownership | `src/Contracts/Sensors/IIRCommandSensor.h`: `IRCommand`; assinatura constante em `IRCommandInterpreter.h` | EC-AVIR-06 | Não conduzir nem possuir sensor; não ampliar os tipos. |
| Build Arduino | `platformio.ini`: `env:esp32_dev`; `configs/base_esp.ini`: `env:base_esp` | EC-AVIR-08, 09 | Somente construção do recorte; outros environments não são qualificados. |

## 12. Evolução e exceções — EC-AVIR-11

- Mudança de comportamento exige contrato funcional e análise aplicáveis.
  Nova abstração transversal, factory, lifecycle, persistência ou consumidor
  está fora deste contrato e deve retornar ao Arquiteto antes de implementar.
- Exceção exige decisão humana com regra, motivo, alcance e validade.
  Nenhuma exceção é solicitada nesta versão.
- Mudança material das regras, baseline ou dependências abrangidas exige
  reavaliação do alcance. Edição documental de estado não invalida, por si só,
  a conformidade técnica.
- Aprovação deste contrato não substitui análise da tarefa, avaliação de
  readiness ou confirmação humana da habilitação e seu alcance.

## 13. Informações sensíveis e versionamento

Nenhuma informação sensível pode integrar código, documentação, exemplos,
relatórios, testes, logs, anexos ou metadados Git. Usar somente códigos IR e
dados sintéticos no material do recorte. Conferir delta e commits antes do
push; não usar `git add -f` para incorporar configuração privada.

Os destinos locais `private.ini`, `wifi_credentials.ini`,
`configs/wifi_credentials.ini` e `src/Config/WifiCredentials.generated.h`
estão ignorados e não rastreados na baseline. Não ler seus valores para esta
tarefa nem copiá-los para evidências. O pré-build existente pode gerar o
header ignorado; isso não autoriza versioná-lo ou publicá-lo.

A observação prévia de credenciais em outro environment de `platformio.ini`
permanece um achado fora da modificação AVANT, não aceito como conforme nem
como débito técnico. Esta qualificação não cobre esse environment ou elimina
a exposição histórica. O build `esp32_dev` não requer copiar tais valores.
Não reproduzir segredos nem corrigir configuração fora do recorte. Rotação,
reescrita de histórico e operações externas exigem ordem própria; `.gitignore`
não remove conteúdo já rastreado.

## 14. Aprovação e habilitação

- `AVIR-ENG-DEC-001` aprova a versão 0.1, incluindo alcance, precedentes e
  justificativas de não aplicabilidade. Essa aprovação não exige reprodução
  a cada tarefa enquanto versão e alcance permanecerem válidos.
- A avaliação de conformidade e a confirmação humana da habilitação ficam
  registradas separadamente em
  [REPOSITORY-READINESS.md](REPOSITORY-READINESS.md).
- Nenhuma habilitação global do IoTSmartSysCore ou correção do achado fora
  do recorte é proposta.
