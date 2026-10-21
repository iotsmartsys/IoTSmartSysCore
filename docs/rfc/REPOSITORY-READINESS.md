# Repository Readiness — recorte AVANT

**Classe da fonte:** Operacional

**Estado:** Ready exclusivamente no escopo `AVIR-INTERPRETER`

**Repositório:** `iotsmartsys/IoTSmartSysCore`

**Contrato e revisão:** [REPOSITORY-ENGINEERING-CONTRACT.md](REPOSITORY-ENGINEERING-CONTRACT.md),
0.1, aprovado por Marcelo Miranda em `AVIR-ENG-DEC-001`

**Avaliação vigente:** [avaliação AVANT 0.1](../reports/repository-readiness/2026-10-21T025824Z-0.1-1828a725-6b44-4573-8353-3df2fc27cf2f-repository-readiness.md)

**Decisão humana de habilitação:** `AVIR-ENG-DEC-002`, Marcelo Miranda,
Arquiteto, confirmação nesta sessão registrada em 2026-10-21T03:03:35Z:
"Confirmo a habilitação de AVIR-INTERPRETER conforme contrato v0.1 e avaliação
apresentada, sem estender a outros escopos."

## Escopos

| Escopo e dependências materiais | Estado da avaliação | Bloqueadores ou limites | Decisão humana e alcance |
|---|---|---|---|
| `AVIR-INTERPRETER`: interpreter AVANT, constantes, enum, interfaces preservadas, documentação e build `esp32_dev`, conforme especificação 0.1 | Ready | Sem bloqueador técnico identificado | Habilitado por Marcelo Miranda em `AVIR-ENG-DEC-002`, somente neste alcance. |
| Demais funcionalidades, platforms, environments e serviços | Não avaliados / não habilitados | Fora do contrato; não extrapolar o resultado AVANT | Nenhuma habilitação concedida. |

Não se declara `Ready` global do IoTSmartSysCore. O resultado técnico do recorte
não substitui sua habilitação humana nem a ordem explícita para implementar
uma versão com análise funcional `Ready`.

Contrato aprovado, avaliação válida e habilitação do recorte estão registrados.
As recusas anteriores por ausência de qualificação permanecem históricas; essa
condição de entrada foi suprida sem dispensa e sem alterar a análise funcional.
Esta entrega é documental e não inicia a implementação.

## Validade e reavaliação

Baseline técnica: `b72778c7e07819a0d212d8237cbd35078a03e161`.
Contrato aprovado: versão 0.1, identificado também por hash na avaliação.
Especificação: `IOTSSC-AVANT-LS963231305-IR-COMMAND@0.1`.

Nova avaliação é exigida após mudança material de contrato, comportamento,
interfaces, dependências, build ou alcance. Aprovação revogada ou evidência
superada suspende a habilitação aplicável. A simples execução de uma tarefa
conforme não exige repetir aprovação vigente.

A qualificação é específica do recorte e não reclassifica achados fora dele,
incluindo a exposição preexistente de credenciais em outro environment.
Nenhuma operação de hardware, teste, release, merge ou deploy é habilitada por
este documento. Relatórios anteriores são históricos e não devem ser alterados.
