# Exemplos do IoTSmartSysCore

Os exemplos executáveis são aplicações Arduino reais selecionadas por environment do PlatformIO. Não copie nem renomeie fontes: escolha o environment desejado.

| Exemplo | Environment | Hardware |
|---|---|---|
| [`basic_light`](executable/basic_light/README.md) | `example_basic_light_mcb_r1` | saída/relé GPIO 26 da MCB R1 |
| [`environment_dht`](executable/environment_dht/README.md) | `example_environment_dht_mcb_r1` | DHT11 no GPIO 23 da MCB R1 |
| [`environment_ntc`](executable/environment_ntc/README.md) | `example_environment_ntc_mcb_r1` | MF52-103 10 kΩ em divisor no GPIO 34 (J4) da MCB R1 |
| [`screen_console`](executable/screen_console/README.md) | `example_screen_console_esp32_dev` | Ideaspark ESP32 1.9 inch TFT LCD, ST7789 170 × 320 |
| [`current_sensor`](executable/current_sensor/README.md) | `example_current_sensor_mcb_r1` | ACS712-30A no GPIO 34 (J4) da MCB R1 |
| [`voltage_sensor`](executable/voltage_sensor/README.md) | `example_voltage_sensor_mcb_r1` | divisor resistivo no GPIO 34 (J4) da MCB R1 |

Fluxo comum:

```sh
pio run -e <environment>
pio run -e <environment> -t upload
pio device monitor -e <environment>
```

Cada environment herda o perfil de runtime da placa correspondente, exclui apenas `src/main.cpp`, habilita o runner e seleciona exatamente uma aplicação. Para incluir um novo exemplo:

1. crie `examples/executable/<id>/example.hpp` com um único `setup()`/`loop()` e um README completo;
2. adicione um seletor mutuamente exclusivo em `src/ExecutableExampleRunner.cpp`;
3. crie um environment estável em `configs/executable_examples.ini`, com dependências e flags explícitas; para placas com pinout normativo, use diretamente seus símbolos oficiais em vez de redefinir pinos no environment;
4. adicione o environment à matriz de CI e execute seu procedimento de hardware.

Propriedades locais como porta serial e toda credencial permanecem fora do catálogo versionado. Os exemplos legados nas demais pastas são apenas material histórico e não possuem o contrato executável deste catálogo.
