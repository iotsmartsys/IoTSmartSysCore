#pragma once

#include <functional>
#include <string>
#if defined(ESP32)
#include <WebServer.h>
#include <DNSServer.h>
using HttpServer = WebServer;
#elif defined(ESP8266)
#include <ESP8266WebServer.h>
#include <DNSServer.h>
using HttpServer = ESP8266WebServer;
#endif

#include "Core/Provisioning/IProvisioningChannel.h"
#include "Contracts/Connections/WiFiManager.h"

namespace iotsmartsys::core::provisioning
{

    /// @brief Canal de provisionamento baseado em portal web (captive portal via Wi-Fi AP).
    class WebPortalProvisioningChannel : public IProvisioningChannel
    {
    public:
        /// @brief Cria uma nova instância do canal de portal web.
        WebPortalProvisioningChannel(core::WiFiManager &wifiManager);

        /// @brief Inicia o portal de configuração em modo AP.
        void begin() override;

        /// @brief Processa requisições DNS/HTTP do portal de configuração.
        void loop() override;

        /// @brief Encerra o portal de configuração e o AP.
        void stop() override;

        /// @brief Indica se o portal está ativo.
        bool isActive() const override { return _active; }

        /// @brief Define o callback chamado quando uma nova configuração é recebida.
        void onConfigReceived(ConfigCallback cb) override { _configCb = cb; }

        /// @brief Define o callback chamado quando o status do canal muda.
        void onStatusChanged(StatusCallback cb) override { _statusCb = cb; }

        /// @brief Prioridade do canal de portal web.
        uint8_t priority() const override { return 50; }

    private:
        bool _active = false;
        bool _configSaved = false;

        core::WiFiManager &_wifiManager;
        HttpServer _server;
        DNSServer _dnsServer;

        ConfigCallback _configCb = nullptr;
        StatusCallback _statusCb = nullptr;

        std::string _wifiSsidStorage;
        std::string _wifiPasswordStorage;
        std::string _deviceApiKeyStorage;
        std::string _basicAuthStorage;

        void sendStatus(ProvisioningStatus status, const char *msg);

        void handleRoot();
        void handleSave();
        void handleNotFound();

        String buildConfigPage();
    };

} // namespace iotsmartsys::core::provisioning