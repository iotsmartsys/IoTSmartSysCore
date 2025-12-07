
#include "OTA.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include "Utils/Logger.h"

void setupOTA()
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
        LOG_PRINTLN("Iniciando atualização OTA: " + type); });

    ArduinoOTA.onEnd([]()
                     { LOG_PRINTLN("\nAtualização OTA concluída."); });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { LOG_PRINTF("Progresso: %u%%\r", (progress * 100) / total); });

    ArduinoOTA.onError([](ota_error_t error)
                       {
        LOG_PRINTF("Erro[%u]: ", error);
        if (error == OTA_AUTH_ERROR) LOG_PRINTLN("Falha de autenticação");
        else if (error == OTA_BEGIN_ERROR) LOG_PRINTLN("Falha ao iniciar");
        else if (error == OTA_CONNECT_ERROR) LOG_PRINTLN("Falha na conexão");
        else if (error == OTA_RECEIVE_ERROR) LOG_PRINTLN("Falha na recepção");
        else if (error == OTA_END_ERROR) LOG_PRINTLN("Falha na finalização"); });

    ArduinoOTA.begin();
    LOG_PRINTLN("OTA pronto.");
}

void handleOTA()
{
    ArduinoOTA.handle();
}
