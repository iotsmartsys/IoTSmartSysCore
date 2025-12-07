#include "IoTJson.h"

String getJsonValue(const String &json, const String &key)
{
    String searchKey = "\"" + key + "\":";
    int startIndex = json.indexOf(searchKey);
    if (startIndex == -1)
    {
        return "";
    }

    
    startIndex = json.indexOf("\"", startIndex + searchKey.length());
    if (startIndex == -1)
    {
        return "";
    }
    startIndex++; 

    
    int endIndex = json.indexOf("\"", startIndex);
    if (endIndex == -1)
    {
        return "";
    }

    return json.substring(startIndex, endIndex);
}