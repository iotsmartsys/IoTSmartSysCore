#pragma once

#include "Capability.h"
#if defined(RGB_ENABLED)
#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 1 

class RGBLightCapability : public Capability
{
public:
    RGBLightCapability(int lightPin);
    RGBLightCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

    void toggle();
    void turnOn();
    void turnOff();
    void setColor(int r, int g, int b);
    bool isOn();
    void executeCommand(String state);

private:
    int lightPin;
    bool lightState;
    const int COLOR_R;
    const int COLOR_G;
    const int COLOR_B;

    Adafruit_NeoPixel pixels;

    void power(String state);
};

#endif