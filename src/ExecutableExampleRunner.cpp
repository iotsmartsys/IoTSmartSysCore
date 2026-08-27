#if defined(APP_EXAMPLE_RUNNER)

#if (defined(IOTSMARTSYS_EXAMPLE_BASIC_LIGHT) + defined(IOTSMARTSYS_EXAMPLE_ENVIRONMENT_DHT) + \
     defined(IOTSMARTSYS_EXAMPLE_SCREEN_CONSOLE) + defined(IOTSMARTSYS_EXAMPLE_CURRENT_SENSOR)) != 1
#error "Example environment must select exactly one IOTSMARTSYS_EXAMPLE_* application"
#endif

#if defined(IOTSMARTSYS_EXAMPLE_BASIC_LIGHT)
#include "../examples/executable/basic_light/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_ENVIRONMENT_DHT)
#include "../examples/executable/environment_dht/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_SCREEN_CONSOLE)
#include "../examples/executable/screen_console/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_CURRENT_SENSOR)
#include "../examples/executable/current_sensor/example.hpp"
#endif

#endif
