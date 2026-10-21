# Interpreters de comandos IR

## AVANT-LS963231305

O `AvantLS963231305ControlInterpreter` converte os oito códigos do controle
AVANT-LS963231305 em textos, conforme a
[especificação 0.1](specs/AVANT-LS963231305-IR-COMMAND.md).
O consumo é explícito; nenhum modelo padrão da aplicação é alterado.

```cpp
#include "Platform/Arduino/Interpreters/AvantLS963231305ControlInterpreter.h"

using iotsmartsys::core::IRCommand;
using iotsmartsys::platform::arduino::AvantLS963231305ControlInterpreter;
using iotsmartsys::platform::arduino::AvantLS963231305ControlInterpreterConstants;

AvantLS963231305ControlInterpreter interpreter;
IRCommand command{true, 0xFF22DD, "NEC"};
std::string result = interpreter.interpret(command);
bool isOnCommand =
    result == AvantLS963231305ControlInterpreterConstants::ON_COMMAND;
```

O exemplo representa uma chamada local, sem receptor físico. `isOnCommand`
indica a tecla reconhecida, não o estado confirmado de um aparelho.
A aplicação decide o que fazer com o resultado.

| Tecla | Código | Constante pública | Retorno |
|---|---|---|---|
| ON | `0xFF22DD` | `ON_COMMAND` | `"On"` |
| OFF | `0xFFC23D` | `OFF_COMMAND` | `"Off"` |
| MODE | `0xFFE01F` | `MODE_COMMAND` | `"Mode"` |
| AUTO | `0xFF906F` | `AUTO_COMMAND` | `"Auto"` |
| 30MIN | `0xFF6897` | `TIMER_30MIN_COMMAND` | `"30Min"` |
| 2H | `0xFFB04F` | `TIMER_2H_COMMAND` | `"2H"` |
| 5H | `0xFF10EF` | `TIMER_5H_COMMAND` | `"5H"` |
| 10H | `0xFF5AA5` | `TIMER_10H_COMMAND` | `"10H"` |

As constantes pertencem a `AvantLS963231305ControlInterpreterConstants`.
O membro `iotsmartsys::core::IRControlModel::AVANT_LS963231305` identifica o
modelo no enum; não cria uma instância ou seleciona um factory.

### Contrato e limites

- A correspondência usa o valor integral de `IRCommand::code` (`uint64_t`).
  `type` e `triggered` não influenciam o retorno.
- NEC e 32 bits descrevem a captura informada. O interpreter não valida
  protocolo ou largura; `IRCommand` não contém a quantidade de bits.
- Qualquer código não listado retorna `"unknown"`, incluindo `0`,
  `0xFFFFFFFFFFFFFFFFULL` e `0x100FF22DD`. Não há truncamento para 32 bits.
- O interpreter não modifica o comando, marca leitura, retém estado ou
  suprime chamadas repetidas. O consumo de `triggered` e a marcação de leitura
  permanecem com a aplicação e os componentes existentes.
- A repetição NEC bruta não é interpretada como a última tecla. Quando usado,
  o sensor Arduino mantém seu tratamento de repetição antes da interpretação.
- `Mode`, `Auto` e os textos de duração não executam modos, automações ou
  temporizadores. Não há transmissão IR, publicação ou atuação em dispositivos.
- Códigos compartilhados com IoTSM1541 não identificam automaticamente um
  modelo; cada instância preserva seu próprio mapeamento.

### Construção

O componente segue C++17 no runtime Arduino sobre ESP32 e não depende
diretamente de `IRremoteESP8266`, logger ou sensor.

```sh
pio run -e esp32_dev
```

O build compila o componente sem trocar a aplicação padrão. Não representa
teste comportamental, recepção física ou autorização de upload.
