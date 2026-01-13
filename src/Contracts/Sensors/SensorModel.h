#pragma once

namespace iotsmartsys::core
{
    enum class TemperatureSensorModel : uint8_t
    {
        #ifdef DS18B20_SENSOR_ENABLED
        DS18B20 = 0,
        #endif
        #ifdef DHT_SENSOR_ENABLED
        DHT = 1,
        #endif
        #ifdef BMP180_SENSOR_ENABLED
        BMP180 = 2
        #endif
    };

    enum class HumiditySensorModel : uint8_t
    {
        DHT = 0,
        HTU21D = 1
    };
} // namespace iotsmartsys::core