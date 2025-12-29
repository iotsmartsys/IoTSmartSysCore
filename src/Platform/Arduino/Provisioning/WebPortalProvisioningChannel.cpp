#include "Platform/Arduino/Provisioning/WebPortalProvisioningChannel.h"

#include <Arduino.h>
#include <vector>
#include <string>

namespace iotsmartsys::core::provisioning
{

    constexpr uint8_t DNS_PORT = 53;

    WebPortalProvisioningChannel::WebPortalProvisioningChannel(core::WiFiManager &wifiManager, core::ILogger &logger)
        : _wifiManager(wifiManager), _logger(logger), _server(80)
    {
    }

    void WebPortalProvisioningChannel::begin()
    {
        if (_active)
        {
            return;
        }

        _configSaved = false;

        sendStatus(ProvisioningStatus::Idle, "[PortalConfig] Iniciando portal de configuracao...");

        WiFi.mode(WIFI_AP);

#if defined(ESP32)
        String apName = "ESP-Config-" + String((uint32_t)ESP.getEfuseMac(), HEX);
#else
        String apName = "ESP-Config-" + String(ESP.getChipId(), HEX);
#endif

        WiFi.softAP(apName.c_str());

        IPAddress ip = WiFi.softAPIP();
        _logger.info("[PortalConfig]", "Modo de configuracao iniciado.");
        _logger.info("[PortalConfig]", "SSID do AP: %s", apName.c_str());
        _logger.info("[PortalConfig]", "Acesse em: http://%s", ip.toString().c_str());

        _dnsServer.start(DNS_PORT, "*", ip);

        _server.on("/", HTTP_GET, [this]()
                   { handleRoot(); });
        _server.on("/generate_204", HTTP_GET, [this]()
                   { handleRoot(); });
        _server.on("/hotspot-detect.html", HTTP_GET, [this]()
                   { handleRoot(); });
        _server.on("/ncsi.txt", HTTP_GET, [this]()
                   { handleRoot(); });
        _server.on("/connecttest.txt", HTTP_GET, [this]()
                   { handleRoot(); });

        _server.on("/save", HTTP_POST, [this]()
                   { handleSave(); });
        _server.onNotFound([this]()
                           { handleNotFound(); });

        _server.begin();
        _logger.info("[PortalConfig]", "Servidor HTTP iniciado na porta 80.");

        _active = true;
        sendStatus(ProvisioningStatus::WaitingUserInput, "Portal em execucao; aguardando configuracao do usuario");
    }

    void WebPortalProvisioningChannel::loop()
    {
        if (!_active)
        {
            return;
        }

        _dnsServer.processNextRequest();
        _server.handleClient();
    }

    void WebPortalProvisioningChannel::stop()
    {
        if (!_active)
        {
            return;
        }

        _server.stop();
        _dnsServer.stop();
        WiFi.softAPdisconnect(true);

        _active = false;

        _logger.info("[PortalConfig]", "Portal de configuracao finalizado.");
        sendStatus(ProvisioningStatus::Idle, "Portal parado");
    }

    void WebPortalProvisioningChannel::sendStatus(ProvisioningStatus status, const char *msg)
    {
        if (_statusCb)
        {
            _statusCb(status, msg);
        }
        if (msg)
        {
            _logger.info("[PortalConfig]", "%s", msg);
        }
    }

