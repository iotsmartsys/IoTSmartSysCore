# Especificação — SolarChargeController

**ID:** `IOTSSC-SOLAR-CHARGE-CONTROLLER`

**Classe da fonte:** Normativa

**Versão:** 0.1

**Estado normativo:** Rascunho [`Draft`]

**Estado da implementação:** Não iniciada [`Not Started`]

**Estado da entrega:** Pendente [`Pending`]

**Revisão de implementabilidade:** Pendente [`Pending`]

**Bloqueio arquitetural:** Nenhum conhecido antes da análise formal

**Relações normativas e de dependência:**

- Nova [`New`] — decisão lógica de habilitação do buck por qualidade da geração;
- preserva `PUBLIC-API-COMPATIBILITY.md`, `CORE-RUNTIME-LIFECYCLE.md` e
  `RUNTIME-CAPABILITY-CAPACITY.md`, sem modificar API ou lifecycle existentes;
- preserva `INA3221-SENSORS.md`, `VOLTAGE-SENSING-CAPABILITY.md`,
  `CURRENT-SENSING-CAPABILITY.md` e `POWER-ENERGY-CAPABILITY.md`: aquisição,
  calibração e publicação permanecem nos responsáveis atuais;
- não altera o exemplo `mcb01_solar_controller` nesta autoria.

## 1. Objetivo e contexto

Decidir quando habilitar, desabilitar e testar novamente o buck solar com base
na geração efetivamente aproveitável. Evitar desligamentos por oscilações
breves e permanência indefinida em um ponto de operação de baixa tensão.

Corrente entrando na bateria não basta para declarar geração saudável. A
avaliação combina tensão do painel, potência extraída do painel e corrente
líquida de carga da bateria. Tensão em circuito aberto permite tentar uma
recuperação; somente a validação sob carga confirma sua utilidade.

## 2. Escopo

- máquina de estados e duas fases internas do recovery probe;
- classificação `Healthy`, `Degraded` e `Weak`;
- critérios elétricos configuráveis, histerese e temporizações;
- contrato conceitual de entradas, saída e falhas críticas;
- lógica determinística e testável sem hardware.

Esta entrega registra o contrato. Não implementa o controller nem sua integração.

## 3. Fora de escopo

- controle de corrente constante/tensão constante (CC/CV), PWM ou setpoints;
- MPPT, busca do ponto de máxima potência ou garantia de recuperação ótima;
- algoritmo de carga por química, SOC, balanceamento ou substituição do BMS;
- GPIO, polaridade de EN, I²C, drivers, sensores ou placas concretas;
- novas capabilities, comandos MQTT, persistência, tarefas ou lifecycle global;
- alterações no firmware MCB01, builds, testes ou operação de hardware nesta
  autoria. Nenhum artefato automatizado de teste integra este recorte documental.

## 4. Contrato e responsabilidades

- **SCC-001:** o controller deve receber medições e tempo por parâmetros ou
  abstração substituível. Não acessa GPIO, sensores, relógio concreto, rede ou
  storage. A integração adquire as entradas e aplica a decisão ao EN do buck.
- **SCC-002:** entradas elétricas mínimas, em unidades físicas:

| Entrada | Unidade | Significado |
|---|---|---|
| `panelVoltage` | V | Tensão nos terminais do painel |
| `panelCurrent` | A | Positiva do painel para o buck |
| `batteryVoltage` | V | Tensão da bateria |
| `batteryChargeCurrent` | A | Corrente líquida positiva entrando na bateria; negativa em descarga |

`batteryChargeCurrent` não é a corrente total na saída do buck. A integração
normaliza a polaridade do sensor, inclusive quando o shunt mede no sentido
bateria → cargas. Calcula-se `panelPower = panelVoltage * panelCurrent`, em W,
sem usar valor absoluto para converter fluxo reverso em geração útil.

- **SCC-003:** o contrato deve também disponibilizar habilitação externa,
  indicação de falha crítica, reset explícito de falha, instante monotônico e
  validade/frescor do conjunto de medições. O adaptador deve rejeitar sentinels,
  valores não finitos e leituras sem coerência temporal. Limites de idade e
  defasagem das amostras pertencem à configuração de integração e devem ser
  definidos antes de operação real; não se presume que um número finito é válido.
