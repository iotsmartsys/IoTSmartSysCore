#include "Arduino.h"

class Timezone
{
private:
    String timezone;
    int day_of_week;
    int day_of_year;
    // 2025-11-18T11:54:34.1244713-03:00
    String datetime;
    // 2025-11-18T14:54:34.1248925+00:00
    String utc_datetime;
    int week_number;

public:
    Timezone();
    ~Timezone();
};

/*
{
    "timezone": "America/Bahia",
    "day_of_week": 2,
    "day_of_year": 322,
    "week_number": 47
}
     */