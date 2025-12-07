#if defined(ST7789_170x320_ENABLED)
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>


#define TFT_DC 15
#define TFT_RST 16
#define TFT_SCLK 9
#define TFT_MOSI 8
#define TFT_CS 14 


extern Adafruit_ST7789 tft;

void setup_ST7789_170x320();
void displayMessage(const String &message);
void displayError(const String &message);
void displayInfo(const String &message);
void displayWarning(const String &message);
#endif 