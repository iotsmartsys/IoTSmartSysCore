# fan

Demonstra a API pública `SmartSysApp::addFanCapability()` controlando um
ventilador como atuador binário pela saída de relé da MCB R1. A capability usa
o type `Fan Actuator`, inicia desligada e responde aos mesmos comandos binários
de `SwitchCapability`.

O exemplo não implementa PWM, seleção de velocidade, inversão, tacômetro ou
controle térmico automático.

## API e configuração

- `FanConfig`, com identificador `fan`;
- `ITS_MCB01_RELAY_PIN` como fonte exclusiva do GPIO;
- nível ativo alto por `EXAMPLE_FAN_ACTIVE_HIGH=1`;
- `SmartSysApp::addFanCapability()` antes de `SmartSysApp::setup()`;
- `SmartSysApp::handle()` continuamente no `loop()`.

## Hardware e ligação

| Sinal | MCB R1 | Configuração |
|---|---:|---|
| Comando do ventilador/relé | GPIO 26 | `ITS_MCB01_RELAY_PIN` |
| Nível ativo | alto | `EXAMPLE_FAN_ACTIVE_HIGH=1` |

```text
MCB R1 / K1 (relé) ── circuito de alimentação adequado ── ventilador
```

Não conecte o motor diretamente ao GPIO. Use a saída de relé ou um driver
dimensionado para a tensão, corrente de partida e natureza indutiva da carga.
Em driver DC externo, use proteção contra retorno indutivo conforme o circuito.
Para tensão de rede, use isolamento, caixa, aterramento, fusível e componentes
certificados, com intervenção de profissional qualificado.

Mantenha pás, hélices e partes móveis protegidas contra contato e bloqueie a
alimentação antes de qualquer alteração na montagem.

## Build, upload e monitor

```sh
pio run -e example_fan_mcb_r1
pio run -e example_fan_mcb_r1 -t upload
pio device monitor -e example_fan_mcb_r1
```

Credenciais e endpoints eventualmente necessários ao runtime permanecem na
configuração privada vigente e não integram o exemplo.

## Validação manual

1. Com a alimentação desligada, conecte uma carga segura e adequada à saída de
   relé da MCB R1.
2. Grave o firmware e abra o monitor serial a 115200 baud.
3. Confirme o boot com `id=fan`, placa `iotsmartsys_mcb_r1`, GPIO 26,
   `active_high=1` e capability `fan`.
4. Confirme que o ventilador permanece desligado depois do boot quando não há
   estado persistido ligado.
5. Conclua o provisioning, se necessário, e envie comandos `on`, `off` e
   `toggle` para a capability `fan`.
6. Observe que a carga acompanha o estado confirmado e registre logs e
   comportamento físico.

Resultado esperado: a saída inicia em estado seguro e acompanha os comandos
binários válidos. Um estado `on` previamente persistido pode ser restaurado no
`setup()`; registros antigos com type `Switch` não pertencem à identidade
`Fan Actuator`.
