
#include "OTA.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "Contracts/Logging/Log.h"

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

namespace iotsmartsys::ota
{
    OTA::OTA(ILogger &logger)
        : _logger(logger)
    {
    }
    void OTA::setup()
    {

        const char *hostname = WiFi.getHostname();
        ArduinoOTA.setHostname(hostname);
        ArduinoOTA.onStart([]()
                           {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { 
            type = "filesystem";
        }
       auto &logger = Log::get();
       logger.info("OTA","Iniciando atualização OTA"); });

        ArduinoOTA.onEnd([]()
                         { 
                            auto &logger = Log::get();
                            logger.info("OTA", "Atualização OTA concluída."); });

        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                              {
                                  auto &logger = Log::get();
                                  logger.info("OTA", "Progresso: %u%%", (progress * 100) / total); });

        ArduinoOTA.onError([](ota_error_t error)
                           {
                               auto &logger = Log::get();
                               logger.error("OTA","Erro[%u]: ", error);
                               if (error == OTA_AUTH_ERROR) logger.error("OTA","Falha de autenticação");
        else if (error == OTA_BEGIN_ERROR) logger.error("OTA","Falha ao iniciar");
        else if (error == OTA_CONNECT_ERROR) logger.error("OTA","Falha na conexão");
        else if (error == OTA_RECEIVE_ERROR) logger.error("OTA","Falha na recepção");
        else if (error == OTA_END_ERROR) logger.error("OTA","Falha na finalização"); });

        ArduinoOTA.begin();
        _initialized = true;
        _logger.info("OTA", "Pronto.");
    }

    void OTA::handle()
    {
        ArduinoOTA.handle();
    }
} // namespace iotsmartsys::ota