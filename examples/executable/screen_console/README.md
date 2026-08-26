# screen_console

Demonstra o console de diagnóstico ST7789 e o `ScreenMirrorLogger`: uma única mensagem de boot é enviada ao logger serial corrente e espelhada no display integrado da placa Ideaspark ESP32 1.9 inch TFT LCD.

## Hardware, configuração e pinout

O environment usa a definição genérica `esp32dev` com o painel integrado ST7789 de 170 × 320 pixels. Os valores abaixo pertencem somente a este exemplo; não são defaults da implementação de plataforma.

| Sinal | GPIO | Macro do exemplo |
|---|---:|---|
| LCD CS | 15 | `EXAMPLE_SCREEN_CS_PIN` |
| LCD DC | 2 | `EXAMPLE_SCREEN_DC_PIN` |
| LCD RST | 4 | `EXAMPLE_SCREEN_RESET_PIN` |
| LCD SCLK | 18 | `EXAMPLE_SCREEN_CLOCK_PIN` |
| LCD MOSI | 23 | `EXAMPLE_SCREEN_MOSI_PIN` |
| LCD backlight | 32 | `EXAMPLE_SCREEN_BACKLIGHT_PIN` |
| Largura nativa | 170 px | `EXAMPLE_SCREEN_NATIVE_WIDTH` |
| Altura nativa | 320 px | `EXAMPLE_SCREEN_NATIVE_HEIGHT` |
| Rotação | 1 (paisagem) | `EXAMPLE_SCREEN_ROTATION` |

A configuração usa rotação 1, produzindo área lógica horizontal de 320 × 170 pixels, texto tamanho 1, margem horizontal de 2 pixels, espaçamento de 1 pixel, fundo preto, texto padrão branco e backlight ativo em nível alto.

## Montagem

O LCD é integrado à placa: não faça pontes externas entre o ESP32 e o painel. Com a placa desenergizada, inspecione se não há curto, alimente-a somente pela entrada USB prevista pelo fabricante e não conecte periféricos aos GPIOs 2, 4, 15, 18, 23 ou 32 durante esta demonstração.

## Build, upload e monitor

```sh
pio run -e example_screen_console_esp32_dev
pio run -e example_screen_console_esp32_dev -t upload
pio device monitor -e example_screen_console_esp32_dev
```

O upload requer que a porta da placa esteja disponível e corretamente configurada no ambiente local. O exemplo não contém nem imprime credenciais.

## Validação manual

1. Com a placa desenergizada, confirme o modelo Ideaspark ESP32 1.9 inch TFT LCD e a ausência de conexões conflitantes nos GPIOs reservados ao painel.
2. Conecte a placa por USB, grave o environment próprio e abra o monitor serial a 115200 baud.
3. Aguarde a inicialização do runtime e localize a mensagem com `id=screen_console`, placa, controlador, dimensões e todos os pinos.
4. Confirme que a mesma mensagem de nível `Info` aparece na serial e, em orientação horizontal, quebrada em linhas quando necessário na base da área útil do display em branco.
5. Mantenha o dispositivo em execução e confirme que o loop cooperativo permanece responsivo, sem escrita contínua no painel.

Resultado esperado: o build seleciona apenas este exemplo; após o boot, a mensagem diagnóstica aparece na serial e no display em orientação horizontal, com seu trecho mais recente na base da tela. O `SmartSysApp::handle()` continua sendo chamado no `loop()`.

## Limitações e riscos

A renderização SPI é síncrona e destina-se a diagnóstico, não a caminhos quentes. A validação visual depende do lote do painel, alimentação, orientação e compatibilidade real do controlador. Upload e confirmação física devem ser executados apenas com autorização operacional e hardware disponível; o build, isoladamente, não comprova comportamento elétrico ou visual.