- **SCC-004:** a saída conceitual mínima é:

```cpp
struct SolarChargeDecision {
    SolarChargeState state;
    bool buckEnabled;
};
```

A saída representa a decisão lógica, não confirmação física do atuador. Fase,
qualidade, motivo e idade do Voc podem ser observáveis para diagnóstico e
validação, sem criar requisito de transporte ou publicação.

- **SCC-005:** inicialização produz `Off` e `buckEnabled = false`, sem Voc
  válido. Cada atualização é limitada e não bloqueante; não usa `delay()` nem
  espera ativa. Mesmas entradas, histórico e tempo produzem a mesma decisão.

## 5. Estados

| Estado | Significado | `buckEnabled` |
|---|---|---|
| `Off` | Desabilitado externamente; nenhuma tentativa automática | `false` |
| `WaitingForRecovery` | Aguarda cooldown e tensão em circuito aberto suficiente | `false` |
| `TestingRecovery` | Recovery probe com estabilização sem carga e validação sob carga | Depende da fase |
| `Charging` | Geração saudável confirmada; tolera degradação breve enquanto a confirma | `true` |
| `Degraded` | Geração útil, mas sem qualidade saudável de tensão; duração limitada | `true` |
| `WeakGeneration` | Janela de confirmação de geração insuficiente; ainda permite recuperação | `true` |
| `Fault` | Falha crítica retida; retorno depende de remoção e reset explícito | `false` |

- **SCC-006:** qualidade da medição e estado operacional são distintos.
  `WeakGeneration` começa na detecção de `Weak`, antes de sua confirmação;
  somente a persistência pelo tempo configurado provoca desligamento.
- **SCC-007:** `TestingRecovery` tem exclusivamente as fases internas
  `OpenCircuitStabilization` (`false`) e `LoadedValidation` (`true`). Toda
  entrada nesse estado começa pela primeira fase, inclusive a partir de `Off`.

## 6. Qualidade da geração e histerese

### 6.1 Critérios básicos

- **SCC-008:** sob carga, a condição mínima para manter geração útil é:

```text
panelPower = panelVoltage * panelCurrent
useful = panelPower >= minimumUsefulPanelPower
         AND batteryChargeCurrent >= minimumUsefulBatteryChargeCurrent

effectiveHealthyPanelVoltage = max(
    minimumHealthyPanelVoltage,
    lastOpenCircuitVoltage * minimumLoadedVoltageRatio)

healthy = useful
          AND vocValid
          AND panelVoltage >= effectiveHealthyPanelVoltage
          AND panelVoltage >= batteryVoltage + minimumBuckHeadroomVoltage
```

A expressão relativa só é calculada com Voc válido. Sem ele, `healthy` é falso;
não se usa Voc vencido nem se recorre silenciosamente apenas ao limite absoluto.

- **SCC-009:** a classificação usa os predicados com histerese:

```text
healthy                       → Healthy
useful AND NOT healthy        → Degraded
NOT useful                    → Weak
```

Com buck desligado, não se classifica geração por corrente ou potência. Voc
alto sozinho não equivale a `Healthy`. A margem sobre a bateria é critério de
qualidade; violação de limite elétrico crítico é tratada separadamente em `Fault`.

### 6.2 Entrada, manutenção e recuperação

- **SCC-010:** `useful` e `healthy` são memórias booleanas do classificador.
  Na entrada em `LoadedValidation`, ambas começam falsas. Na faixa entre os
  limiares de manutenção e recuperação, preserva-se o valor anterior.

| Memória | Desativa quando | Ativa novamente quando |
|---|---|---|
| `useful` | Potência abaixo do mínimo OU corrente de carga abaixo do mínimo | Potência >= `usefulRecoveryPanelPower` E corrente >= `usefulRecoveryBatteryChargeCurrent` |
| `healthy` | `useful` falso, Voc inválido, tensão abaixo do limite híbrido de manutenção OU margem abaixo do mínimo | `useful` verdadeiro, Voc válido, tensão >= limite híbrido de recuperação E margem >= `buckHeadroomRecoveryVoltage` |

O limite híbrido de recuperação é:

```text
max(healthyRecoveryPanelVoltage,
    lastOpenCircuitVoltage * loadedVoltageRecoveryRatio)
```

