# Instrução experimental — Engenheiro Implementador do controle de garagem

Use todo o conteúdo abaixo como uma única instrução para o agente executor.
Esta instrução é autocontida quanto às regras EKM aplicáveis à implementação.
O executor não precisa carregar `EKM-guidelines` nem reler
`docs/rfc/EKM-GUIDELINES.md`.

---

## Ordem do Arquiteto

Execute exclusivamente a etapa de implementação da especificação
`IOTSSC-GARAGE-CONTROL@0.1` no repositório:

`/Users/marcelocostamiranda/source/IoT/SmartHome/IoTSmartSysCore`

Branch designada: `fix/estado_do_garage_control_incosistente`.

Implemente, teste e reconcilie o conhecimento afetado. Não realize merge,
release, deploy ou validação física em nome do Arquiteto.

## Autoridade e responsabilidade

O Arquiteto humano é a autoridade final sobre intenção, escopo, arquitetura,
risco, validação e integração. Esta ordem autoriza apenas a implementação do
contrato abaixo.

- Não invente comportamento, requisito ou decisão arquitetural.
- Não amplie o escopo para refatorações ou correções adjacentes.
- Não altere requisitos para acomodar a implementação existente.
- Não converta build, teste ou validação falha em evidência aprovada.
- Se surgir uma decisão normativa ausente, interrompa o recorte afetado e
  devolva a decisão ao Arquiteto.

## Condições obrigatórias de entrada

Antes da primeira alteração:

1. confirme que está na branch designada;
2. confirme que a árvore de trabalho está limpa;
3. confirme que `docs/specs/GARAGE-CONTROL-STATE.md` permanece na versão 0.1,
   com revisão de implementabilidade `Implementable`;
4. confirme que `EKM-CHG-0007` permanece a transação relacionada.

Se qualquer condição falhar, não inicie a implementação e informe o bloqueio.

## Resultado pretendido

Corrigir a máquina de estados da `GarageControlCapability` para publicar a
posição física confirmada e o movimento observado, sem manter indefinidamente a
intenção de um comando contra a evidência dos sensores.

## Escopo autorizado

- leitura e debounce dos sensores de abertura e fechamento;
- máquina de estados `unknown`, `opened`, `closed`, `opening` e `closing`;
- efeito de `open` e `close` sobre a direção do movimento;
- movimento iniciado por comando ou externamente;
- configuração `sensorDebounceTimeMs`;
- compatibilidade da API pública e dos builders atuais;
- testes automatizados da máquina de estados;
- atualização da especificação, changelog e mapa de conhecimento afetados.

Arquivos esperados no recorte:

- `src/Core/Capabilities/GarageControlCapability.cpp`;
- `src/Contracts/Capabilities/GarageControlCapability.h`;
- `src/App/Builders/Configs/CapabilityConfig.h`;
- `src/App/Builders/Builders/CapabilitiesBuilder.cpp`;
- novos testes e mocks estritamente necessários em `test/`;
- `docs/specs/GARAGE-CONTROL-STATE.md`;
- `docs/rfc/EKM-CHANGELOG.md`;
- `docs/rfc/KNOWLEDGE-MAP.md`.

Alterações fora dessa relação exigem necessidade técnica diretamente causada
por GAR-001 a GAR-020 e devem ser explicadas no resultado.

## Fora de escopo

- alterar a sequência elétrica dos relés;
- alterar `lock`, `unlock`, `stop` ou `stop_unlock`;
- criar telemetria, timeout ou estados novos de falha;
- detectar velocidade, obstrução ou sentido com sensores adicionais;
- alterar MQTT, persistência ou protocolo de comandos;
- alterar o limite de capabilities ou o ciclo de vida do `SmartSysApp`;
- redesenhar capabilities, adapters, builders ou a API pública;
- modificar `private.ini`, credenciais ou secrets.

## Contrato funcional integral

- **GAR-001:** sensores de fim de curso continuam em `PULL_UP`; nível `LOW`
  significa extremo ativo.
- **GAR-002:** uma mudança de sensor só afeta o estado após permanecer estável
  pelo intervalo de debounce.
