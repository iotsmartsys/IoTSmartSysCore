#pragma once

namespace iotsmartsys::core
{

    class Property
    {
    public:
        const char *property_name;
        const char *value;
        Property(const char *name, const char *val) : property_name(name), value(val) {}
    };
}