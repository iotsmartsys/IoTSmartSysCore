# EKOM — Diretrizes locais

**Classe da fonte:** Normativa

**Estado da fonte:** Vigente

**Versão do documento:** 4.6

**Versão do modelo EKOM:** 4.6

**Escopo:** Todo o repositório

## 1. Autoridade e fonte aplicável

O EKOM 4.6 vigente em
`/Volumes/NVME/Developer/Source/EKM-guidelines` governa o método. O Arquiteto
mantém autoridade sobre intenção, arquitetura, risco, suficiência das
evidências, conclusão, reabertura e integração.

Estas diretrizes contêm somente adaptações locais. O histórico produzido sob
EKM 1.x permanece factual e consultável em `docs/rfc/EKM-GUIDELINES.md` e
`docs/rfc/EKM-CHANGELOG.md`, mas não governa novas atuações.

## 2. Workflow vigente

```text
Autoria → Análise de Implementabilidade → Implementação → Revisão
```

- a Autoria usa investigação dirigida e rascunho conversacional confirmado
  antes da escrita normativa de especificação nova ou revisão material;
- a implementação exige relatório `Ready` da versão normativa corrente e
  ordem explícita do Arquiteto;
- `Ready` usa suficiência, não exaustão, dentro da baseline e do recorte;
- capacidade arquitetural ausente, independente e transversal é pré-requisito
  separado, não detalhe absorvido pela funcionalidade;
- o build canônico proporcional integra a implementação de artefato
  construível;
- testes só integram o recorte quando a especificação os exige e não são
  executados sem permissão operacional;
- Revisão é o quarto estágio, com profundidade e independência proporcionais ao
  risco;
- somente o Arquiteto determina `Done`, reabertura e integração.

Estados, pareceres e transações encerrados sob EKM 1.x não são reinterpretados
retroativamente. Toda nova atuação ou revisão material aplica EKOM 4.6.

## 3. Roteamento documental

- especificação: comportamento, limites, estados e critérios de aceite;
- ADR ou RFC: decisão arquitetural durável e consequências;
- relatório: fatos e evidências de uma execução;
- mapa: autoridade, localização, relações, lacunas e débitos aceitos;
- changelog EKOM: estado resumido de transações iniciadas após esta migração;
- changelog EKM legado: histórico imutável das transações anteriores;
- Git: autoria, diferenças, branch e linhagem técnica.

Relatórios concluídos permanecem históricos. Correções factuais usam novo
relatório ou adendo relacionado. Especificações concluídas são preservadas;
mudança posterior usa relação normativa explícita ou reabertura decidida pelo
Arquiteto.

## 4. Conhecimento e débito técnico

O mapa mantém índice, árvore e diagrama conforme os gatilhos do EKOM 4.6.
Lacuna representa conhecimento necessário ausente. Débito técnico exige
postergação explícita pelo Arquiteto, identidade `EKOM-DEBT-NNNN`, consequência
e gatilho ou critério objetivo de quitação. Achado ou risco não se torna débito
por inferência do agente.

Os identificadores `EKM-CHG-*` e `EKM-GAP-*` existentes são preservados. Novas
transações, lacunas e débitos usam o namespace `EKOM`.

## 5. Git e entrega

Atuações começam em branch derivada da `main` e com árvore limpa. Mudança
material autorizada inclui commit e push da branch corrente e termina com
árvore limpa. Trabalho governado por especificação usa a branch previsível
derivada do nome do documento. Force push, merge, tag, release, deploy,
exclusão de branch e reescrita de histórico exigem ordem específica.

## 6. Regras específicas do projeto

- `main` é a referência de produção.
- O runtime suportado é Arduino sobre ESP32.
- ESP-IDF permanece preparação futura e não pode degradar o runtime vigente.
- ESP8266 não constitui plataforma suportada.
- APIs públicas, defaults e comportamentos normativos são preservados salvo
  mudança explicitamente aprovada em especificação.
- Capabilities são configuradas antes de `SmartSysApp::setup()`; o runtime
  preserva processamento cooperativo e o limite de oito capabilities.
- Mudanças funcionais relevantes seguem *specification on touch*.
- `private.ini`, credenciais, secrets e configuração privada não são alterados.
- Não existe garantia automatizada EKOM implantada no repositório.

## 7. Validações canônicas

- integridade textual: `git diff --check`;
- guarda estrutural EKOM: script oficial
  `templates/tools/validate_ekom_documents.py` da fonte EKOM 4.6;
- build suportado: `pio run -e esp32_dev`;
- testes PlatformIO/Unity: `pio test -e esp32s3_test`.

Build e testes só se aplicam ao recorte e às permissões operacionais da
atuação. Upload, hardware, publicação, release e deploy exigem ordem explícita.
