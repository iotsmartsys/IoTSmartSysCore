#include "Platform/Arduino/Provisioning/WebPortalProvisioningChannel.h"
#include "Platform/Arduino/Provisioning/ProvisioningJsonExtractor.h"

#if defined(WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE) && (WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE != 0)

#include <Arduino.h>
#include <vector>
#include <string>

#ifndef WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE
#define WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE 0
#endif

namespace iotsmartsys::core::provisioning
{

    constexpr uint8_t DNS_PORT = 53;

    WebPortalProvisioningChannel::WebPortalProvisioningChannel(core::WiFiManager &wifiManager, core::ILogger &logger, core::IDeviceIdentityProvider &deviceIdentityProvider)
        : _wifiManager(wifiManager), _logger(logger),
         _server(80),
          _deviceIdentityProvider(deviceIdentityProvider)
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

        // AP+STA allows scanning nearby networks while SoftAP is running
        WiFi.mode(WIFI_AP_STA);
        auto deviceId = _deviceIdentityProvider.getDeviceID();

        unsigned long last6 = strtoul(deviceId.substr(deviceId.length() - 6).c_str(), nullptr, 16);
        char ssidBuf[32];
        char passBuf[32];
        snprintf(ssidBuf, sizeof(ssidBuf), "iotsmartsys-%06lX", last6);
        snprintf(passBuf, sizeof(passBuf), "setup-%06lX", last6);
        String apName = String(ssidBuf);
        String apPass = "123456789"; // String(passBuf);

        // WPA2 password must be >= 8 chars; using setup-XXXXXX (11 chars)
        WiFi.softAP(apName.c_str(), apPass.c_str());

        IPAddress ip = WiFi.softAPIP();
        _availableSsids = _wifiManager.getAvailableSSIDs();
       // _logger.info("[PortalConfig]", "Modo de configuracao iniciado.");
       // _logger.info("[PortalConfig]", "DEVICE ID: %s", deviceId.c_str());
       // _logger.info("[PortalConfig]", "Acesse em: http://%s", ip.toString().c_str());
       // _logger.info("[PortalConfig]", "SSID: %s", apName.c_str());
       // _logger.info("[PortalConfig]", "Senha: %s", apPass.c_str());

#if defined(WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE) && (WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE != 0)
        _dnsServer.start(DNS_PORT, "*", ip);
#endif

        _server.on("/", HTTP_GET, [this]()
                   { handleRoot(); });
#if defined(WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE) && (WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE != 0)
        _server.on("/generate_204", HTTP_GET, [this]()
                   { handleRoot(); });
        _server.on("/hotspot-detect.html", HTTP_GET, [this]()
                   { handleRoot(); });
        _server.on("/ncsi.txt", HTTP_GET, [this]()
                   { handleRoot(); });
        _server.on("/connecttest.txt", HTTP_GET, [this]()
                   { handleRoot(); });
#endif

        _server.on("/save", HTTP_POST, [this]()
                   { handleSave(); });

        _server.on("/info", HTTP_GET, [this, apName]()
                   {
                       // {"device_id":"<id>", "ssid":"<ssid>"}
                       String info = F("{\"device_id\":\"");
                       info += _deviceIdentityProvider.getDeviceID().c_str();
                       info += F("\", \"ssid\":\"");
                       info += apName;
                       info += F("\"}");
                       _server.send(200, "application/json", info); });

        _server.on("/wifi/scan", HTTP_GET, [this]()
                   {
                       // Refresh scan results on each request
                       _availableSsids = _wifiManager.getAvailableSSIDs();

                       String result = F("[");
                       for (size_t i = 0; i < _availableSsids.size(); i++)
                       {
                           result += F("\"");
                           result += String(_availableSsids[i].c_str());
                           result += F("\"");
                           if (i < _availableSsids.size() - 1)
                           {
                               result += F(",");
                           }
                       }
                       result += F("]");
                       _server.send(200, "application/json", result); });
        _server.onNotFound([this]()
                           { handleNotFound(); });

        _server.begin();
       // _logger.info("[PortalConfig]", "Servidor HTTP iniciado na porta 80.");