    String WebPortalProvisioningChannel::buildConfigPage()
    {
        String page;
        page += F("<!DOCTYPE html><html lang='pt-BR'><head><meta charset='utf-8'>");
        page += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
        page += F("<title>Configura&#231;&#227;o do Dispositivo</title>");
        page += F("<style>body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Helvetica,Arial,sans-serif;padding:16px;background:#f4f4f4;color:#333;}");
        page += F(".card{max-width:480px;margin:0 auto;background:#fff;border-radius:8px;padding:16px;box-shadow:0 2px 6px rgba(0,0,0,0.1);}");
        page += F("h1{font-size:20px;margin-top:0;margin-bottom:8px;text-align:center;}");
        page += F("p{font-size:13px;color:#555;}");
        page += F("label{display:block;font-size:13px;margin-top:12px;margin-bottom:4px;}");
        page += F("input{width:100%;padding:8px 10px;font-size:14px;border-radius:4px;border:1px solid #ccc;box-sizing:border-box;}");
        page += F("select{width:100%;padding:8px 10px;font-size:14px;border-radius:4px;border:1px solid #ccc;box-sizing:border-box;background:#fff;}");
        page += F("button{margin-top:16px;width:100%;padding:10px 14px;font-size:15px;border:none;border-radius:4px;background:#007aff;color:#fff;cursor:pointer;}");
        page += F("button:active{opacity:.8;}");
        page += F("</style></head><body>");
        page += F("<div class='card'>");
        page += F("<h1>Configura&#231;&#227;o inicial</h1>");
        page += F("<p>Conecte o dispositivo &#224; sua rede Wi-Fi e defina a chave da API do dispositivo.</p>");
        page += F("<form method='POST' action='/save'>");
        page += F("<label for='ssid'>Nome da rede Wi-Fi (SSID)</label>");
        page += F("<select id='ssid' name='ssid' required>");

        std::vector<std::string> ssids = _wifiManager.getAvailableSSIDs();
        if (ssids.empty())
        {
            page += F("<option value=''>Nenhuma rede encontrada</option>");
        }
        else
        {
            for (const auto &s : ssids)
            {
                String ssidStr(s.c_str());
                page += F("<option value='");
                page += ssidStr;
                page += F("'>");
                page += ssidStr;
                page += F("</option>");
            }
        }

        page += F("</select>");
        page += F("<label for='password'>Senha da Wi-Fi</label>");
        page += F("<input type='password' id='password' name='password' required>");
        page += F("<label for='device_api_key'>Device API Key</label>");
        page += F("<input type='text' id='device_api_key' name='device_api_key' required>");
        page += F("<label for='basic_auth'>Basic Auth (user:pass)</label>");
        page += F("<input type='text' id='basic_auth' name='basic_auth' required>");
        page += F("<button type='submit'>Salvar e continuar</button>");
        page += F("</form></div></body></html>");
        return page;
    }

    void WebPortalProvisioningChannel::handleRoot()
    {
        _server.send(200, "text/html", buildConfigPage());
    }

    void WebPortalProvisioningChannel::handleSave()
    {
        const String ssid = _server.arg("ssid");
        const String password = _server.arg("password");
        const String deviceApiKey = _server.arg("device_api_key");
        const String basicAuth = _server.arg("basic_auth");

        _server.send(200, "text/html",
                     F("<!DOCTYPE html><html><head><meta charset='utf-8'>"
                       "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                       "<title>Configura&#231;&#227;o salva</title></head><body>"
                       "<h1>Configura&#231;&#227;o salva!</h1>"
                       "<p>Voc&#234; j&#225; pode fechar esta p&#225;gina. O dispositivo ir&#225; reiniciar ou tentar conectar &#224; rede configurada.</p>"
                       "</body></html>"));

        _configSaved = true;

        _wifiSsidStorage = ssid.c_str();
        _wifiPasswordStorage = password.c_str();
        _deviceApiKeyStorage = deviceApiKey.c_str();
        _basicAuthStorage = basicAuth.c_str();

        if (_configCb)
        {
            DeviceConfig cfg;
            cfg.wifi.ssid = _wifiSsidStorage.c_str();
            cfg.wifi.password = _wifiPasswordStorage.c_str();
            cfg.deviceApiKey = _deviceApiKeyStorage.empty() ? nullptr : _deviceApiKeyStorage.c_str();
            cfg.basicAuth = _basicAuthStorage.empty() ? nullptr : _basicAuthStorage.c_str();
            cfg.source = ProvisioningSource::WebPortal;

            _configCb(cfg);
        }

        sendStatus(ProvisioningStatus::Applied, "[PortalConfig] Configuracao recebida via portal");
    }

    void WebPortalProvisioningChannel::handleNotFound()
    {
        handleRoot();
    }

} // namespace iotsmartsys::core::provisioning
