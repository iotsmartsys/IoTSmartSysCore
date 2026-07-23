# EKM Changelog — IoTSmartSysCore

## EKM-CHG-0001 — Fundação EKM para o legado

**Estado:** Closed

**Data:** 22/07/2026

### Objetivo

Instituir a governança mínima EKM, registrar o baseline do projeto e criar fontes normativas iniciais para API pública, runtime e release.

### Baseline

- Branch `main`.
- Commit `0c6d5e63eb09d826beba2e16a3085c1a8f814668`.
- Worktree inicial limpo.

### Fontes criadas

- `AGENTS.md`;
- `docs/rfc/EKM-GUIDELINES.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`;
- `docs/specs/PUBLIC-API-COMPATIBILITY.md`;
- `docs/specs/CORE-RUNTIME-LIFECYCLE.md`;
- `docs/specs/RELEASE-AND-DISTRIBUTION.md`.

### Restrições

Nenhum código, build, workflow, teste ou processo de release deve ser alterado nesta transação.

### Validações requeridas

- referências internas e metadados conferidos;
- estados reconciliados com o mapa de conhecimento;
- contratos de API e limite de oito capabilities confrontados com `SmartSysApp.h`;
- divergência do header de versão confirmada no `Makefile` e no GitHub Actions;
- `git diff --check`: aprovado;
- nenhuma alteração fora dos sete ativos EKM aprovados.

### Resultado

A fundação EKM foi instituída sem modificar código, build, workflow, testes ou comportamento. As lacunas descobertas permanecem `Open` no mapa e não impedem o encerramento desta transação documental.

## EKM-CHG-0002 — Especificação dos exemplos executáveis

**Estado:** Open

**Data:** 22/07/2026

### Objetivo

Especificar um catálogo de exemplos reais das capabilities e funcionalidades públicas, selecionáveis por environment PlatformIO e utilizáveis em validação de hardware.

### Baseline

- Branch `main`.
- Worktree inicial limpo.
- `src/main.cpp` é a aplicação padrão e executável atual de hardware.
- Existem exemplos em `examples/`, mas sem seleção uniforme por environment.

### Escopo da fase documental

- criar `EXECUTABLE-HARDWARE-EXAMPLES.md`, submetê-la à decisão humana e promovê-la para `Active` após aprovação;
- registrar o domínio e a lacuna de decisões no mapa;
- não alterar código, PlatformIO, exemplos existentes, testes ou automações.

### Decisões aprovadas

- placa canônica: `iotsmartsys_mcb_r1`;
- exemplos iniciais: `basic_light` e `environment_dht`;
- serviços externos: infraestrutura real somente por configuração privada existente;
- CI: build sem upload dos environments `example_basic_light_mcb_r1` e `example_environment_dht_mcb_r1`.

### Estado documental

- `EXECUTABLE-HARDWARE-EXAMPLES.md`: `Active` / `Implemented`;
- `EKM-GAP-0006`: `Closed`;
- primeiro recorte implementado com runner único, perfil MCB R1, exemplos `basic_light` e `environment_dht`, environments estáveis e matriz de CI;
- o build padrão foi preservado e sua flag de LED foi reconciliada com o `src/main.cpp` legado.

### Requisitos implementados

- `HWEX-001` a `HWEX-017`: atendidos no recorte aplicável por aplicações Arduino completas, seleção em build time, configuração explícita, documentação de hardware, CI e preservação da API pública;
- `HWEX-DEC-001` a `HWEX-DEC-004`: materializados sem incorporar credenciais ou portas locais.

### Evidências da implementação

- resolução dos environments por `pio project config --json-output`: aprovada;
- `pio run -e esp32_dev`: aprovado;
- `pio run -e example_basic_light_mcb_r1`: aprovado;
- `pio run -e example_environment_dht_mcb_r1`: aprovado;
- inspeção de símbolos dos dois firmwares: exatamente um `setup()` e um `loop()` em cada um;
- busca por indicadores de segredos no novo catálogo e configuração: nenhum resultado;
- `git diff --check`: aprovado.

### Validações pendentes

- upload e validação manual em MCB R1 de pelo menos um exemplo;
- confirmação em hardware do identificador/configuração no boot e dos estímulos documentados;
- execução da matriz no GitHub Actions após integração.

### Critério de encerramento

Implementação e validação dos requisitos da especificação, reconciliação das evidências e atualização do estado da implementação. A implementação automatizável foi concluída, mas a transação permanece `Open` até existir a evidência física exigida para promoção a `Validated`.

## EKM-CHG-0003 — Technical Readiness e atomicidade

**Estado:** Closed

**Data:** 22/07/2026

### Problema observado

Na implementação de `EXECUTABLE-HARDWARE-EXAMPLES.md`, o executor definiu `LED_BUILTIN=23` para preservar o build padrão. A decisão foi tecnicamente coerente, mas a especificação não autorizava explicitamente escolher entre essa solução e outras alternativas possíveis. A descoberta ocorreu durante a implementação, quando o fluxo já havia começado.

### Decisão

- toda especificação deve passar por Technical Readiness Review integral antes de qualquer alteração de implementação;
- o resultado é `Implementable` ou `Needs Clarification`;
- uma lacuna relevante bloqueia atomicamente todos os itens do recorte;
- a decisão ausente deve ser resolvida na especificação e a análise integral repetida;
- o executor não pode converter inferência relevante em implementação.

### Ativos alterados

- `AGENTS.md`;
- `docs/rfc/EKM-GUIDELINES.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`;
- `docs/rfc/EKM-CHANGELOG.md`.

### Validação

- consistência entre instrução de entrada, diretriz, mapa e histórico;
- nenhuma especificação funcional, código, build, teste ou automação alterados;
- `git diff --check` aprovado.

### Encerramento

A governança local foi promovida para o modelo EKM 1.4. Especificações existentes permanecem válidas, mas qualquer nova implementação ou retomada deve cumprir a análise de implementabilidade antes de alterar o repositório.
