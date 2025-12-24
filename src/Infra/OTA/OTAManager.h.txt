#pragma once

#include <Arduino.h>
#include "Infra/Settings/Models/Settings.h"

namespace OTA
{
    // Carrega settings (via ConfigManager), configura o fwUpdater global e decide
    // se executa a verificação automática. Chame isto durante setup do sistema.
    void setup(FirmwareConfig settings);

    void update(FirmwareConfig settings);
}
