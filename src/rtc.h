#ifndef RTC_H_
#define RTC_H_

#include <time.h>

typedef struct RTC_ALARM_ {
    uint8_t hour;
    uint8_t minute;
}  RTC_ALARM;

extern const char SzDayOfWeek[7][4];
extern struct tm Clock;
extern struct tm ClockSet;

void rtc_init();
void rtc_set_clock(struct tm &clock);
bool rtc_get_clock(struct tm &clock);
void rtc_set_daily_alarm(RTC_ALARM &alarm);
bool rtc_get_daily_alarm(RTC_ALARM &alarm);

#endif