`minimumLoadedVoltageRatio` governa manutenção; `loadedVoltageRecoveryRatio`
governa reentrada. Igualdade satisfaz o limiar; a saída exige valor estritamente
inferior. Histerese elétrica e confirmação temporal são complementares.

- **SCC-011:** a perda de `healthy` inicia confirmação de degradação enquanto
  `useful` permanece verdadeiro. Retornar a `Healthy` cancela essa confirmação.
  A recuperação `Degraded` → `Charging` usa os limiares superiores, não os mínimos.

## 7. Voc e recovery probe

- **SCC-012:** `lastOpenCircuitVoltage` deve ser capturado de amostra válida
  adquirida após `openCircuitStabilizationMs` contínuos com buck desligado.
  Registra-se o instante da aquisição; releitura do mesmo snapshot não renova
  sua idade. A integração deve garantir que desligar o buck efetivamente alivia
  a carga do painel o suficiente para a referência de circuito aberto.
- **SCC-013:** `vocValid` exige referência capturada e idade estritamente menor
  que `openCircuitVoltageValidityMs`. Não se renova Voc sob carga. Ao vencer
  durante operação, solicita-se novo probe conforme a tabela de transições.
  Reinicialização, `Off` e `Fault` invalidam a referência.
- **SCC-014:** Voc suficiente para autorizar o teste exige simultaneamente
  `panelVoltage >= minimumOpenCircuitVoltage` e
  `panelVoltage >= batteryVoltage + buckHeadroomRecoveryVoltage`, usando
  medição atual sem carga. Isso autoriza somente o teste, não o carregamento.
- **SCC-015:** em `OpenCircuitStabilization`, o buck permanece desligado pelo
  intervalo completo. Depois, captura-se Voc; se insuficiente, retorna-se a
  `WaitingForRecovery`. Se suficiente, habilita-se o buck e inicia-se
  `LoadedValidation` com classificadores reiniciados.
- **SCC-016:** `recoveryTestDurationMs` conta somente `LoadedValidation`, após
  habilitar o buck. Avaliam-se amostras novas adquiridas sob carga, nunca a
  amostra de circuito aberto. Ao fim da janela: `Healthy` em todas as amostras
  leva a `Charging`; geração útil em todas, mas nem sempre saudável, leva a
  `Degraded`; qualquer amostra `Weak` torna o teste insuficiente. Nesse último
  caso, pode-se encerrar imediatamente em `WaitingForRecovery`.

Não existe tolerância implícita de partida nesta v0.1. A janela deve conter
amostras novas desde a primeira oportunidade de aquisição sob carga até seu
fim, respeitando o contrato de frescor. Falta de medições válidas é falha de
sensor, não aprovação do teste. A qualificação “durante toda a janela” refere-se
às amostras válidas observadas, não à garantia de continuidade analógica.

## 8. Temporizações e configuração

- **SCC-017:** todos os valores elétricos e temporais abaixo são defaults
  provisórios e ajustáveis. Exigem calibração no painel, buck, bateria, carga e
  sensores reais; não são limites de proteção certificados.

| Parâmetro | Default provisório | Função |
|---|---:|---|
| `minimumHealthyPanelVoltage` | 18 V | Limite absoluto de manutenção saudável |
| `healthyRecoveryPanelVoltage` | 20 V | Limite absoluto de reentrada saudável |
| `minimumLoadedVoltageRatio` | 0,55 | Fração mínima do Voc válido |
| `loadedVoltageRecoveryRatio` | 0,60 | Fração do Voc para reentrada |
| `minimumUsefulPanelPower` | 20 W | Manutenção útil |
| `usefulRecoveryPanelPower` | 25 W | Reentrada útil |
| `minimumUsefulBatteryChargeCurrent` | 0,5 A | Manutenção útil |
| `usefulRecoveryBatteryChargeCurrent` | 0,7 A | Reentrada útil |
| `minimumBuckHeadroomVoltage` | 2 V | Margem mínima painel/bateria |
| `buckHeadroomRecoveryVoltage` | 3 V | Margem para reentrada/teste |
| `minimumOpenCircuitVoltage` | 20 V | Voc mínimo para tentar carga |
| `weakGenerationConfirmationMs` | 3000 ms | Persistência contínua de `Weak` antes de desligar |
| `degradedConfirmationMs` | 3000 ms | Persistência contínua de `Degraded` antes de mudar de estado |
| `maximumDegradedDurationMs` | 60000 ms | Prazo máximo do episódio degradado até novo probe |
| `recoveryWaitMs` | 30000 ms | Espera após tentativa insuficiente/geração fraca confirmada |
| `recoveryTestDurationMs` | 3000 ms | Janela exclusivamente sob carga |
| `openCircuitStabilizationMs` | 2500 ms | Estabilização contínua com buck desligado |
| `openCircuitVoltageValidityMs` | 300000 ms | Validade temporal do Voc |

