#include "Capability.h"

#if defined(GLP_METER_ENABLED)
#include "HX711.h"

#ifndef HX711_OFFSET
#define HX711_OFFSET -600142
#endif
#ifndef HX711_SCALE_FACTOR
#define HX711_SCALE_FACTOR 20380.0f
#endif

class GlpMeterCapability : public Capability
{
public:
    GlpMeterCapability(String capability_name, HX711 *scale, int dout_pin, int sck_pin, float tare_weight_kg, float weight_capacity_kg);
    GlpMeterCapability(HX711 *scale, int dout_pin, int sck_pin, float tare_weight_kg, float weight_capacity_kg);

    void setup() override;
    void handle() override;
    float getKg();
    float getPercent();

private:
    HX711 *scale;
    float actualPercent = 0.0;
    float lastPercent = 0.0;
    float actualKg = 0.0;
    float lastKg = 0.0;
    int dout_pin;
    int sck_pin;
    float tare_weight_kg;
    float weight_capacity_kg;
};

#endif
