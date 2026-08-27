# current_sensor

Demonstra a API pública `SmartSysApp::addCurrentSensor()` medindo a corrente
contínua entre o painel fotovoltaico e a entrada do conversor buck com o
ACS712-30A, conforme `docs/specs/CURRENT-SENSING-CAPABILITY.md`.

O exemplo apenas configura a capability, apresenta a última medição estável e
solicita recalibração de zero. Aquisição, calibração, conversão elétrica,
filtragem, faixas e qualificação permanecem no adapter da biblioteca.

## APIs e capability demonstradas

- `SmartSysApp::addCurrentSensor(CurrentSensorConfig)`;
- perfis públicos `CurrentSensorConfig::ACS712_30A_3V3()` e
  `CurrentSensorConfig::ACS712_30A_5V()`;
- `CurrentSensorCapability::currentMeasurement()` e
  `CurrentSensorCapability::requestZeroCalibration()`;
- capability `Current Sensor (A)` com identificador `pv-current`.

## Hardware e pinos

| Sinal | MCB R1 | Configuração |
|---|---:|---|
| Saída analógica do ACS712 | GPIO 34 (conector J4) | `ITS_MCB01_J4_EXT_ADC` |
| Perfil elétrico | — | `EXAMPLE_CURRENT_SENSOR_PROFILE_3V3=1` |
| Identificador da capability | — | `EXAMPLE_CURRENT_SENSOR_ID="pv-current"` |
| Cadência da apresentação | 1000 ms | `EXAMPLE_CURRENT_LOG_INTERVAL_MS=1000` |

O GPIO 34 é entrada apenas e pertence ao ADC1 do ESP32 clássico, o único
controlador utilizável com o Wi-Fi ativo do runtime. O valor numérico consta
somente para montagem e diagnóstico: o código e o environment usam o símbolo
oficial do pinout.

### Ligação

```text
Painel (+) ──► IP+  ACS712-30A  IP- ──► entrada do buck
                    VCC ──► 3V3 da placa (perfil 3V3)
                    GND ──► GND comum com a MCB R1
                    OUT ──► J4 / GPIO 34 (ITS_MCB01_J4_EXT_ADC)
```

Perfil `ACS712_30A_3V3`: alimentação de 3,3 V, sem divisor, `OUT` ligado
diretamente ao GPIO 34. Esse perfil é `PROJECT_VALIDATED`, fora da faixa de
alimentação oficialmente garantida pelo fabricante do ACS712 original.

Perfil `ACS712_30A_5V` (`EXAMPLE_CURRENT_SENSOR_PROFILE_5V=1`, bancada):
alimentação de 5 V e divisor obrigatório de 10 kΩ / 20 kΩ entre `OUT` e o GPIO.
Ligar a saída de 5 V diretamente ao ADC danifica o SoC.

## Alimentação não monitorada

O exemplo não configura `supplyMonitorAdcPin`. O estado publicado é sempre
`NOT_MONITORED` e, nessa condição, a exatidão contratada da medição **não é
afirmada**, mesmo quando o estado de medição for `VALID`.

## Build, upload e monitor

```sh
pio run -e example_current_sensor_mcb_r1
pio run -e example_current_sensor_mcb_r1 -t upload
pio device monitor -e example_current_sensor_mcb_r1
```

Credenciais e endpoints, quando necessários ao runtime, vêm da configuração
privada já suportada pelo projeto e não fazem parte deste exemplo.

## Validação manual

1. Com tudo desenergizado, monte o ACS712 no perfil escolhido, com GND comum, e
   garanta corrente zero no circuito medido.
2. Grave o firmware e abra o monitor a 115200 baud.
3. Confirme o boot `id=current_sensor`, a placa, `signal_pin=34`,
   `profile=ACS712_30A_3V3` e `capability=pv-current`.
4. Durante o aquecimento de 60 s, observe `current=<none>` com
   `measurementStatus=NOT_READY`, depois `CALIBRATING` e, ao final, medições com
   `supplyStatus=NOT_MONITORED`.
5. Sem corrente, confirme `ESTIMATED` com `current=0.000 A`.
6. Aplique correntes conhecidas dentro da faixa calibrada (0,50 A a 15,00 A) e
   compare com um instrumento de referência de erro máximo ±1%.
7. Inverta o sentido da corrente e confirme a preservação do sinal.
8. Com corrente zero garantida e alimentação estável, envie `c` pelo monitor e
   confirme `CALIBRATING` seguido de retorno a leituras normais.

Resultado esperado: nenhum valor numérico antes da calibração; zero entre
−0,05 A e +0,05 A; leituras dentro de `max(0,10 A; 5%)` na faixa calibrada;
`OUT_OF_CALIBRATED_RANGE` e `OVERCURRENT_OR_SATURATION` sem valor numérico.

## Limitações e riscos

- Corrente contínua fotovoltaica apenas; não há valor eficaz, potência ou
  energia acumulada.
- O ACS712 é inserido em série com a corrente do painel: desenergize antes de
  qualquer alteração de montagem e dimensione condutores e conectores.
- A calibração de zero não é persistida; cada reinício repete o aquecimento.
- O perfil de 3,3 V depende de validação integral do projeto; o perfil de 5 V
  exige o divisor contratado.
- Correntes acima de 15 A devem ser validadas por injeção instrumentada,
  conforme a especificação, e não por produção física obrigatória de 30 A.
