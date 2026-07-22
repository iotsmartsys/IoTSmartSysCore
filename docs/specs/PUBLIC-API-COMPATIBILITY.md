# Especificação — Compatibilidade da API Pública

**ID:** IOTSSC-PUBLIC-API

**Estado normativo:** Active

**Estado de implementação:** Implemented

**Última atualização:** 22/07/2026

## 1. Objetivo

Preservar a API pública usada pelos firmwares Arduino sobre ESP32 e tornar explícitos seus contratos de configuração e compatibilidade.

## 2. Superfície pública

São API pública retrocompatível:

- `SmartSysApp`;
- `SensorFactory`;
- interfaces públicas de sensores;
- tipos públicos de configuração;
- tipos necessários para construir capabilities pelo padrão vigente.

`src/main.cpp` é o exemplo oficial de consumo e o executável de validação em hardware.

## 3. Requisitos

- **API-001:** o runtime atualmente suportado deve continuar funcionando em Arduino sobre ESP32.
- **API-002:** nomes, assinaturas, tipos, defaults e semântica observável da superfície pública não podem sofrer quebra silenciosa.
- **API-003:** o padrão de consumo deve permanecer `construir → configurar → app.setup() → app.handle()`.
- **API-004:** toda configuração e adição de capabilities deve terminar antes de `app.setup()`; adições posteriores não são suportadas.
- **API-005:** o limite máximo de oito capabilities por aplicação é uma regra intencional.
- **API-006:** falha de capacidade ou alocação em builders deve ser observável pelo contrato atual e não pode resultar em corrupção ou ownership ambíguo.
- **API-007:** objetos retornados por builders internos da aplicação permanecem sob ownership da aplicação; objetos criados por `SensorFactory` seguem o ownership expresso por sua API.
- **API-008:** `SmartSysApp`, `SensorFactory`, interfaces e configs devem evoluir de forma compatível; quebra exige especificação aprovada e estratégia explícita de migração/versionamento.
- **API-009:** preparação interna para ESP-IDF não pode degradar o runtime Arduino vigente.
- **API-010:** ESP8266 não faz parte da plataforma suportada e sua presença no repositório não constitui contrato público ativo.

## 4. Fora de escopo

- declarar suporte completo a ESP-IDF nativo;
- remover código ESP8266;
- redesenhar a API;
- aumentar o limite de capabilities.

## 5. Critérios de aceite

- inventário da superfície pública reconciliado com headers;
- exemplo `src/main.cpp` compilável;
- testes representativos de builders, ownership, limite de oito slots e falhas;
- validação em hardware do fluxo público quando houver mudança comportamental.

## 6. Estado conhecido

A implementação foi identificada no legado, mas ainda não existe uma matriz completa de compatibilidade. Por isso o estado é `Implemented`, não `Validated`.

## 7. Relações

- `CORE-RUNTIME-LIFECYCLE.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `EKM-GAP-0001`.
