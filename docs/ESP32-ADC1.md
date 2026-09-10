# ADC1 e presets NTC por modelo de ESP32

O suporte de GPIO usa o mapa `soc/adc_channel.h` do framework selecionado.
`Esp32Adc1Pin` e `isEsp32Adc1Gpio(int)` estão em
`Platform/Arduino/Sensors/Esp32Adc1.h`, na camada Arduino. ADC2 continua
rejeitado por NTC, corrente ACS712 e tensão por divisor, inclusive no pino
opcional de monitoramento da alimentação de corrente.

## Migração dos presets NTC

As assinaturas de `NTC_100K_B3950` e `MF52_103_B3950` agora exigem
`Esp32Adc1Pin`. Não há overload inteiro. No ESP32-C3:

```cpp
using namespace iotsmartsys::platform::arduino;

auto config0 = NtcTemperatureSensorConfig::NTC_100K_B3950(Esp32Adc1Pin::GPIO0);
auto config1 = NtcTemperatureSensorConfig::MF52_103_B3950(Esp32Adc1Pin::GPIO1);
auto sensor = sensorFactory.createNtcTemperatureSensor(config0);
```

`Esp32Adc1Pin::GPIO5` não existe no C3, pois GPIO5 pertence a ADC2.
`NTC_100K_B3950(0)` deixa de compilar. O mesmo nome de enum expõe os membros
correspondentes ao SoC de cada build; no clássico, por exemplo, há `GPIO34`.
Isso é uma quebra explícita de fonte restrita aos dois presets NTC.

Os campos `adcPin` continuam inteiros e as APIs de criação dos sensores mantêm
suas assinaturas. Para configuração dinâmica, preencha os campos da struct,
incluindo resistências e Beta, e passe-a ao factory. Ela passa pela validação
em runtime. Um cast explícito pode fabricar um valor de enum em C++; por isso
a validação de configuração não foi removida. O construtor direto do NTC
continua protegendo aquisição com configuração inválida.

Para adaptar um símbolo de placa, confira o mapa antes do cast:

```cpp
static_assert(isEsp32Adc1Gpio(ITS_MCB01_J4_EXT_ADC), "Board pin must be ADC1");
auto config = NtcTemperatureSensorConfig::MF52_103_B3950(
    static_cast<Esp32Adc1Pin>(ITS_MCB01_J4_EXT_ADC));
```

A verificação em runtime também respeita a disponibilidade de ADC informada
pelo Arduino e seus limites de numeração, antes de converter para `uint8_t`.
O enum usa GPIO físico, não número do conector ou alias Arduino. Um GPIO ADC1
pode não estar exposto no módulo ou estar ocupado pelo circuito da placa;
pertencer ao enum não reserva o pino nem garante sua disponibilidade elétrica.

## Mapas e evidência de construção

Mapas conferidos nos headers do SDK local que acompanha Arduino-ESP32 3.3.11
(ESP-IDF v5.5.5). O C3 também foi construído com Arduino 2.0.14, plataforma
`platformio/espressif32@6.5.0`.

| SoC | GPIOs ADC1 | Construção Arduino 3.3.11 |
|---|---|---|
| ESP32 clássico | 32–39 | 6 componentes |
| ESP32-C3 | 0–4 | 6 componentes; também 6 no Arduino 2.0.14 |
| ESP32-S2 | 1–10 | 6 componentes |
| ESP32-S3 | 1–10 | 6 componentes |
| ESP32-C5 | 1–6 | 6 componentes |
| ESP32-C6 | 0–6 | 6 componentes |
| ESP32-H2 | 1–5 | 5 componentes; builder falha por dependência preexistente de Wi-Fi |
| ESP32-P4 | 16–23 | 6 componentes |

Os seis componentes são NTC, ACS712, divisor resistivo,
`ArduinoHardwareAdapterFactory`, `SensorFactory` e `CapabilitiesBuilder`.
No H2, a falha do builder ocorre em `WiFiManager.h`, que exige `WiFiEvent_t`,
`WiFiEventInfo_t`, `wl_status_t` e `WiFiEventId_t`. A compilação do builder da
baseline anterior reproduz essa falha. Não foi acrescentada conectividade ao H2.

