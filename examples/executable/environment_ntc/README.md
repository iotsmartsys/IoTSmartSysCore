# environment_ntc

Demonstra `SensorFactory::createNtcTemperatureSensor()` e
`SmartSysApp::addTemperatureSensorCapability()` com um MF52-103 10 kΩ tratado
empiricamente como B3950. A biblioteca adquire o ADC, calcula a média de 16
amostras, resolve o divisor e aplica a equação Beta; o exemplo apenas configura
e conduz `SmartSysApp`.

## APIs e capability demonstradas

- `NtcTemperatureSensorConfig::MF52_103_B3950()`;
- `SensorFactory::createNtcTemperatureSensor()`;
- `SmartSysApp::addTemperatureSensorCapability()`;
- capability `environment_temperature`.

## Hardware e pinos

| Sinal | MCB R1 | Configuração |
|---|---:|---|
| Nó do divisor | GPIO 34 (J4) | `ITS_MCB01_J4_EXT_ADC` |
| Resistor série | — | 10 kΩ |
| NTC | — | MF52-103 10 kΩ/B3950 empírico |
| Alimentação | — | 3,3 V |

O GPIO 34 é entrada ADC1 e permanece utilizável com o Wi-Fi ativo. O valor
numérico é apenas informativo; código e environment consomem o símbolo oficial.

### Ligação

Monte com a placa desenergizada:

```text
3V3
 │
[resistor série 10 kΩ]
 │
 ├──── J4 / GPIO 34 (ITS_MCB01_J4_EXT_ADC)
 │
[MF52-103 10 kΩ]
 │
GND
```

O NTC deve ficar entre o ADC e GND. Inverta os componentes somente se também
usar outro algoritmo; a orientação inversa não é suportada por este exemplo.
Nunca aplique ao GPIO tensão superior à admitida pelo ESP32 e mantenha GND
comum.

### Alternativa 100 kΩ

Para um NTC 100 kΩ B3950, use
`NtcTemperatureSensorConfig::NTC_100K_B3950()` e troque o resistor série por
100 kΩ. Não misture o preset e o resistor do perfil de 10 kΩ.

## Build, upload e monitor

```sh
pio run -e example_environment_ntc_mcb_r1
pio run -e example_environment_ntc_mcb_r1 -t upload
pio device monitor -e example_environment_ntc_mcb_r1
```

Upload, monitor e hardware devem ser executados somente quando autorizados.
Credenciais e endpoints permanecem na configuração privada do projeto.

## Validação manual

1. Confira o divisor com a placa desenergizada e energize em 3,3 V.
2. Grave o firmware e abra o monitor a 115200 baud.
3. Confirme no boot `id=environment_ntc`, GPIO 34, perfil
   `MF52_103_B3950`, R0 e resistor série de 10 kΩ, Beta 3950, T0 de 25 °C e 16
   amostras.
4. Próximo de 25 °C, confronte a temperatura com um instrumento de referência;
   a resistência calculada deve ficar próxima de 10 kΩ.
5. Aqueça e depois deixe resfriar o NTC. A resistência deve diminuir durante o
   aquecimento e a temperatura deve aumentar, invertendo o movimento no
   resfriamento.
6. Para diagnóstico detalhado, compile com nível de log `DEBUG` e registre
   média ADC fracionária, tensão, resistência e temperatura.

Resultado esperado: a capability publica temperatura válida e acompanha os
estímulos monotonicamente. ADC em zero ou fundo de escala, divisor inválido ou
resultado matemático não finito produzem o sentinel `-1000.0f`, rejeitado pela
capability como temperatura física.

## Limitações

- O MF52-103 foi compatível com B3950 apenas na validação empírica aproximada
  entre 23 °C e 30 °C; isso não garante o lote ou toda a faixa do fabricante.
- O ADC, a tolerância dos resistores e o contato térmico limitam a exatidão.
- Termômetros infravermelhos medem a superfície observada e podem divergir da
  temperatura do encapsulamento do NTC.
- Não há Steinhart–Hart, calibração persistente ou correção geral da não
  linearidade do ADC nesta versão.
