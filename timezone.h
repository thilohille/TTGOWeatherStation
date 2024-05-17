#include <stdint.h>
#include <time.h>


typedef struct {
    const char* country_code; // ISO alpha-2 country code
    int dst_offset;           // DST offset in hours
    int dst_start_month;      // Month when DST starts (1 = January, 12 = December)
    int dst_start_week;       // Week of the month when DST starts (1 = first week, 2 = second week, etc.)
    int dst_start_day;        // Day of the week when DST starts (0 = Sunday, 1 = Monday, ..., 6 = Saturday)
    int dst_end_month;        // Month when DST ends (1 = January, 12 = December)
    int dst_end_week;         // Week of the month when DST ends (1 = first week, 2 = second week, etc.)
    int dst_end_day;          // Day of the week when DST ends (0 = Sunday, 1 = Monday, ..., 6 = Saturday)
} CountryDSTInfo;

#define MAX_TIMEZONES 64

const CountryDSTInfo countries_with_dst[] = {
    {"AU", 10, 10, 1, 0, 4, 1, 0},    // Australia (DST starts 1st Sunday in October, ends 1st Sunday in April)
    {"BR", 1, 10, 2, 0, 2, 2, 0},     // Brazil (DST starts 3rd Sunday in October, ends 3rd Sunday in February)
    {"CA", 1, 3, 2, 0, 11, 1, 0},     // Canada (DST starts 2nd Sunday in March, ends 1st Sunday in November)
    {"DE", 3, 3, 5, 0, 10, 5, 0},     // Germany (DST starts last Sunday in March, ends last Sunday in October)
    {"EG", 2, 4, 5, 5, 9, 5, 4},      // Egypt (DST starts last Friday in April, ends last Thursday in September)
    {"FR", 1, 3, 5, 0, 10, 5, 0},
    {"GB", 1, 3, 5, 0, 10, 5, 0},     // United Kingdom (DST starts last Sunday in March, ends last Sunday in October)
    {"IE", 1, 3, 5, 0, 10, 5, 0},     // Ireland (DST starts last Sunday in March, ends last Sunday in October)
    {"IT", 1, 3, 5, 0, 10, 5, 0},
    {"MX", 1, 4, 1, 0, 10, 5, 0},     // Mexico (DST starts 1st Sunday in April, ends last Sunday in October)
    {"RU", 3, 3, 4, 0, 10, 5, 0},     // Russia (DST starts last Sunday in March, ends last Sunday in October)
    {"SG", 8, 4, 1, 0, 10, 5, 0},     // Singapore (DST is not observed)
    {"US", 1, 3, 2, 0, 11, 1, 0},     // USA (DST starts 2nd Sunday in March, ends 1st Sunday in November)
};


typedef struct {
    char name[50];
    int offset_hours;
} Timezone;

