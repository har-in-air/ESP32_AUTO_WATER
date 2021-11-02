#ifndef RTC_H_
#define RTC_H_

typedef struct RTC_ {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfMonth;
    uint8_t dayOfWeek;
    uint8_t month;
    uint8_t year;
    } RTC;

typedef struct RTC_ALARM_ {
    uint8_t hour;
    uint8_t minute;
}  RTC_ALARM;

extern const char szDayOfWeek[7][4];
extern RTC Clock;
extern RTC ClockSet;

void rtc_init();
void rtc_set_clock(RTC &clock);
bool rtc_get_clock(RTC &clock);
void rtc_set_daily_alarm(RTC_ALARM &alarm);
bool rtc_get_daily_alarm(RTC_ALARM &alarm);

#endif
