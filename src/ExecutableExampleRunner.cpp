#if defined(APP_EXAMPLE_RUNNER)

#if (defined(IOTSMARTSYS_EXAMPLE_BASIC_LIGHT) + defined(IOTSMARTSYS_EXAMPLE_ENVIRONMENT_DHT) + \
     defined(IOTSMARTSYS_EXAMPLE_ENVIRONMENT_NTC) + \
     defined(IOTSMARTSYS_EXAMPLE_SCREEN_CONSOLE) + defined(IOTSMARTSYS_EXAMPLE_CURRENT_SENSOR) + \
     defined(IOTSMARTSYS_EXAMPLE_VOLTAGE_SENSOR) + defined(IOTSMARTSYS_EXAMPLE_POWER_ENERGY) + \
     defined(IOTSMARTSYS_EXAMPLE_INA3221_VOLTAGE_CURRENT) + defined(IOTSMARTSYS_EXAMPLE_FAN)) != 1
#error "Example environment must select exactly one IOTSMARTSYS_EXAMPLE_* application"
#endif

#if defined(IOTSMARTSYS_EXAMPLE_BASIC_LIGHT)
#include "../examples/executable/basic_light/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_ENVIRONMENT_DHT)
#include "../examples/executable/environment_dht/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_ENVIRONMENT_NTC)
#include "../examples/executable/environment_ntc/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_SCREEN_CONSOLE)
#include "../examples/executable/screen_console/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_CURRENT_SENSOR)
#include "../examples/executable/current_sensor/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_VOLTAGE_SENSOR)
#include "../examples/executable/voltage_sensor/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_POWER_ENERGY)
#include "../examples/executable/power_energy/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_INA3221_VOLTAGE_CURRENT)
#include "../examples/executable/ina3221_voltage_current/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_FAN)
#include "../examples/executable/fan/example.hpp"
#endif

#endif
