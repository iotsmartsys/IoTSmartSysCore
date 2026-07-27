# basic_light

Demonstra a API pública `SmartSysApp::addLightCapability()` controlando a saída de relé da MCB R1. A saída inicia desligada pelo adapter da capability e só deve ser energizada por um comando válido recebido pelo runtime.

## Hardware e pinos

| Sinal | MCB R1 | Configuração |
|---|---:|---|
| Saída da luz/relé | GPIO 26 | `ITS_MCB01_RELAY_PIN` |
| Nível ativo | alto | `EXAMPLE_LIGHT_ACTIVE_HIGH=1` |

Use somente a saída lógica/relé da MCB R1 durante o primeiro teste. Não conecte cargas de rede elétrica sem isolamento, caixa e dimensionamento adequados.

## Build, upload e monitor

```sh
pio run -e example_basic_light_mcb_r1
pio run -e example_basic_light_mcb_r1 -t upload
pio device monitor -e example_basic_light_mcb_r1
```

Credenciais e endpoints, quando necessários ao runtime, devem ser fornecidos pela configuração privada já suportada pelo projeto. Eles não fazem parte deste exemplo.

## Validação manual

1. Com a placa desenergizada, conecte uma carga segura de baixa tensão à saída de relé correspondente ao GPIO 26.
2. Grave o firmware e abra o monitor a 115200 baud.
3. Confirme o log de boot `id=basic_light`, a placa, o pino 26 resolvido pelo símbolo oficial `ITS_MCB01_RELAY_PIN` e `active_high=1`.
4. Conclua o provisioning, se necessário, e envie comandos de ligar e desligar para a capability `basic_light` pela infraestrutura configurada.
5. Registre o log e observe a comutação da saída nos dois sentidos.

Resultado esperado: a saída permanece segura durante o boot e acompanha os comandos da capability. A validação depende da conectividade/provisioning privados disponíveis e não cobre cargas de potência.