- **SCC-018:** parâmetros devem ser finitos e positivos; razões pertencem a
  `(0, 1]`. Limiares de recuperação devem superar os respectivos mínimos.
  `openCircuitVoltageValidityMs` deve superar `recoveryTestDurationMs`;
  `maximumDegradedDurationMs` deve superar `degradedConfirmationMs`.
  Configuração inconsistente impede habilitação e produz `Fault`.
- **SCC-019:** tempos são decorridos monotônicos; expiração ocorre em
  `elapsed >= duration`. Usar diferenças seguras no rollover, nunca comparar
  diretamente timestamps absolutos. Para contador de 32 bits, durações e
  intervalo entre atualizações devem ser menores que meia faixa do contador.
- **SCC-020:** o episódio degradado começa na primeira amostra `Degraded`
  após `Healthy`, incluindo sua confirmação. Passagens por `WeakGeneration`
  não reiniciam esse prazo. Ele termina por retorno a `Healthy`, desligamento
  ou novo probe. Se o teste termina útil/degradado, o episódio começa ao sair
  de `LoadedValidation`. `Degraded` nunca pode prolongar seu prazo reiniciando
  timers a cada atualização ou alternando com `WeakGeneration`.
- **SCC-021:** o timer fraco inicia na entrada em `WeakGeneration` e só conta
  `Weak` contínuo. Recuperação cancela o desligamento pendente. Entrada em
  `WaitingForRecovery` inicia novo cooldown. Voc insuficiente durante espera
  não reinicia esse cooldown; pode-se tentar quando a tensão se tornar suficiente.

## 9. Tabela formal de transições

- **SCC-022:** aplicar primeiro as regras globais na ordem abaixo. Depois,
  aplicar a primeira regra local satisfeita. A saída corresponde ao estado e
  fase resultantes na mesma atualização. Ausência de condição mantém estado,
  fase e saída; não executar cascata que pule uma fase física de estabilização.

| Origem | Condição global, em ordem de prioridade | Destino | Ação |
|---|---|---|---|
| Qualquer | Falha crítica presente | `Fault` | Desabilitar imediatamente; invalidar Voc e timers |
| `Fault` | Falha removida, sem reset explícito | `Fault` | Manter desligado |
| `Fault` | Falha removida + reset; habilitação externa falsa | `Off` | Manter desligado e limpar retenção |
| `Fault` | Falha removida + reset; habilitação externa verdadeira | `WaitingForRecovery` | Limpar retenção e iniciar cooldown |
| Qualquer, exceto `Fault` | Habilitação externa falsa | `Off` | Desabilitar, invalidar Voc e limpar timers |

