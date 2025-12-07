#pragma once
#include <Arduino.h>

#define VERSION "1.25.11.272348"  // Major.Minor.Patch.Build (HHMM)
#define IOT_PRIVATE_HOME_VERSION VERSION


// Helper: convert __DATE__ and __TIME__ to aa.MM.ddHHmm
String getBuildIdentifier();