- **GAR-003:** `GarageControlConfig` oferece `sensorDebounceTimeMs`, default 50
  ms. `debounceTimeMs` conserva exclusivamente a duração do pulso dos relés.
- **GAR-004:** somente fechamento ativo e estável publica `closed`.
- **GAR-005:** somente abertura ativa e estável publica `opened`.
- **GAR-006:** `open` com fechamento ainda ativo aciona o relé, mas conserva
  `closed` até a liberação estável do sensor.
- **GAR-007:** depois da liberação estável do fechamento, sem extremo ativo, o
  estado passa para `opening`.
- **GAR-008:** `close` com abertura ainda ativa aciona o relé, mas conserva
  `opened` até a liberação estável do sensor.
- **GAR-009:** depois da liberação estável da abertura, sem extremo ativo, o
  estado passa para `closing`.
- **GAR-010:** durante `opening`, abertura ativa e estável produz `opened`; o
  retorno estável ao fechamento produz `closed`.
- **GAR-011:** durante `closing`, fechamento ativo e estável produz `closed`; o
  retorno estável à abertura produz `opened`.
- **GAR-012:** a liberação estável de `closed` infere movimento externo
  `opening`; a liberação estável de `opened` infere `closing`.
- **GAR-013:** com ambos os extremos inativos, `open` define `opening` e `close`
  define `closing`.
- **GAR-014:** comando oposto durante o percurso atualiza a direção solicitada,
  sem impedir que um extremo estável determine o estado terminal.
- **GAR-015:** oscilações menores que o debounce não publicam estados
  intermediários nem invertem a direção inferida.
- **GAR-016:** ambos os sensores ativos e estáveis produzem `unknown`; nenhum
  extremo possui precedência.
- **GAR-017:** após inicialização, publique `closed` ou `opened` quando a
  combinação correspondente permanecer estável pelo debounce. Com ambos
  inativos ou ambos ativos, permaneça `unknown` até haver evidência suficiente.
- **GAR-018:** com sensores ausentes ou parciais, use somente os extremos
  observáveis e os comandos; não fabrique confirmação de extremo ausente.
- **GAR-019:** preserve nomes de comandos e estados, duração dos pulsos,
  ownership, limite de capabilities e compatibilidade de código-fonte.
- **GAR-020:** publique evento somente quando o estado lógico mudar; leituras
  estáveis repetidas não geram duplicidade.

## Fluxos e bordas obrigatórios

Fluxo comandado a partir de `closed`:

```text
closed
→ comando open e pulso no relé
→ closed enquanto o fim de curso fechado estiver ativo
→ liberação estável do fechamento
→ opening
→ ativação estável da abertura
→ opened
```

O fechamento é simétrico. O retorno ao extremo de origem deve funcionar:

```text
closed → opening → closed
opened → closing → opened
```

Sensores ausentes:

- sem sensores, comandos publicam `opening` ou `closing`, sem término
  fabricado;
- somente abertura pode confirmar `opened`, nunca `closed`;
- somente fechamento pode confirmar `closed`, nunca `opened`.

Inicialização no meio do percurso começa em `unknown`. Falha de partida mantém
o estado terminal observado. Dois extremos estáveis simultâneos produzem
`unknown`.

## Restrições de engenharia aplicáveis

- Preserve Arduino sobre ESP32, o limite de oito capabilities e o processamento
  cooperativo de `handle()`.
- Mantenha `GarageControlCapability` responsável por sua máquina de estados;
  não crie task, singleton, serviço global ou nova camada para este recorte.
- Use `ICapability::timeProvider` para o debounce determinístico dos sensores.
  Não use `delay()` para debounce de leitura; o `delay(debounceTimeMs)` existente
  continua restrito ao pulso dos relés.
- Mantenha estados bruto e estável de cada sensor separados. Uma leitura bruta
  não pode publicar transição antes do debounce.
- Preserve a publicação centralizada por mudança lógica em `handle()`; não
  publique repetidamente o mesmo estado.
- Preserve os adapters de entrada criados pelo builder com `PULL_UP`.
- Adicione `sensorDebounceTimeMs` com default compatível de 50 ms e propague-o
  pelo builder sem mudar o significado de `debounceTimeMs`.
