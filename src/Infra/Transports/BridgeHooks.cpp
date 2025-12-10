#include "Infra/Transports/BridgeHooks.h"


__attribute__((weak)) void mqtt_to_espnow_forward(const CapabilityCommand &cmd) {
    (void)cmd;
}

