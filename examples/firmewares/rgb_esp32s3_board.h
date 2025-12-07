#include "Arduino.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48  
#define NUMPIXELS 1 

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
    pixels.begin(); 
    pixels.clear(); 
    pixels.show();
}


const int COLOR_R = 102;
const int COLOR_G = 178;
const int COLOR_B = 178;


void loop()
{
    
    pixels.setPixelColor(0, pixels.Color(COLOR_R, COLOR_G, COLOR_B));
    pixels.show(); 
    delay(20000);

    
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    delay(1000);

    
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
    delay(1000);

    
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
    pixels.show();
    delay(1000);

    
    pixels.setPixelColor(0, pixels.Color(255, 255, 255));
    pixels.show();
    delay(1000);
}