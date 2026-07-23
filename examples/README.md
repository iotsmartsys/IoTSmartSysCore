# Exemplos do IoTSmartSysCore

Os exemplos executáveis são aplicações Arduino reais selecionadas por environment do PlatformIO. Não copie nem renomeie fontes: escolha o environment desejado.

| Exemplo | Environment | Hardware |
|---|---|---|
| [`basic_light`](executable/basic_light/README.md) | `example_basic_light_mcb_r1` | saída/relé GPIO 26 da MCB R1 |
| [`environment_dht`](executable/environment_dht/README.md) | `example_environment_dht_mcb_r1` | DHT11 no GPIO 23 da MCB R1 |

Fluxo comum:

```sh
pio run -e <environment>
pio run -e <environment> -t upload
pio device monitor -e <environment>
```

Cada environment herda o perfil `example_board_iotsmartsys_mcb_r1`, exclui apenas `src/main.cpp`, habilita o runner e seleciona exatamente uma aplicação. Para incluir um novo exemplo:

1. crie `examples/executable/<id>/example.hpp` com um único `setup()`/`loop()` e um README completo;
2. adicione um seletor mutuamente exclusivo em `src/ExecutableExampleRunner.cpp`;
3. crie um environment estável em `configs/executable_examples.ini`, com pinos, dependências e flags explícitos;
4. adicione o environment à matriz de CI e execute seu procedimento de hardware.

Propriedades locais como porta serial e toda credencial permanecem fora do catálogo versionado. Os exemplos legados nas demais pastas são apenas material histórico e não possuem o contrato executável deste catálogo.
