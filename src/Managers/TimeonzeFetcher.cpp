#include "TimezoneFetcher.h"
#if defined(ESP32)
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#endif


#include "Settings/ConfigManager.h"

// Url do serviço de timezone (pode ser mantida aqui, não é sensível)
#ifndef API_IOTSMARTSYS_TIMEZONE_URL
#define API_IOTSMARTSYS_TIMEZONE_URL "/devices/api/v1/timezone/datetime?zone=America%2FBahia"
#endif

String fetchDatetimeNow()
{
    HTTPClient http;

    http.begin(API_IOTSMARTSYS_TIMEZONE_URL);

    const Settings &settings = ConfigManager::instance().get();
    if (!settings.api.key.isEmpty())
    {
        http.addHeader("x-api-key", settings.api.key);
    }
    if (!settings.api.basic_auth.isEmpty())
    {
        http.addHeader("Authorization", settings.api.basic_auth);
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK)
    {
        http.end();
        return "";
    }

    String payload = http.getString();
    http.end();

    if (payload.isEmpty())
    {
        return "";
    }

    return payload;
}