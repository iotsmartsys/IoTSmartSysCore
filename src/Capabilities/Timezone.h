#pragma once

#include <Arduino.h>
#include "Utils/Logger.h"

class Timezone
{
public:
    String timezone;
    int day_of_week;
    int day_of_year;
    String datetime;
    String utc_datetime;
    int week_number;

    void print() const
    {
        LOG_PRINTLN("Timezone: " + timezone);
        LOG_PRINTLN("Day of week: " + String(day_of_week));
        LOG_PRINTLN("Day of year: " + String(day_of_year));
        LOG_PRINTLN("Datetime: " + datetime);
        LOG_PRINTLN("UTC Datetime: " + utc_datetime);
        LOG_PRINTLN("Week number: " + String(week_number));
    }
};