- Preserve consumidores atuais do construtor de `GarageControlCapability` por
  overload compatível ou parâmetro final com default, escolhendo a menor mudança
  coerente com os padrões já existentes no código.
- Não altere ownership dos adapters ou da capability.
- Prefira nomes que distingam leitura bruta, estado estável, instante da última
  mudança e direção solicitada.
- Não introduza abstração ou design pattern sem necessidade demonstrada pelos
  requisitos deste recorte.

## Referências técnicas autorizadas

Inspecione somente o necessário para implementar corretamente:

- `src/Core/Capabilities/GarageControlCapability.cpp`;
- `src/Contracts/Capabilities/GarageControlCapability.h`;
- `src/App/Builders/Configs/CapabilityConfig.h`;
- `src/App/Builders/Builders/CapabilitiesBuilder.cpp`;
- `src/Contracts/Capabilities/ICapability.h` para `timeProvider` e publicação;
- `src/Contracts/Adapters/IInputHardwareAdapter.h`;
- `src/Contracts/Adapters/ICommandHardwareAdapter.h`;
- `src/Contracts/Events/ICapabilityEventSink.h`;
- testes Unity existentes como referência de estrutura, não de comportamento.

Você pode localizar dependências diretas adicionais no código. Não precisa ler
a metodologia EKM completa nem documentos históricos não relacionados.

## Testes obrigatórios

Adicione testes PlatformIO/Unity que comprovem, no mínimo:

1. default e separação entre debounce dos sensores e pulso dos relés;
2. inicialização nas quatro combinações dos sensores;
3. `closed → opening → opened` e o fluxo simétrico;
4. falha de partida, mantendo o extremo de origem;
5. retorno ao extremo de origem durante o percurso;
6. movimento externo a partir de `closed` e `opened`;
7. comando com ambos os extremos inativos;
8. reversão de direção durante o percurso;
9. bounce menor que 50 ms e mudança estável maior ou igual ao debounce;
10. ambos os sensores ativos produzindo `unknown`;
11. ausência dos dois sensores e presença de apenas cada um deles;
12. ordem dos eventos e ausência de publicações duplicadas.

Use provider de tempo controlável e mocks de input, output e event sink. Não
dependa de espera real para testar debounce.

## Validações da etapa

Execute:

```text
git diff --check
pio run -e esp32_dev
pio test -e esp32s3_test
```

Se o ambiente de teste depender de hardware indisponível, execute tudo que for
possível sem fabricar sucesso e registre precisamente a limitação. Build e
testes automatizados aprovados permitem declarar a implementação
`Implemented`. Somente validação física do Arquiteto permite `Validated`.

## Reconciliação EKM

Se código, build e testes automatizados forem aprovados:

- atualize `GARAGE-CONTROL-STATE.md` para implementação `Implemented`;
- registre na seção de evidências somente resultados materiais e limitações;
- mantenha a entrega `Not Ready` e não declare `Validated` ou `Done` sem a
  validação física do Arquiteto;
- atualize `EKM-CHG-0007` com decisões surgidas, evidências e resultado;
- mantenha `EKM-CHG-0007` aberta até validação física e decisão do Arquiteto;
- mantenha `EKM-GAP-0009` aberta enquanto faltar validação física;
- atualize o estado correspondente no `KNOWLEDGE-MAP.md` sem apagar a lacuna.

Não registre SHAs, branch de origem, checkpoints ou diário de comandos nos
documentos EKM. O Git já preserva essa linhagem.

## Entrega Git obrigatória

Ao concluir a tarefa iniciada:

1. revise o diff completo e confirme ausência de alterações fora do recorte;
2. crie um commit material com código, testes e conhecimento reconciliado;
3. faça push para `fix/estado_do_garage_control_incosistente`;
4. confirme que a árvore terminou limpa.

Não use commit vazio. Não execute force push, reescrita de histórico, merge,
tag, release ou deploy.

## Resposta final esperada

Informe de forma objetiva:

- comportamento implementado;
- arquivos e contratos afetados;
- testes e builds executados, com seus resultados reais;
- validação física ainda pendente, se aplicável;
- decisões ausentes ou limitações;
- estados finais da especificação, `EKM-CHG-0007` e `EKM-GAP-0009`;
- confirmação de commit, push e árvore limpa.
