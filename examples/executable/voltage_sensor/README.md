# voltage_sensor

Demonstra `SmartSysApp::addVoltageSensor()` medindo tensão não negativa com um
divisor resistivo no ADC da MCB R1. A capability é conceitualmente neutra quanto
a DC ou AC; este primeiro Hardware Adapter implementa a conversão do divisor.

O exemplo apenas configura a capability e apresenta a última medição estável.
A aquisição, a média, os limites e a conversão elétrica permanecem no adapter.

## API e configuração

- capability `Voltage Sensor (V)` com identificador `pv-voltage`;
- `VoltageSensorConfig` com R1 = 330000 Ω e R2 = 10000 Ω;
- razão calculada `(R1 + R2) / R2 = 34`;
- limiar inferior de 144 mV e avaliação a cada 1000 ms pelos defaults;
- `VoltageSensorCapability::voltageMeasurement()` para consulta não proprietária.

## Ligação

```text
VIN ── R1 (330 kΩ) ──┬── ADC / J4 / ITS_MCB01_J4_EXT_ADC
                     │
                  R2 (10 kΩ)
                     │
GND ─────────────────┘
```

O GND da fonte medida e o GND da MCB R1 devem ser comuns. O GPIO vem
exclusivamente do símbolo oficial `ITS_MCB01_J4_EXT_ADC`; o exemplo não define
um literal alternativo.

## Faixa e estados

Com razão 34:1, o limiar ADC de 144 mV corresponde a 4,896 V na entrada.
Leituras abaixo desse limiar produzem `BELOW_MINIMUM` e o sentinel público
`-1000.00`; ele significa leitura abaixo do mínimo possível, não tensão física.

O limite ADC de 3100 mV corresponde à fronteira calculada de 105,400 V na
entrada. Essa conta não afirma que resistores, placa, isolamento ou montagem
sejam seguros nessa tensão. Na fronteira ou acima dela, o estado é
`ADC_SATURATION` e não existe valor numérico.

## Build, upload e monitor

```sh
pio run -e example_voltage_sensor_mcb_r1
pio run -e example_voltage_sensor_mcb_r1 -t upload
pio device monitor -e example_voltage_sensor_mcb_r1
```

## Validação manual

1. Desenergize o circuito e monte o divisor conforme o diagrama, verificando
   valores, potência, tensão nominal dos resistores e GND comum.
2. Confirme por instrumento independente que o nó ADC permanece dentro da
   faixa segura antes de conectá-lo à placa.
3. Grave o firmware e confirme no boot a placa, o GPIO, `pv-voltage`, R1, R2,
   razão 34 e limiar de 144 mV.
4. Observe `NOT_READY` até completar o primeiro lote.
5. Verifique que nó ADC abaixo de 144 mV produz `BELOW_MINIMUM` e `-1000.00`,
   144 mV produz aproximadamente 4,90 V e 3100 mV produz `ADC_SATURATION`.

Proteção contra sobretensão, isolamento, fusível e dimensionamento elétrico
estão fora do software. Não use a fronteira matemática como limite seguro do
hardware.
