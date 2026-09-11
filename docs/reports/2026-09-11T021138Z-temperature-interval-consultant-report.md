# Correção pequena — intervalo de leitura de temperatura

**Classe da fonte:** Relatório

**Papel:** Consultor de Arquitetura

**Especificação:** Não se aplica — correção pequena autorizada sem especificação

**Revisão confrontada:** implementação de `TemperatureSensorCapability` na baseline local

**Estado:** Implementada e construída; aguardando avaliação do Arquiteto

## Autorização e alcance

O Arquiteto classificou a correção como pequena e ordenou implementação sem
especificação pelo Consultor. Aplicado o EKOM 4.7 local declarado no AGENTS.md;
esta atuação não migra a governança para 5.0. O recorte é exclusivamente o
controle de cadência de `TemperatureSensorCapability` e seu registro no mapa.
A alteração preexistente em `MqttService.cpp` foi preservada na árvore original;
a entrega foi produzida em worktree isolada derivada de `main`.

## Causa e implementação

A condição anterior só aguardava o intervalo quando `lastValue() > 0.0f`.
O Arquiteto confirmou temperatura de −17,25 °C no dispositivo afetado.
Um indicador privado de tentativa realizada substitui a dependência do valor
medido. A primeira tentativa permanece imediata. As seguintes consultam o
intervalo existente, incluindo temperaturas negativas, zero e tentativas
inválidas. Intervalo zero continua permitindo leitura em cada chamada.

Não houve alteração do helper compartilhado, assinaturas públicas, defaults,
conversão NTC, faixa válida ou mecanismo de publicação. Nenhuma nova camada,
dependência ou mudança de persistência foi introduzida.

## Evidências e limitações

- Inspeção da condição e dos caminhos válido/inválido: ambos registram o tempo
  pelo mecanismo existente e passam a respeitar a guarda após a primeira
  tentativa, independentemente do sinal da temperatura.
- `pio run -e esp32_dev`: SUCCESS, código 0; 31,13 segundos, plataforma
  Espressif32 6.5.0, Arduino, board `esp32dev`.
- `git diff --check`: código 0.
- Guarda estrutural EKOM global: código 1. A saída foi comparada com a baseline
  e é idêntica; os apontamentos preexistentes não foram alterados neste recorte.
- Testes automatizados e hardware: não executados; nenhum teste foi criado ou
  alterado. O build não comprova a cadência observada no dispositivo C3.

Implementação construída e disponível para avaliação do Arquiteto, sem
alegação de revisão independente, validação física ou integração à `main`.
