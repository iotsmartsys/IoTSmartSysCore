#pragma once
#include <cstdint>

namespace iotsmartsys::core::provisioning
{

    /// @brief Identifica a origem da configuração de provisionamento.
    enum class ProvisioningSource : uint8_t
    {
        Unknown,
        Ble,
        WebPortal,
        Serial,
        FactoryDefault
    };

    /// @brief Credenciais de Wi-Fi básicas.
    struct WifiCredentials
    {
        const char *ssid = nullptr;
        const char *password = nullptr;
    };

    /// @brief Configuração completa de provisionamento do dispositivo.
    struct DeviceConfig
    {
        WifiCredentials wifi;
        const char *deviceApiKey = nullptr;
        const char *basicAuth = nullptr;
        ProvisioningSource source = ProvisioningSource::Unknown;
    };

} // namespace iotsmartsys::core::provisioning