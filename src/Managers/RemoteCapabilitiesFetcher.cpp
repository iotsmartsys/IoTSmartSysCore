#include "RemoteCapabilitiesFetcher.h"
#if defined(ESP32)
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#endif

RemoteCapabilitiesFetcher::RemoteCapabilitiesFetcher(const String &endpoint)
    : endpoint(endpoint) {}

bool RemoteCapabilitiesFetcher::fetch(std::vector<CapabilityTiny> &capabilities)
{
    HTTPClient http;
    

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK)
    {
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    if (payload.isEmpty())
    {
        return false;
    }
        
    capabilities.clear();
    int start = 0;
    while (start < payload.length()) {
        int end = payload.indexOf('\n', start);
        if (end == -1) end = payload.length();
        String line = payload.substring(start, end);
        int sepIndex1 = line.indexOf(';');
        int sepIndex2 = line.indexOf(';', sepIndex1 + 1);

        if (sepIndex1 > 0 && sepIndex2 > sepIndex1) {
            String name = line.substring(0, sepIndex1);
            String value = line.substring(sepIndex1 + 1, sepIndex2);
            String updatedAt = line.substring(sepIndex2 + 1);

            capabilities.push_back(CapabilityTiny{
                .capability_name = name,
                .value = value,
                .updated_at = updatedAt});
        }

        start = end + 1;
    }

    return true;
}