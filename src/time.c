#define _CRT_SECURE_NO_WARNINGS
#include "nmea/time.h" // Include the header for nmeaTIME structure
#include <time.h>      // time_t and struct tm for handling time
#include <stddef.h>    // NULL definition
#include <stdbool.h>   // Boolean definitions

#ifdef NMEA_WIN
#   include <windows.h>
#endif

#ifdef NMEA_WIN
void nmea_time_now(nmeaTIME *t) {
    SYSTEMTIME st;
    GetSystemTime(&st);

    t->year = st.wYear - 1900;  // Convert to years since 1900
    t->mon = st.wMonth - 1;     // Convert to 0-based month index
    t->day = st.wDay;
    t->hour = st.wHour;
    t->min = st.wMinute;
    t->sec = st.wSecond;
    t->hsec = st.wMilliseconds / 10;  // Convert milliseconds to hundredths
}
#else
#include <stdint.h>
#include "driverlib/hibernate.h"

typedef long time_t;

struct tm {
    int tm_sec;   // Seconds [0,60].
    int tm_min;   // Minutes [0,59].
    int tm_hour;  // Hour [0,23].
    int tm_mday;  // Day of month [1,31].
    int tm_mon;   // Month of year [0,11].
    int tm_year;  // Years since 1900.
    int tm_wday;  // Day of week [0,6] (Sunday = 0).
    int tm_yday;  // Day of year [0,365].
    int tm_isdst; // Daylight Savings flag.
};


void nmea_time_now(nmeaTIME *t) {
    time_t current_time = time(NULL);
    struct tm *time_info = gmtime(&current_time);

    if (time_info) {
        t->year = time_info->tm_year + 1900;
        t->mon = time_info->tm_mon;
        t->day = time_info->tm_mday;
        t->hour = time_info->tm_hour;
        t->min = time_info->tm_min;
        t->sec = time_info->tm_sec;
        t->hsec = 0;
    } else {
        t->year = 0;
        t->mon = 0;
        t->day = 1;
        t->hour = 0;
        t->min = 0;
        t->sec = 0;
        t->hsec = 0;
    }
}

#endif
