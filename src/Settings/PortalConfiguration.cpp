#include "Arduino.h"
#include "PortalConfiguration.h"
#include "Wifi/WifiHelper.h"

#include "Settings/ConfigManager.h"
#include <vector>
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

namespace
{
    HttpServer server(80);
    bool configSaved = false;
    DNSServer dnsServer;
    constexpr uint8_t DNS_PORT = 53;

    String buildConfigPage()
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
#if ARDUINO
        {
            // Busca as redes disponíveis usando o helper já existente
            std::vector<std::string> ssids = getAvailableSSIDs();
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
        }
#endif
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

    void handleRoot()
    {
        server.send(200, "text/html", buildConfigPage());
    }

    void handleSave()
    {
        const String ssid = server.arg("ssid");
        const String password = server.arg("password");
        const String deviceApiKey = server.arg("device_api_key");
        const String basicAuth = server.arg("basic_auth");

        ConfigManager::instance().setConfigValue("wifi_ssid", ssid);
        ConfigManager::instance().setConfigValue("wifi_password", password);
        ConfigManager::instance().setConfigValue("device_api_key", deviceApiKey);
        ConfigManager::instance().setConfigValue("basic_auth", basicAuth);

        server.send(200, "text/html",
                    F("<!DOCTYPE html><html><head><meta charset='utf-8'>"
                      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                      "<title>Configura&#231;&#227;o salva</title></head><body>"
                      "<h1>Configura&#231;&#227;o salva!</h1>"
                      "<p>Voc&#234; j&#225; pode fechar esta p&#225;gina. O dispositivo ir&#225; reiniciar ou tentar conectar &#224; rede configurada.</p>"
                      "</body></html>"));

        configSaved = true;
        ConfigManager::instance().finalizeConfigMode();
    }

    void handleNotFound()
    {
        // Para captive portal, qualquer rota leva para a página principal
        handleRoot();
    }
}

namespace BootstrapDevice
{
    void initialize(Settings const &settings)
    {
        Serial.begin(115200);
        delay(1000); // Aguarda inicialização do Serial
        if (settings.in_config_mode == false)
        {
            Serial.println(F("[PortalConfig] Dispositivo nao esta em modo de configuracao. Pulando portal."));
            return;
        }

        Serial.println(F("[PortalConfig] Iniciando portal de configuracao..."));

        (void)settings;

        WiFi.mode(WIFI_AP);

#if defined(ESP32)
        String apName = "ESP-Config-" + String((uint32_t)ESP.getEfuseMac(), HEX);
#else
        String apName = "ESP-Config-" + String(ESP.getChipId(), HEX);
#endif
        WiFi.softAP(apName.c_str());

        IPAddress ip = WiFi.softAPIP();
        Serial.println(F("[PortalConfig] Modo de configuracao iniciado."));
        Serial.print(F("[PortalConfig] SSID do AP: "));
        Serial.println(apName);
        Serial.print(F("[PortalConfig] Acesse em: http://"));
        Serial.println(ip);

        // Inicia DNS para captive portal: qualquer domínio aponta para o IP do ESP
        dnsServer.start(DNS_PORT, "*", ip);

        // Rotas HTTP
        server.on("/", HTTP_GET, handleRoot);
        // Endpoints que alguns sistemas usam para detectar portal cativo
        server.on("/generate_204", HTTP_GET, handleRoot);        // Android
        server.on("/hotspot-detect.html", HTTP_GET, handleRoot); // iOS antigo / macOS
        server.on("/ncsi.txt", HTTP_GET, handleRoot);            // Windows
        server.on("/connecttest.txt", HTTP_GET, handleRoot);     // Outros/variações

        server.on("/save", HTTP_POST, handleSave);
        server.onNotFound(handleNotFound);

        server.begin();
        Serial.println(F("[PortalConfig] Servidor HTTP iniciado na porta 80."));

        while (!configSaved)
        {
            dnsServer.processNextRequest();
            server.handleClient();
            delay(10);
        }

        server.stop();
        dnsServer.stop();
        WiFi.softAPdisconnect(true);
        Serial.println(F("[PortalConfig] Portal de configuracao finalizado."));
    }

}
