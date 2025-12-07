Sketch: rgb_esp32s3_party.ino

Descrição:
Este sketch transforma os LEDs RGB em uma "luz de festa" contínua, transicionando suavemente pelas cores.

Uso:
- Coloque este arquivo na pasta `examples/` do projeto.
- Se sua placa tem pino vermelho disponível, edite `RED_PIN` no topo do arquivo e coloque o número do pino (por exemplo 45).
- Compile e faça upload usando PlatformIO ou Arduino IDE selecionando uma placa ESP32-S3 compatível.

Atenção:
- A placa de desenvolvimento ESP32-S3 deste repositório define apenas os pinos GREEN=43 e BLUE=44 por padrão. Nem sempre existe RED onboard. Se não houver pino vermelho, o sketch usará apenas G e B.
