#include "VersionInfo.h"
#include <cstdio>
#include <cstring>

// Helper: convert __DATE__ and __TIME__ to aa.MM.ddHHmm
const char *getBuildIdentifier()
{
  // __DATE__ → "Nov 07 2025"
  // __TIME__ → "23:41:10"

  const char *date = __DATE__;
  const char *time = __TIME__;

  char monthStr[4];
  int day, year;
  sscanf(date, "%3s %d %d", monthStr, &day, &year);

  int hour, minute, second;
  sscanf(time, "%d:%d:%d", &hour, &minute, &second);

  int month = 0;
  if (strcmp(monthStr, "Jan") == 0)
    month = 1;
  else if (strcmp(monthStr, "Feb") == 0)
    month = 2;
  else if (strcmp(monthStr, "Mar") == 0)
    month = 3;
  else if (strcmp(monthStr, "Apr") == 0)
    month = 4;
  else if (strcmp(monthStr, "May") == 0)
    month = 5;
  else if (strcmp(monthStr, "Jun") == 0)
    month = 6;
  else if (strcmp(monthStr, "Jul") == 0)
    month = 7;
  else if (strcmp(monthStr, "Aug") == 0)
    month = 8;
  else if (strcmp(monthStr, "Sep") == 0)
    month = 9;
  else if (strcmp(monthStr, "Oct") == 0)
    month = 10;
  else if (strcmp(monthStr, "Nov") == 0)
    month = 11;
  else if (strcmp(monthStr, "Dec") == 0)
    month = 12;

  static char staticBuffer[20];
  std::snprintf(staticBuffer, sizeof(staticBuffer), "%02d.%02d.%02d%02d%02d",
                year % 100, month, day, hour, minute);
  return staticBuffer;
}
