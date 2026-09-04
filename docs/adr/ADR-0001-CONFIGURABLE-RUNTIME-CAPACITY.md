# ADR-0001 — Capacidade estática configurável do runtime

**Estado:** Aceita pelo Arquiteto [`Accepted`]

**Data:** 03/09/2026

**Especificação:** `IOTSSC-RUNTIME-CAPABILITY-CAPACITY@0.1`

## Contexto

O runtime mantém capabilities, destrutores e adapters em armazenamento estático
e rejeita o nono registro por um limite fixo de oito. A aplicação
`ESP32_MCB01` requer nove capabilities simultâneas. Aumentar literais isolados
não cobre bookkeeping, arena nem o provider NVS, cujo formato versão 2 também
possui oito registros.

Microcontroladores e environments possuem orçamentos distintos. Um aumento
universal consumiria memória sem necessidade, enquanto capacidade dinâmica
mudaria ownership e previsibilidade do runtime.

## Decisão

1. A capacidade de capabilities permanece estática e determinada em build.
2. O default público permanece oito.
3. `ESP32_MCB01` usa capacidade doze; doze é o máximo suportado nesta revisão.
4. Slots, destrutores, adapters e bookkeeping dependente compartilham a mesma
   autoridade de configuração ou um limite explicitamente suficiente.
5. A arena continua estática e deve ser dimensionada e validada para a
   composição de cada perfil, sem fallback em heap.
6. O formato NVS novo possui doze registros em todos os environments, separado
   da capacidade ativa do runtime, para que o layout persistente não varie por
   build.
7. O provider migra snapshots válidos da versão 2 com oito registros para o
   formato novo sem write síncrono, erase global ou acesso ao domínio de
   settings.
8. Configuração e registro continuam concluídos antes de `SmartSysApp::setup()`;
   não existe mutação posterior do conjunto administrado.

## Consequências

- O perfil MCB01 ganha margem para a composição atual e crescimento até doze.
- Consumers sem override preservam o limite e o consumo de slots vigentes.
- O aumento do formato NVS consome memória estática adicional no provider e
  exige compatibilidade explícita com o blob anterior.
- Capacidade nominal não elimina falhas independentes de arena, adapter,
  identidade ou recurso; todas permanecem observáveis e atômicas.
- Uma capacidade futura acima de doze exige nova revisão normativa e de formato
  persistente.

## Alternativas rejeitadas

- **Trocar somente `8` por `9`:** acopla a arquitetura ao inventário atual e
  mantém limites divergentes.
- **Aumentar universalmente para doze:** impõe memória adicional a todo
  environment sem decisão específica.
- **Registrar depois de `setup()`:** o manager já captura a contagem e essa
  opção viola o lifecycle vigente.
- **Alocação dinâmica:** reduz previsibilidade de memória e altera ownership.
- **Descartar o snapshot versão 2:** perde estados válidos após atualização.