| Origem | Condição local, em ordem por origem | Destino | Ação |
|---|---|---|---|
| `Off` | Habilitação externa verdadeira | `TestingRecovery/OpenCircuitStabilization` | Manter desligado e iniciar estabilização |
| `WaitingForRecovery` | Cooldown ou estabilização sem carga ainda incompleto | Mesmo | Manter desligado |
| `WaitingForRecovery` | Espera cumprida, medição atual de Voc insuficiente | Mesmo | Manter desligado e continuar observando |
| `WaitingForRecovery` | Espera cumprida + Voc suficiente | `TestingRecovery/OpenCircuitStabilization` | Iniciar fase completa de estabilização |
| `TestingRecovery/OpenCircuitStabilization` | Estabilização incompleta | Mesmo | Manter desligado |
| `TestingRecovery/OpenCircuitStabilization` | Estabilização concluída + Voc insuficiente | `WaitingForRecovery` | Capturar referência e reiniciar cooldown |
| `TestingRecovery/OpenCircuitStabilization` | Estabilização concluída + Voc suficiente | `TestingRecovery/LoadedValidation` | Capturar Voc, habilitar e iniciar janela sob carga |
| `TestingRecovery/LoadedValidation` | Amostra `Weak` ou Voc vencido | `WaitingForRecovery` | Desabilitar e reiniciar cooldown |
| `TestingRecovery/LoadedValidation` | Janela incompleta, sem insuficiência | Mesmo | Manter ligado e acumular resultado |
| `TestingRecovery/LoadedValidation` | Janela concluída, todas as amostras `Healthy` | `Charging` | Manter ligado |
| `TestingRecovery/LoadedValidation` | Janela concluída, todas úteis, alguma `Degraded` | `Degraded` | Manter ligado e iniciar episódio degradado |
| `Charging` ou `Degraded` | Qualidade `Weak` | `WeakGeneration` | Manter ligado e iniciar confirmação fraca |
| `Charging` ou `Degraded` | Voc vencido ou prazo do episódio degradado expirado | `TestingRecovery/OpenCircuitStabilization` | Desabilitar e iniciar probe |
| `Charging` | `Degraded` contínuo por `degradedConfirmationMs` | `Degraded` | Manter ligado e preservar início do episódio |
| `Charging` | `Healthy` ou degradação ainda não confirmada | Mesmo | Manter ligado; `Healthy` limpa episódio/confirmação |
| `Degraded` | `Healthy` | `Charging` | Manter ligado e cancelar episódio |
| `Degraded` | `Degraded`, dentro do prazo | Mesmo | Continuar aproveitando geração |
| `WeakGeneration` | `Weak` contínuo por `weakGenerationConfirmationMs` | `WaitingForRecovery` | Desabilitar e iniciar cooldown |
| `WeakGeneration` | Voc vencido ou prazo degradado expirado | `TestingRecovery/OpenCircuitStabilization` | Desabilitar e iniciar probe |
| `WeakGeneration` | `Healthy`, antes da confirmação fraca | `Charging` | Cancelar confirmação e episódio |
| `WeakGeneration` | `Degraded`, antes da confirmação fraca | `Degraded` | Cancelar confirmação fraca; preservar episódio existente ou iniciar um |
| `WeakGeneration` | `Weak`, antes do timeout | Mesmo | Manter ligado durante confirmação |

A prioridade de `Weak` em `Charging`/`Degraded` pode adiar o probe até a
próxima atualização em `WeakGeneration`, sem reiniciar qualquer prazo.

## 10. Falhas e limites de interpretação

- **SCC-023:** falha crítica elétrica/térmica indicada pela integração,
  configuração inválida ou conjunto obrigatório de medições inválido/vencido
  produz `Fault` na primeira atualização que observa a condição. Não aguarda
  confirmação fraca, fim de teste ou prazo degradado. Reset com falha ainda
  presente não tem efeito; remoção isolada não religa automaticamente.
- **SCC-024:** a integração define e valida limites elétricos/térmicos conforme
  o hardware. A v0.1 consome essa indicação e não exige sensor de temperatura
  como quinta entrada mínima. Zero de geração, descarga da bateria e Voc baixo
  não são por si só falhas críticas; são condições de geração/espera quando as
  medições permanecem válidas.
- **SCC-025:** `Weak` significa insuficiência para o objetivo de carga desta
  versão. Bateria cheia ou consumo da casa absorvendo toda a geração pode
  resultar em `Weak` mesmo com painel produtivo. A v0.1 não distingue esses
  casos nem altera seu critério para manter somente alimentação das cargas.
  A decisão de EN não substitui proteções físicas nem comprova que o buck
  obedeceu ao comando.

## 11. Critérios de aceite e evidências

Os cenários abaixo orientam a validação futura com medições e relógio simulados,
sem GPIO. Esta autoria não cria nem executa suíte; todos os resultados
comportamentais e físicos permanecem `Not Executed`.

