#include "Core/CommandExecutor.h"
#include "Utils/Logger.h"
#include "Settings/ConfigManager.h"
#include "Wifi/WifiHelper.h"

void applyCommandToCapabilities(const CapabilityCommand &command,
                                std::vector<Capability *> &capabilities,
                                bool &power_on,
                                MqttClientHandler *mqttHandler)
{
    const char *cmd = command.capability_name.c_str();

    if (strcmp(cmd, "power") == 0)
    {
        if (command.value == POWER_ON_COMMAND)
        {
            power_on = true;
            LOG_PRINTLN("Ligando o dispositivo...");
        }
        else if (command.value == POWER_OFF_COMMAND)
        {
            power_on = false;
            LOG_PRINTLN("Desligando o dispositivo...");
        }
        else if (command.value == "restart")
        {
            LOG_PRINTLN("Reiniciando o dispositivo...");
            ESP.restart();
        }
        return;
    }
    else if (command.capability_name == "configure")
    {
        ConfigManager::instance().enterConfigMode();
        CapabilityState state(getDeviceId(), "device_state", "in_configuration_mode");
        mqttHandler->sendState(state);
        return;
    }

    if (!power_on)
    {
        LOG_PRINTLN("Dispositivo está desligado. Ignorando comando.");
        return;
    }

    LOG_PRINTLN("Aplicando comando na capability: " + command.capability_name + " com valor: " + command.value);
    for (auto cap : capabilities)
    {
        LOG_PRINTLN("Verificando capability: " + cap->capability_name);
        LOG_PRINTLN("Comparando com: " + command.capability_name);
        if (cap->capability_name == command.capability_name)
        {
            LOG_PRINTLN("Executando comando na capability: " + cap->capability_name);
            cap->execute(command.value);
        }
    }
}
