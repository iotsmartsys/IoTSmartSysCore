#pragma once
#include "Arduino.h"
#include "Models/CapabilityTiny.h"
#include <vector>

class RemoteCapabilitiesFetcher
{
public:
    RemoteCapabilitiesFetcher(const String &endpoint);

    bool fetch(std::vector<CapabilityTiny> &capabilities);

private:
    String endpoint;
};