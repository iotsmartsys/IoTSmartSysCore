# power_energy

Demonstra a API pública `SmartSysApp::addPowerEnergyCapability()` compondo as
últimas medições de tensão e corrente em potência não negativa e energia
acumulada volátil. O exemplo usa a MCB R1, um ACS712-30A e um divisor resistivo,
conforme `docs/specs/POWER-ENERGY-CAPABILITY.md`.

Os adapters são instâncias externas possuídas pelo exemplo. Ele chama
`setup()` e `handle()` diretamente em cada sensor e registra somente a
`PowerEnergyCapability`; não registra `CurrentSensorCapability` nem
`VoltageSensorCapability`. Aquisição, calibração, conversão, qualificação,
cálculo de potência, integração e cadências internas permanecem na biblioteca.

## APIs e componentes demonstrados

- `ACS712C30ACurrentSensor` com
  `CurrentSensorConfig::ACS712_30A_3V3()`;
- `ResistiveDividerVoltageSensor` com `VoltageSensorConfig`;
- `SmartSysApp::addPowerEnergyCapability(PowerEnergyConfig, IVoltageSensor&,
  ICurrentSensor&)`;
- `PowerEnergyCapability::powerEnergyMeasurement()` e
  `PowerEnergyCapability::resetEnergy()`;
- capability `Power Energy (W/Wh)` com identificador `pv-power-energy`.

## Hardware, pinos e configuração

| Função | MCB R1 | Símbolo/configuração |
|---|---:|---|
| Saída analógica do ACS712-30A | GPIO 34, ADC1 | `ITS_MCB01_J4_EXT_ADC` |
| Nó ADC do divisor de tensão | GPIO 33, ADC1 | `ITS_MCB01_J4_EXT_IO33` |
| Perfil de corrente | alimentação 3,3 V | `ACS712_30A_3V3` |
| Divisor de tensão | R1 = 330 kΩ; R2 = 10 kΩ | limiar ADC inferior = 144 mV |
| Sensor de corrente | — | `pv-current-source` |
| Sensor de tensão | — | `pv-voltage-source` |
| Capability composta | — | `pv-power-energy`; leitura a cada 1000 ms |
| Apresentação serial | — | `EXAMPLE_POWER_ENERGY_LOG_INTERVAL_MS=1000` |

Os valores numéricos de GPIO aparecem apenas para montagem e diagnóstico. O
código e o environment usam diretamente os símbolos oficiais do pinout. Ambos
os sinais usam ADC1, preservando a operação quando o Wi-Fi estiver ativo.

### Ligação

```text
Painel (+) ──► IP+  ACS712-30A  IP- ──► entrada positiva da carga/buck
                    VCC ──► 3V3 da MCB R1
                    GND ──► GND comum
                    OUT ──► J4 / GPIO 34 (ITS_MCB01_J4_EXT_ADC)

Tensão medida (+) ── R1 330 kΩ ──┬──► J4 / GPIO 33 (ITS_MCB01_J4_EXT_IO33)
                                  │
                              R2 10 kΩ
                                  │
Tensão medida (−) / GND comum ────┴──► GND da MCB R1
```

O ACS712 fica em série com a carga. O divisor mede a tensão entre o positivo
indicado e o mesmo GND da placa. Não conecte sinais sem confirmar polaridade,
faixa, isolamento, potência e tensão nominal dos resistores.

## Lifecycle e resultado esperado

No `setup()`, o exemplo inicializa uma vez o sensor de corrente e depois o de
tensão, registra a capability composta e só então chama `SmartSysApp::setup()`.
Em cada `loop()`, chama uma vez o `handle()` de corrente, o de tensão e, nessa
ordem, `SmartSysApp::handle()`.

O ACS712 permanece sem `supplyMonitorAdcPin`. Portanto, após aquecimento e
calibração, quando tensão e corrente forem numéricas, o estado composto
esperado é `ESTIMATED`, nunca uma afirmação de `VALID` ou de exatidão
contratada. A potência é apresentada com duas casas decimais, a energia com
três e o estado pelo token de `measurementStatus`.

## Build, upload e monitor

```sh
pio run -e example_power_energy_mcb_r1
pio run -e example_power_energy_mcb_r1 -t upload
pio device monitor -e example_power_energy_mcb_r1
```

Upload e operação em hardware devem ser feitos somente em uma bancada
autorizada. O monitor usa 115200 baud. Credenciais e endpoints permanecem na
configuração privada suportada pelo projeto e não integram este exemplo.

## Validação manual

1. Com todo o circuito desenergizado, confira o GND comum, a ligação em série
   do ACS712 e o divisor de 330 kΩ/10 kΩ.
2. Garanta corrente zero para a calibração inicial, grave o firmware e abra o
   monitor serial.
3. Confirme no boot o exemplo, a placa, os dois símbolos e GPIOs, perfil
   `ACS712_30A_3V3`, resistores, limiar, identificadores e intervalo de 1000 ms.
4. Durante o aquecimento de 60 s e a calibração, observe potência ausente e
   estado `NOT_READY`.
5. Aplique tensão acima do limiar do ADC do divisor — aproximadamente 4,90 V
   na entrada com a relação configurada — e uma corrente mensurável.
6. Confirme potência não negativa, energia não decrescente e estado
   `ESTIMATED`; varie a carga e observe a mudança da potência.
7. Envie `r` ou `R` pelo monitor. Confirme o registro do reset local e
   `energy=0.000 Wh` até existir nova integração elegível.
8. Remova ou invalide uma das entradas e confirme potência ausente, estado
   correspondente e ausência de acumulação naquele intervalo.

## Limitações e riscos elétricos

- Energia é volátil e reinicia no boot; o comando `r` é exclusivamente local.
- A capability consome snapshots e não verifica se os sensores foram
  inicializados ou atualizados corretamente.
- O perfil ACS712 de 3,3 V é `PROJECT_VALIDATED`, fora da alimentação
  oficialmente garantida para o componente original; sem monitor de
  alimentação não há exatidão contratada.
- O ADC do ESP32 não tolera sobretensão. Dimensione o divisor para a maior
  tensão possível e considere tolerância, potência, tensão nominal,
  espaçamento, fusível e isolamento apropriados.
- A montagem não é adequada para rede elétrica ou outras tensões perigosas.
  Use fonte limitada e instrumento independente em bancada.
- O ACS712 conduz a corrente da carga: desenergize antes de alterar a montagem
  e dimensione trilhas, fios, conectores e proteção para a corrente aplicada.
