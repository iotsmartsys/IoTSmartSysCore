#include <Arduino.h>
#include "pins.h"
#include "Utils/Logger.h"









const int RED_PIN = -1; 
const int GREEN_PIN = ESP32S3_BOARD_AI_LED_GREEN;
const int BLUE_PIN = ESP32S3_BOARD_AI_LED_BLUE;


const int FREQ = 5000;
const int RESOLUTION = 8; 
const int CH_R = 0;
const int CH_G = 1;
const int CH_B = 2;


#if defined(BOARD_LED_STATE_ON) && defined(LOW)
#if BOARD_LED_STATE_ON == LOW
const bool LED_ACTIVE_HIGH = false;
#else
const bool LED_ACTIVE_HIGH = true;
#endif
#else
const bool LED_ACTIVE_HIGH = true;
#endif


void hsvToRgb(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b){
  float c = v * s;
  float hh = fmod(h / 60.0, 6.0);
  float x = c * (1 - fabs(fmod(hh, 2) - 1));
  float m = v - c;
  float rf=0, gf=0, bf=0;
  if(hh >= 0 && hh < 1){ rf = c; gf = x; bf = 0; }
  else if(hh >= 1 && hh < 2){ rf = x; gf = c; bf = 0; }
  else if(hh >= 2 && hh < 3){ rf = 0; gf = c; bf = x; }
  else if(hh >= 3 && hh < 4){ rf = 0; gf = x; bf = c; }
  else if(hh >= 4 && hh < 5){ rf = x; gf = 0; bf = c; }
  else { rf = c; gf = 0; bf = x; }

  r = (uint8_t)constrain((rf + m) * 255.0, 0, 255);
  g = (uint8_t)constrain((gf + m) * 255.0, 0, 255);
  b = (uint8_t)constrain((bf + m) * 255.0, 0, 255);
}

void setup(){
  
  if(RED_PIN >= 0){
    ledcSetup(CH_R, FREQ, RESOLUTION);
    ledcAttachPin(RED_PIN, CH_R);
  }
  ledcSetup(CH_G, FREQ, RESOLUTION);
  ledcAttachPin(GREEN_PIN, CH_G);
  ledcSetup(CH_B, FREQ, RESOLUTION);
  ledcAttachPin(BLUE_PIN, CH_B);

  
  LOG_BEGIN(115200);
  LOG_PRINTLN("RGB Party iniciado");
}

void setLedColor(uint8_t r, uint8_t g, uint8_t b){
  
  uint8_t outR = LED_ACTIVE_HIGH ? r : 255 - r;
  uint8_t outG = LED_ACTIVE_HIGH ? g : 255 - g;
  uint8_t outB = LED_ACTIVE_HIGH ? b : 255 - b;

  if(RED_PIN >= 0) ledcWrite(CH_R, outR);
  ledcWrite(CH_G, outG);
  ledcWrite(CH_B, outB);
}

void loop(){
  static float hue = 0.0;
  const float DH = 0.5; 
  const float S = 1.0;  
  const float V = 0.6;  

  uint8_t r,g,b;
  hsvToRgb(hue, S, V, r, g, b);
  setLedColor(r,g,b);

  hue += DH;
  if(hue >= 360.0) hue -= 360.0;

  
  delay(20);
}