Timezone timezones[MAX_TIMEZONES] = {
    {"America/Anchorage", -9 * 60},
    {"America/Los_Angeles", -8 * 60},
    {"America/Denver", -7 * 60},
    {"America/Chicago", -6 * 60},
    {"America/New_York", -5 * 60},
    {"America/Sao_Paulo", -3 * 60},
    {"GMT", 0},
    {"Europe/London", 0},
    {"Europe/Dublin", 0},
    {"Africa/Cairo", 2 * 60},
    {"Asia/Bangkok", 7 * 60},
    {"Asia/Singapore", 8 * 60},
    {"Asia/Tokyo", 9 * 60},
    {"Australia/Sydney", 10 * 60},
    {"Antarctica/McMurdo", 12 * 60},
    {"Pacific/Auckland", 12 * 60},
    {"Pacific/Honolulu", -10 * 60},
    {"Asia/Kolkata", 5 * 60},
    {"Asia/Dubai", 4 * 60},
    {"Asia/Tehran", 3 * 60},
    {"Europe/Paris", 1 * 60},
    {"Europe/London", 0},
    {"Europe/Dublin", 0},
    {"Europe/Berlin", 60},        // Central European Time (CET)
    {"Europe/Rome", 60},          // Central European Time (CET)
    {"Europe/Madrid", 60},        // Central European Time (CET)
    {"Europe/Amsterdam", 60},     // Central European Time (CET)
    {"Europe/Brussels", 60},      // Central European Time (CET)
    {"Europe/Vienna", 60},        // Central European Time (CET)
    {"Europe/Zurich", 60},        // Central European Time (CET)
    {"Europe/Stockholm", 60},     // Central European Time (CET)
    {"Europe/Oslo", 60},          // Central European Time (CET)
    {"Europe/Copenhagen", 60},    // Central European Time (CET)
    {"Europe/Budapest", 60},      // Central European Time (CET)
    {"Europe/Prague", 60},        // Central European Time (CET)
    {"Europe/Warsaw", 60},        // Central European Time (CET)
    {"Europe/Sofia", 120},        // Eastern European Time (EET)
    {"Europe/Bucharest", 120},    // Eastern European Time (EET)
    {"Europe/Helsinki", 120},     // Eastern European Time (EET)
    {"Europe/Tallinn", 120},      // Eastern European Time (EET)
    {"Europe/Riga", 120},         // Eastern European Time (EET)
    {"Europe/Vilnius", 120},      // Eastern European Time (EET)
    {"Europe/Athens", 120},       // Eastern European Time (EET)
    {"Europe/Sofia", 120},        // Eastern European Time (EET)
    {"Europe/Nicosia", 120},      // Eastern European Time (EET)
    {"Europe/Istanbul", 180},     // Turkey Time (TRT)
    {"Europe/Kiev", 120},         // Eastern European Time (EET)
    {"Europe/Minsk", 180},        // Moscow Standard Time (MSK)
    {"Europe/Moscow", 180},       // Moscow Standard Time (MSK)
    {"Europe/Samara", 240},       // Samara Time (SAMT)
    {"Europe/Volgograd", 240},    // Volgograd Time (VOLT)
    {"Europe/Kaliningrad", 120},   // Eastern European Time (EET)    
    {"Africa/Johannesburg", 2 * 60},
    {"Asia/Hong_Kong", 8 * 60},
    {"Asia/Shanghai", 8 * 60},
    {"Asia/Seoul", 9 * 60},
    {"Asia/Jakarta", 7 * 60},
    {"Australia/Perth", 8 * 60},
    {"Australia/Brisbane", 10 * 60},
    {"Australia/Adelaide", 9 * 60},
    {"Pacific/Fiji", 12 * 60},
    {"Pacific/Apia", 13 * 60},
    {"Pacific/Tongatapu", 13 * 60}
};


int getOffsetByName(const char* name) {
    Serial.println(name);
    for (int i = 0; i < MAX_TIMEZONES; ++i) {
        if (strcmp(name, timezones[i].name) == 0) {
            return 60 * timezones[i].offset_hours;
        }
    }
    return -1; // Return -1 if timezone name not found
}

// Define the function to get the DST offset in seconds
int getDSTOffsetInSeconds(const char* country_code, time_t current_time) {
    // Get the current time
    struct tm* timeinfo = localtime(&current_time);
    int month = timeinfo->tm_mon + 1; // Adjust month to 1-based index
    int day = timeinfo->tm_mday;
    int week = (day - 1) / 7 + 1; // Calculate week of the month

    // Loop through the countries with DST info
    for (size_t i = 0; i < sizeof(countries_with_dst) / sizeof(countries_with_dst[0]); ++i) {
        // Check if the country code matches
        if (strcmp(country_code, countries_with_dst[i].country_code) == 0) {
            // Check if current time is within DST period
            if ((month > countries_with_dst[i].dst_start_month || (month == countries_with_dst[i].dst_start_month && week >= countries_with_dst[i].dst_start_week))
                && (month < countries_with_dst[i].dst_end_month || (month == countries_with_dst[i].dst_end_month && week <= countries_with_dst[i].dst_end_week))) {
                // DST is in effect, return the offset in seconds
                return 3600; // Example: DST offset is 1 hour (3600 seconds)
            }
            break; // No need to continue searching
        }
    }
    // No DST in effect, return 0 offset
    return 0;
}
