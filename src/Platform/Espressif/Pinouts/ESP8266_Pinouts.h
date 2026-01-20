#pragma once

#ifdef ESP8266

// Safe GPIO pins for ESP8266 module
#define ESP8266_SAFE_GPIO4 4   // D2
#define ESP8266_SAFE_GPIO5 5   // D1
#define ESP8266_SAFE_GPIO12 12 // D6
#define ESP8266_SAFE_GPIO13 13 // D7
#define ESP8266_SAFE_GPIO14 14 // D5

// I2C default pins
#define ESP8266_SDA ESP8266_SAFE_GPIO4 // D2
#define ESP8266_SCL ESP8266_SAFE_GPIO5 // D1

/*ADC0*/
#define ESP8266_ADC0 A0
#endif // ESP8266
