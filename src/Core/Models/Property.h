#pragma once
#include <Arduino.h>

class Property {
public:
    String property_name;
    String value;
    Property(const String &name, const String &val) : property_name(name), value(val) {}
};