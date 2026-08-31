# Instruções permanentes e roteamento EKOM

**Modelo EKOM:** 4.7

**Modalidade:** capacidades referenciadas e governança proporcional

**Estado:** vigente

## Autoridade

O Arquiteto humano tem autoridade final sobre intenção, prioridade, escopo,
arquitetura, risco aceitável, relevância das críticas, suficiência das
evidências, aprovação, conclusão ou reabertura e integração. A especificação é
a fonte da verdade para comportamento e governa a execução dos agentes.

## Fonte dos perfis

**Raiz do EKOM:**
`/Volumes/NVME/Developer/Source/EKM-guidelines`

Antes de qualquer atuação EKOM:

1. leia integralmente `roles/REGRAS-COMUNS.md`;
2. leia integralmente somente o perfil correspondente à capacidade recebida;
3. leia a especificação indicada, quando aplicável;
4. leia somente as fontes técnicas pertinentes.

| Capacidade recebida | Perfil |
|---|---|
| Autor da Especificação | `roles/AUTOR-DA-ESPECIFICACAO.md` |
| Engenheiro Analista | `roles/ENGENHEIRO-ANALISTA.md` |
| Engenheiro Implementador | `roles/ENGENHEIRO-IMPLEMENTADOR.md` |
| Crítico ou Engenheiro Revisor | `roles/ENGENHEIRO-REVISOR.md` |
| Consultor de Arquitetura | `roles/CONSULTOR-DE-ARQUITETURA.md` |

Análise de implementabilidade é obrigatória antes da implementação, mas pode
ser feita pelo Autor, pelo Autor apoiado por IA, por agente especializado ou
por especialista separado. Revisão é o quarto estágio; profundidade,
independência e challenge adicional são proporcionais ao risco. A ordem pode
combinar autoria e análise, mas deve declarar segregação quando necessária.

Implementabilidade é avaliada dentro da baseline e do recorte. Capacidade
arquitetural ausente, independente e transversal bloqueia a funcionalidade e
exige decisão do Arquiteto sobre análise e especificação preparatória.

Prontidão exige ao menos uma implementação tecnicamente plausível e conforme,
não uma solução interna completa. Escolhas locais de engenharia e evidências
produzidas durante Implementação ou Revisão são não bloqueantes, salvo quando
forem necessárias para decidir se qualquer implementação conforme é possível.

Autoridade normativa se limita a comportamentos, garantias e restrições
explicitamente declarados. Extensão aditiva presume-se não interferente;
arquivo, classe, componente, dependência ou inventário compartilhado não criam
emenda nem bloqueio.

Análise formal reconcilia bloqueadores anteriores aplicáveis, declara cobertura
de requisitos, critérios e débitos relacionados, preserva até cinco restrições
materiais não bloqueantes e executa challenge limitado antes de `Ready`.
Parecer somente em chat não estabelece `Ready`; o relatório precisa ser
persistido em `docs/reports/`.

Implementação exige análise `Ready` da versão corrente e ordem explícita do
Arquiteto. Não existe promoção ou campo documental intermediário. Satisfeita a
entrada, a implementação de artefato construível inclui seu build canônico
proporcional, mas não autoriza testes, hardware, deploy ou operação externa.

Criação, ampliação, reestruturação ou correção de testes só integra a
implementação quando a especificação corrente exigir explicitamente e vincular
o teste a requisito ou critério de aceite. Criar teste não autoriza executá-lo.

Toda atuação autorizada que produza mudança material inclui commit e push da
branch de trabalho corrente e termina com árvore limpa. Force push, merge, tag,
release, deploy, exclusão de branch e reescrita de histórico exigem ordem
específica.

Trabalho governado por especificação principal usa branch derivada do nome do
documento: `docs/specs/<Nome>.md` corresponde a `spec/<nome-em-minúsculas>`.
Mudança de governança sem especificação usa branch acordada com o Arquiteto.

## Fontes locais do projeto

- diretrizes locais vigentes: `docs/rfc/EKOM-GUIDELINES.md`;
- especificações: `docs/specs/`;
- ADRs: `docs/adr/`; RFCs e diretrizes: `docs/rfc/`;
- relatórios: `docs/reports/`;
- transações EKOM vigentes: `docs/rfc/EKOM-CHANGELOG.md`;
- histórico legado EKM: `docs/rfc/EKM-CHANGELOG.md`;
- mapa de conhecimento: `docs/rfc/KNOWLEDGE-MAP.md`;
- visão e navegação informativa: `README.md` e `docs/REPO_DOSSIER.md`;
- runtime e API pública: `src/SmartSysApp.*`, `src/main.cpp` e `src/Core/`;
- integrações de plataforma: `src/Platform/`;
- build e environments: `platformio.ini`, `configs/`, `boards/` e `Makefile`;
- testes PlatformIO/Unity: `test/`.

## Comandos canônicos

- integridade textual: `git diff --check`;
- guarda estrutural EKOM: `python3 /Volumes/NVME/Developer/Source/EKM-guidelines/templates/tools/validate_ekom_documents.py .`;
- build suportado: `pio run -e esp32_dev`;
- testes PlatformIO/Unity: `pio test -e esp32s3_test`.

Validações adicionais pertencem à especificação aplicável. Upload em hardware,
publicação, release e deploy exigem ordem explícita do Arquiteto.

## Invariantes locais

- Preserve arquitetura, organização e separação de responsabilidades; use o
  precedente equivalente mais próximo. Desvio exige decisão arquitetural
  explícita.
- `main` é a referência de produção; o runtime suportado é Arduino sobre ESP32.
- Preserve APIs públicas, defaults e comportamentos normativos, salvo mudança
  explicitamente aprovada em especificação.
- Configure capabilities antes de `SmartSysApp::setup()`, preserve o
  processamento cooperativo de `handle()` e o limite de oito capabilities.
- ESP-IDF permanece preparação futura e não pode degradar o runtime vigente;
  ESP8266 não constitui plataforma suportada.
- Não altere `private.ini`, credenciais, secrets ou configuração privada.
- Preserve alterações preexistentes e não relacionadas.
- Testes são evidências, não prova absoluta; não os altere apenas para obter
  verde nem os use como argumento autorreferente.
- Análise, implementação, revisão e evidência operacional produzem registros
  separados; somente o Arquiteto incorpora achados em fontes normativas, aceita
  ADRs e determina conclusão ou reabertura.

> **Specifications orchestrate. Code implements.**