        _active = true;
        sendStatus(ProvisioningStatus::WaitingUserInput, "Portal em execucao; aguardando configuracao do usuario");
    }

    void WebPortalProvisioningChannel::loop()
    {
        if (!_active)
        {
            return;
        }

#if defined(WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE) && (WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE != 0)
        _dnsServer.processNextRequest();
#endif
        _server.handleClient();
    }

    void WebPortalProvisioningChannel::stop()
    {
        if (!_active)
        {
            return;
        }

        _server.stop();
#if defined(WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE) && (WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE != 0)
        _dnsServer.stop();
#endif
        WiFi.softAPdisconnect(true);

        _active = false;

       // _logger.info("[PortalConfig]", "Portal de configuracao finalizado.");
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
           // _logger.info("[PortalConfig]", "%s", msg);
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

        if (_availableSsids.empty())
        {
            page += F("<option value=''>Nenhuma rede encontrada</option>");
        }
        else
        {
            for (const auto &s : _availableSsids)
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
       // _logger.info("[PortalConfig]", "Recebendo configuracao via portal...");
        String ssid;
        String password;
        String deviceApiKey;
        String basicAuth;
        String device_api_url;

        // If the request body is JSON (iOS app), parse it from the raw body.
        // Otherwise, fall back to form args (HTML portal).
        const String rawBody = _server.arg("plain");
        const bool looksLikeJson = rawBody.length() > 0 && rawBody[0] == '{';

        if (looksLikeJson)
        {
            ProvisioningJsonFields fields;
            if (!ProvisioningJsonExtractor::tryParse(rawBody, fields))
            {
               // _logger.warn("[PortalConfig]", "JSON parse failed in /save");
                _server.send(400, "application/json", "{\"error\":\"invalid_json\"}");
                return;
            }

            ssid = fields.ssid;
            password = fields.password;
            deviceApiKey = fields.deviceApiKey;
            basicAuth = fields.basicAuth;
            device_api_url = fields.deviceApiUrl;

           // _logger.info("[PortalConfig]", "ssid: %s", ssid.c_str());
           // _logger.info("[PortalConfig]", "password: %s", password.c_str());
           // _logger.info("[PortalConfig]", "device_api_key: %s", deviceApiKey.c_str());
           // _logger.info("[PortalConfig]", "basic_auth: %s", basicAuth.c_str());
           // _logger.info("[PortalConfig]", "device_api_url: %s", device_api_url.c_str());

            if (ssid.isEmpty())
            {
                _server.send(400, "application/json", "{\"error\":\"missing_ssid\"}");
                return;
            }
            // Password may be empty for open networks; accept empty.
        }
        else
        {
            // HTML form post (application/x-www-form-urlencoded)
            ssid = _server.arg("ssid");
            password = _server.arg("password");
            deviceApiKey = _server.arg("device_api_key");
            device_api_url = _server.arg("device_api_url");
            basicAuth = _server.arg("basic_auth");

            if (ssid.isEmpty())
            {
                _server.send(400, "text/plain", "missing ssid");
                return;
            }
        }

        if (looksLikeJson)
        {
            _server.send(200, "application/json", "{\"ok\":true}");
        }
        else
        {
            _server.send(200, "text/html",
                         F("<!DOCTYPE html><html><head><meta charset='utf-8'>"
                           "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                           "<title>Configura&#231;&#227;o salva</title></head><body>"
                           "<h1>Configura&#231;&#227;o salva!</h1>"
                           "<p>Voc&#234; j&#225; pode fechar esta p&#225;gina. O dispositivo ir&#225; reiniciar ou tentar conectar &#224; rede configurada.</p>"
                           "</body></html>"));
        }

        _configSaved = true;

        _wifiSsidStorage = ssid.c_str();
        _wifiPasswordStorage = password.c_str();
        _deviceApiKeyStorage = deviceApiKey.c_str();
        _deviceApiUrlStorage = device_api_url.c_str();
        _basicAuthStorage = basicAuth.c_str();

        if (_configCb)
        {
            DeviceConfig cfg;
            cfg.wifi.ssid = _wifiSsidStorage.c_str();
            cfg.wifi.password = _wifiPasswordStorage.c_str();
            cfg.deviceApiKey = _deviceApiKeyStorage.empty() ? nullptr : _deviceApiKeyStorage.c_str();
            cfg.deviceApiUrl = _deviceApiUrlStorage.empty() ? nullptr : _deviceApiUrlStorage.c_str();
            cfg.basicAuth = _basicAuthStorage.empty() ? nullptr : _basicAuthStorage.c_str();
            cfg.source = ProvisioningSource::WebPortal;

            _configCb(cfg);
        }

        sendStatus(ProvisioningStatus::Applied, "[PortalConfig] Configuracao recebida via portal");
    }

    void WebPortalProvisioningChannel::handleNotFound()
    {
#if defined(WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE) && (WEB_PORTAL_PROVISIONING_CAPTIVE_ENABLE != 0)
        handleRoot();
#else
        _server.send(404, "text/plain", "Not found");
#endif
    }

} // namespace iotsmartsys::core::provisioning

#endif