| Critério | Requisitos | Cenário e resultado observável | Meio |
|---|---|---|---|
| SCC-AC-001 | SCC-001 a SCC-005 | Instanciar sem hardware começa `Off/false`; mesma sequência reproduz decisões; potência e sinais respeitam unidades | Inspeção e simulação determinística |
| SCC-AC-002 | SCC-006, SCC-008 a SCC-011 | Potência ou corrente insuficiente dá `Weak`; ambas úteis com baixa tensão dá `Degraded`; tensão híbrida e margem atendidas dão `Healthy` | Entradas controladas |
| SCC-AC-003 | SCC-008 a SCC-010 | Voc de 40 V e razão 0,55 exigem 22 V na manutenção; Voc de 30 V preserva piso de 18 V; margem da bateria ainda é exigida | Cálculo e simulação |
| SCC-AC-004 | SCC-010, SCC-011, SCC-017, SCC-018 | Oscilar entre limites mantém memória; igualdade satisfaz entrada; sair abaixo do mínimo exige recuperação superior | Simulação de fronteiras |
| SCC-AC-005 | SCC-007, SCC-012 a SCC-016 | Toda entrada de teste começa desligada; captura Voc após estabilização; somente então habilita; teste saudável, degradado e fraco seguem destinos distintos | Traço de fases, tempo e saída |
| SCC-AC-006 | SCC-012, SCC-013, SCC-022 | Voc vence exatamente no limite; snapshot repetido não o renova; operação solicita probe e nunca usa referência vencida como saudável | Relógio e amostras simulados |
| SCC-AC-007 | SCC-019 a SCC-022 | Weak breve recupera sem desligar; persistente desliga exatamente no timeout; espera exige cooldown e Voc suficiente | Simulação temporal e rollover |
| SCC-AC-008 | SCC-011, SCC-020, SCC-022 | Degradação breve é tolerada; persistente muda estado; prazo máximo força probe mesmo alternando com Weak sem recuperar Healthy | Traço de episódio degradado |
| SCC-AC-009 | SCC-022 a SCC-024 | Cada origem/fase com falha vai a Fault/false; falha vence timeout e comando; reset não religa com falha presente | Matriz de transições e prioridades |
| SCC-AC-010 | SCC-003, SCC-016, SCC-023 | NaN, infinito, sentinel, amostras vencidas ou ausência de dados sob carga nunca aprovam teste | Entradas inválidas controladas |
| SCC-AC-011 | SCC-001, SCC-004, SCC-025 | Controller só decide EN; integração normaliza corrente líquida; geração sem carga de bateria não é classificada como útil | Inspeção de fronteiras e cenários |

Para esta entrega documental: `git diff --check` e guarda estrutural EKOM.
Calibração, builds, implementação, criação de testes e ensaios de hardware
pertencem a etapas posteriores e não são declarados concluídos.

## 12. Conhecimento afetado, decisões e pendências

**Fatos observados:** especificações equivalentes ficam em `docs/specs/` e usam
requisitos identificados e tabelas de aceite. O projeto já possui adapters de
medição, exemplo MCB01 e abstração substituível de tempo. Isso não comprova a
implementabilidade formal nem a adequação física dos defaults deste controller.

**Decisões confirmadas na conversa e na ordem de escrita:** estados, duas fases
do teste, avaliação conjunta de tensão/potência/corrente, critério híbrido com
Voc temporário, histerese, aproveitamento degradado limitado, falha crítica,
separação de hardware e decisão exclusiva de habilitar/desabilitar o buck.

**Consolidações propostas neste Draft:** Voc vencido força novo probe; confirmação
fraca ocorre dentro de `WeakGeneration`; o prazo degradado inclui confirmação
e não reinicia em passagens fracas; limiares superiores aplicam-se também à razão
de Voc e à margem; teste exige geração útil em toda a janela observada. Esses
fechamentos eliminam ambiguidades do rascunho conversacional sem afirmar que já
foram validados no sistema real.

**Pendências:** análise formal de implementabilidade; calibração de todos os
defaults; limites de proteção, frescor e coerência de aquisição na integração;
validação do tempo de partida do buck e da aproximação de circuito aberto com
EN desligado. Não há declaração `Ready` nem ordem de implementação nesta autoria.

**Conhecimento afetado:** registrar a fonte, sua fronteira lógica e estado
`Draft` em `docs/rfc/KNOWLEDGE-MAP.md`, e a autoria em
`docs/rfc/EKOM-CHANGELOG.md`. Nenhum débito técnico é aceito por esta autoria.
