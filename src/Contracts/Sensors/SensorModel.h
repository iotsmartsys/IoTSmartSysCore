#pragma once
#include <cstdint>

namespace iotsmartsys::core
{
    enum class TemperatureSensorModel : std::uint8_t
    {
        DS18B20 = 0,
        DHT = 1,
        BMP180 = 2
    };

    enum class HumiditySensorModel : std::uint8_t
    {
        DHT = 0,
        HTU21D = 1
    };
} // namespace iotsmartsys::core