# Instruções permanentes e roteamento EKM

**Modelo EKM:** 1.19

**Modalidade:** atores com perfis referenciados

**Estado:** vigente

## Autoridade

O Arquiteto humano tem autoridade final sobre intenção, prioridade, escopo,
arquitetura, risco, autorização, validação e integração. A ordem recebida por
prompt ou pipeline identifica papel, especificação e recorte autorizado.

## Fonte dos perfis

**Raiz local da EKM:**
`/Users/marcelocostamiranda/source/EKM-guidelines`

Antes de qualquer atuação EKM:

1. leia integralmente
   `/Users/marcelocostamiranda/source/EKM-guidelines/roles/REGRAS-COMUNS.md`;
2. leia integralmente somente o perfil correspondente ao papel recebido;
3. leia a especificação indicada, quando aplicável;
4. leia apenas as fontes técnicas pertinentes ao recorte.

| Papel recebido | Perfil |
|---|---|
| Autor da Especificação | `roles/AUTOR-DA-ESPECIFICACAO.md` |
| Engenheiro Analista | `roles/ENGENHEIRO-ANALISTA.md` |
| Engenheiro Implementador | `roles/ENGENHEIRO-IMPLEMENTADOR.md` |
| Engenheiro Revisor | `roles/ENGENHEIRO-REVISOR.md` |
| Consultor de Arquitetura | `roles/CONSULTOR-DE-ARQUITETURA.md` |

Não carregue perfis de outros papéis nem a metodologia EKM completa. Se a ordem
não identificar papel, resultado e recorte, ou se a fonte não estiver
acessível, não inicie a tarefa; informe o impedimento ao Arquiteto. A
especificação é obrigatória no ciclo funcional; o Consultor pode receber
Não se aplica [`Not Applicable`] em governança ou apoio fora desse ciclo.

## Fontes locais do projeto

- diretrizes locais: `docs/rfc/EKM-GUIDELINES.md`;
- especificações: `docs/specs/`;
- decisões, evidências e transações: `docs/rfc/EKM-CHANGELOG.md`;
- mapa de conhecimento: `docs/rfc/KNOWLEDGE-MAP.md`;
- visão e navegação informativa: `README.md` e `docs/REPO_DOSSIER.md`;
- runtime e API pública: `src/SmartSysApp.*`, `src/main.cpp` e `src/Core/`;
- integrações de plataforma: `src/Platform/`;
- build e environments: `platformio.ini`, `configs/`, `boards/` e `Makefile`;
- testes PlatformIO/Unity: `test/`.

## Comandos canônicos

- integridade textual: `git diff --check`;
- build suportado: `pio run -e esp32_dev`;
- testes PlatformIO/Unity: `pio test -e esp32s3_test`.

Validações adicionais pertencem à especificação aplicável. Upload em hardware,
publicação, release e deploy exigem ordem explícita do Arquiteto. Não declare
como aprovada uma validação que não foi executada ou não chegou a estado
terminal.

## Invariantes locais

- Preserve arquitetura, organização e separação de responsabilidades vigentes;
  use as fontes técnicas acima e o precedente equivalente mais próximo. Desvio
  exige autorização arquitetural explícita na especificação.
- `main` é a referência de produção; o runtime suportado é Arduino sobre ESP32.
- Preserve APIs públicas, defaults e comportamentos normativos, salvo mudança
  explicitamente aprovada em especificação.
- Configure capabilities antes de `SmartSysApp::setup()`, preserve o
  processamento cooperativo de `handle()` e o limite de oito capabilities.
- ESP-IDF permanece preparação futura e não pode degradar o runtime vigente;
  ESP8266 não constitui plataforma suportada.
- Não altere `private.ini`, credenciais, secrets ou configuração privada.
- Preserve alterações preexistentes e não relacionadas.

As regras comuns e o perfil selecionado definem condições de entrada, promoção
de estados, evidência, Git e encerramento. Regras específicas da tarefa
pertencem à especificação ou à ordem do Arquiteto, não a este arquivo.
