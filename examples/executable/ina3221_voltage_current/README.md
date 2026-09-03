# ina3221_voltage_current

Demonstra, na MCB R1, leitura simultânea de tensão de barramento e corrente do
canal 0 de um INA3221. Um único `INA3221Device` possui o driver Adafruit e é
compartilhado por `INA3221VoltageSensor` e `INA3221CurrentSensor`. As duas
capabilities recebem os adapters por referência e conduzem seu `setup()` e
`handle()`; a aplicação do exemplo mantém dispositivo e adapters vivos.

## API e configuração

- `SmartSysApp::addVoltageSensor(id, IVoltageSensor&, 1000)`;
- `SmartSysApp::addCurrentSensor(id, ICurrentSensor&, 1000)`;
- `Wire`, endereço `0x40`, média de 16 amostras e conversões de barramento e
  shunt de aproximadamente 1 ms;
- canal 0, shunt `R100` (`0,100 Ω`) e polaridade `+1`;
- leitura dos adapters a cada 500 ms e apresentação a cada 1000 ms;
- faixa configurada de tensão `[0, 26 V)` e de corrente até `1,500 A` em módulo.

O limite teórico do conversor para `0,100 Ω` é aproximadamente `±1,638 A`,
obtido por `0,1638 V / 0,100 Ω`. Esse valor não é limite térmico seguro.

## Ligação high-side

```text
Fonte + ── VIN+ CH1 ──[ shunt R100 ]── VIN- CH1 ── Carga ── Fonte -
   │                                                    │
   └──────── tensão de barramento monitorada ───────────┘

MCB R1 3V3 ── VCC do módulo       MCB R1 GND ── GND do módulo/Fonte -
MCB R1 ESP32_SDA / GPIO 21 ────── SDA
MCB R1 ESP32_SCL / GPIO 22 ────── SCL
```

Confirme no módulo a identificação de entrada e saída do shunt. Com polaridade
`+1`, corrente no sentido `VIN+` para `VIN-` deve ser positiva; inverta somente
a configuração de polaridade se a orientação física documentada exigir.

| Sinal | Símbolo oficial | GPIO/valor |
|---|---|---:|
| I²C SDA | `ESP32_SDA` | 21 |
| I²C SCL | `ESP32_SCL` | 22 |
| Endereço | — | `0x40` |
| Canal | — | 0 |
| Shunt | `R100` | `0,100 Ω` |

## Build, upload e monitor

```sh
pio run -e example_ina3221_voltage_current_mcb_r1
pio run -e example_ina3221_voltage_current_mcb_r1 -t upload
pio device monitor -e example_ina3221_voltage_current_mcb_r1
```

## Sequência manual

1. Com tudo desenergizado, confirme endereço, shunt, orientação, continuidade,
   GND comum e ausência de curto.
2. Use fonte limitada em corrente e mantenha a validação inicial em no máximo
   `0,5 A` em módulo.
3. Energize o módulo entre `2,7 V` e `5,5 V` e confirme `0x40` no barramento.
4. Faça upload, abra o monitor e confira no boot placa, símbolos/GPIOs, endereço,
   canal, shunt, média, conversões, identidades e intervalos.
5. Compare tensão e corrente apresentadas com instrumentos independentes.
6. Confirme sinal positivo no sentido documentado, `NOT_MONITORED` para a
   alimentação e os estados de faixa ao variar a bancada com segurança.

Resultados esperados são tensão com duas casas e corrente com três, acompanhadas
por `VoltageMeasurementStatus`, `CurrentMeasurementStatus` e
`CurrentSupplyStatus`. Antes da primeira leitura ou quando o dispositivo está
indisponível, o valor fica ausente em `NOT_READY`. Leituras de tensão no limite
configurado produzem `ADC_SATURATION`; corrente acima do limite configurado
produz `OVERCURRENT_OR_SATURATION`.

## Limites e segurança

O INA3221 aceita no máximo 26 V de tensão de barramento em operação. A marcação
`R100` informa resistência, não potência nominal, corrente segura, dissipação ou
capacidade das trilhas do breakout. A faixa configurada não protege o circuito:
use fusível, limitação de corrente, proteção contra sobretensão, curto, inversão
e transientes adequados. Software e cadência de leitura não substituem essas
proteções. Desconexão I²C posterior ao setup pode não ser detectada em toda
leitura pela biblioteca empregada.
