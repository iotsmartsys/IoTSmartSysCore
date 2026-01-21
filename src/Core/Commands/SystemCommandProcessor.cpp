#include "SystemCommandProcessor.h"
#include "Arduino.h"

#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#else
#include "esp_wifi.h"
#include "esp_system.h"
#include "driver/gpio.h"
#endif

namespace iotsmartsys::core
{
    SystemCommandProcessor::SystemCommandProcessor(ILogger &logger)
        : _logger(logger)
    {
    }

    bool SystemCommandProcessor::process(const DeviceCommand &command)
    {
        _logger.warn("SystemCommandProcessor: Processing system command.");
        switch (command.getSystemCommand())
        {
        case SystemCommands::REBOOT:
            _logger.warn("SystemCommandProcessor: Executing REBOOT command.");
            delay(1000); // Pequeno atraso para garantir que o log seja enviado
            full_soft_powercycle_restart();
            return true;
            break;
        case SystemCommands::FACTORY_RESET:
            _logger.info("SystemCommandProcessor: Executing FACTORY_RESET command.");
            /* code for factory reset */
            return true;
            break;
        case SystemCommands::UPDATE_FIRMWARE:
            _logger.info("SystemCommandProcessor: Executing UPDATE_FIRMWARE command.");
            /* code for update firmware */
            return true;
            break;
        default:
            _logger.warn("SystemCommandProcessor: Unknown system command received.");
            return false;
            break;
        }
    }

    void SystemCommandProcessor::reset_all_gpio_safely()
    {
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
        // ESP8266: no generic GPIO reset API like ESP32 IDF.
        // Keep as a no-op to preserve call sites.
#else
        // Ajuste se você quiser excluir pinos usados por USB/UART/etc.
        for (int pin = 0; pin < GPIO_NUM_MAX; pin++)
        {
            // Alguns pinos podem não existir/ser inválidos em certas variantes
            gpio_num_t g = (gpio_num_t)pin;
            gpio_reset_pin(g);
            gpio_set_direction(g, GPIO_MODE_DISABLE); // "hi-z" / desabilita saída
        }
#endif
    }

    void SystemCommandProcessor::full_soft_powercycle_restart()
    {
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
        // ESP8266: best-effort shutdown of Wi-Fi before restart.
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        delay(50);
        ESP.restart();
#else
        esp_wifi_stop();
        esp_wifi_deinit();

        reset_all_gpio_safely();

        esp_restart();
#endif
    }

    void SystemCommandProcessor::restartSafely()
    {
        _logger.warn("SystemCommandProcessor: Performing safe restart.");
        full_soft_powercycle_restart();
    }
} // namespace iotsmartsys::core