C2/C61 não possuem SDK utilizável nesta instalação: construção **Not Executed**.
O código deriva os membros dos canais presentes no header oficial, sem mapear
um SoC desconhecido para os GPIOs do clássico. SoCs futuros não são considerados
validados por esse mecanismo. A ausência de mapa ADC1 produz rejeição.

Compilação de componentes produz objetos `.o`, sem link do runtime, firmware,
upload ou execução. Os builds completos do clássico `esp32_dev` e dos exemplos
`environment_ntc`, `current_sensor` e `voltage_sensor` passaram. Nenhum teste
ou ensaio físico foi executado nesta implementação. Os estados detalhados,
tentativas e limites constam do relatório de implementação.

## Resolução, atenuação e configuração elétrica

O NTC preserva resolução configurável de 9 a 12 bits e default de 12. Nos
outros chips, o Arduino ajusta a resolução retornada; isso não afirma mudança
na resolução física. Corrente/tensão continuam solicitando 12 bits.

No clássico e S3, o NTC seleciona 0/2,5/6 dB até os coeficientes de referência
0,95/1,25/1,75 V, respectivamente. No S2/C3, usa 0,75/1,05/1,30 V. Acima disso,
usa a atenuação máxima. Nos demais targets, usa a máxima para todas as
referências, sem herdar limites intermediários do clássico. Corrente/tensão
usam sempre `FULL_RANGE`, representado por `ADC_11db` no Arduino; no SDK 5,
11 dB é alias de 12 dB. A disponibilidade foi conferida por compilação.

O Arduino documenta faixa de entrada até 2,5 V no S2/C3 e até 3,1 V no
clássico/S3 na atenuação máxima. Para corrente/tensão no S2/C3, configure
explicitamente `adcMaximumMv` compatível com essa faixa, em vez de assumir
que o default clássico de 3100 mV é utilizável. Confira também `adcMinimumMv`,
zero, sensibilidade e divisor do circuito. Nos C5/C6/H2/P4, determine os
limites no datasheet do chip/revisão e no circuito; esta entrega não atribui
3100 mV a esses targets como perfil validado.

No NTC, `supplyVoltageV` é a alimentação real do divisor;
`adcReferenceVoltageV` é o coeficiente usado para converter contagem bruta em
volts. Seu default 3,3 V foi preservado por contrato e **não é calibração do
ADC**. Ajuste esse coeficiente conforme caracterização do circuito/ADC. Copiar
o máximo da faixa calibrada para ele não comprova uma relação linear correta.
Não houve troca para leitura calibrada em milivolts, alteração da equação Beta,
dos presets térmicos ou da média de 16 amostras. A aceitação do GPIO e a
exatidão de temperatura são evidências distintas.

Fontes: [Arduino ADC](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html),
[atenuação ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/v5.4.4/esp32p4/api-reference/peripherals/adc_oneshot.html),
[datasheet C3](https://documentation.espressif.com/esp32-c3_datasheet_en.html).

## Reprodução dos builds de componentes

`configs/adc1_build.ini` define os ambientes. Para usar o PlatformIO:

```sh
python3 tools/build_adc1_components.py adc1_c3_arduino2
```

Para Arduino 3, o script também pode usar fontes, SDK e compiladores já
existentes, sem acionar instalação de toolchain pelo PlatformIO:

```sh
python3 tools/build_adc1_components.py \
  --arduino3-source /path/to/arduino-esp32-3.3.11 \
  --arduino3-sdk /path/to/framework-arduinoespressif32-libs \
  --toolchains /path/to/existing-toolchains \
  --boards /path/to/platform-espressif32/boards
```

A opção direta usa os response files oficiais de flags/defines/includes do
SDK, variante da board, C++17 e dependências locais do build `esp32_dev`.
Registra versão do compilador, comandos JSON, objetos e logs em
`.pio/adc1-components/`. Usa uma configuração de memória do SDK para construir
componentes; não representa um firmware de placa com PSRAM/conectividade.
O retorno agregado é diferente de zero quando qualquer componente falha,
inclusive a limitação conhecida do builder H2. Isso não executa testes.
