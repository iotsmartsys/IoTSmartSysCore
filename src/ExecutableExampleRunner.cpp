#if defined(APP_EXAMPLE_RUNNER)

#if (defined(IOTSMARTSYS_EXAMPLE_BASIC_LIGHT) + defined(IOTSMARTSYS_EXAMPLE_ENVIRONMENT_DHT)) != 1
#error "Example environment must select exactly one IOTSMARTSYS_EXAMPLE_* application"
#endif

#if defined(IOTSMARTSYS_EXAMPLE_BASIC_LIGHT)
#include "../examples/executable/basic_light/example.hpp"
#elif defined(IOTSMARTSYS_EXAMPLE_ENVIRONMENT_DHT)
#include "../examples/executable/environment_dht/example.hpp"
#endif

#endif
