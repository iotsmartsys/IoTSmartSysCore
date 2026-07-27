# Instruções permanentes do repositório

Este arquivo contém somente invariantes válidas para qualquer tarefa. A ordem
do Arquiteto fornece etapa, escopo, especificação e regras particulares.

## Autoridade e escopo

- O Arquiteto humano decide intenção, arquitetura, risco e integração.
- Execute somente a etapa e o recorte recebidos por prompt ou pipeline.
- Não invente requisitos, não amplie escopo nem falsifique evidências.
- Preserve alterações preexistentes e não relacionadas.

## Fontes locais

- especificações normativas: `docs/specs/`;
- decisões, lacunas e evidências materiais: `docs/rfc/EKM-CHANGELOG.md`;
- localização das fontes e lacunas: `docs/rfc/KNOWLEDGE-MAP.md`;
- implementação: `src/`;
- testes PlatformIO/Unity: `test/`.

Uma instrução declarada autocontida fornece as regras EKM e de engenharia da
tarefa e dispensa reler `docs/rfc/EKM-GUIDELINES.md`.

## Invariantes técnicas

- `main` é a referência de produção.
- O runtime suportado é Arduino sobre ESP32.
- Preserve APIs públicas, defaults e comportamentos normativos, salvo mudança
  explicitamente aprovada na especificação.
- Configure capabilities antes de `SmartSysApp::setup()`, preserve o
  processamento cooperativo de `handle()` e o limite de oito capabilities.
- Não altere `private.ini`, credenciais, secrets ou configuração privada.

## Validação

Use como verificações canônicas, quando aplicáveis ao recorte:

- `git diff --check`;
- `pio run -e esp32_dev`;
- `pio test -e esp32s3_test` para testes PlatformIO/Unity.

Não aprove validação não executada; registre limitações reais.

## Git e entrega

- Comece com árvore de trabalho limpa; se não estiver limpa, não inicie.
- Toda tarefa iniciada deve terminar com resultado material, commit, push e
  árvore limpa.
- O Git mantém a linhagem; não a duplique em documentos EKM.
- Não execute force push, reescrita de histórico, merge, tag, release ou deploy
  sem ordem específica do Arquiteto.
