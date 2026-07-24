# environment_dht

Demonstra `SensorFactory`, `addTemperatureSensorCapability()` e `addHumiditySensorCapability()` com um DHT11. O exemplo chama `DHTSensor::handle()` periodicamente antes de `SmartSysApp::handle()`.

## Hardware e pinos

| DHT11 | MCB R1 | Configuração |
|---|---:|---|
| VCC | 3,3 V | alimentação |
| DATA | GPIO 23 | `ITS_MCB01_TEMPERATURE_SENSOR_PIN` |
| GND | GND | referência |

Ligação textual: `3V3 — DHT11 VCC`, `GPIO23 — DHT11 DATA`, `GND — DHT11 GND`. Use resistor pull-up de 4,7 kΩ a 10 kΩ entre DATA e 3,3 V se o módulo não o incluir. Não alimente o sinal com 5 V.

O environment define DHT11 e intervalo de leitura de 2000 ms. Credenciais e endpoints devem permanecer na configuração privada suportada pelo projeto.

## Build, upload e monitor

```sh
pio run -e example_environment_dht_mcb_r1
pio run -e example_environment_dht_mcb_r1 -t upload
pio device monitor -e example_environment_dht_mcb_r1
```

## Validação manual

1. Faça a ligação com a placa desenergizada.
2. Grave o firmware e abra o monitor a 115200 baud.
3. Confirme `id=environment_dht`, `dht_model=DHT11`, GPIO 23 resolvido pelo símbolo oficial `ITS_MCB01_TEMPERATURE_SENSOR_PIN` e intervalo de 2000 ms no boot.
4. Conclua o provisioning, se necessário; estabilize o sensor e depois aqueça-o suavemente com a mão e aproxime uma fonte leve de umidade, sem molhá-lo.
5. Observe e registre as atualizações das capabilities `environment_temperature` e `environment_humidity` na integração configurada.

Resultado esperado: ambas as leituras tornam-se válidas e respondem aos estímulos. Leituras `NaN`, zero persistente ou ausência de atualização indicam ligação, alimentação ou sensor incompatível. O DHT11 é lento e não é instrumento de precisão.
