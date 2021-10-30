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

extern const char szDayOfWeek[7][4];
extern RTC Clock;
extern RTC ClockSet;
    
void rtc_init();
void rtc_set_clock(RTC &clock);
void rtc_get_clock(RTC &clock);
void rtc_set_daily_alarm(uint8_t hour, uint8_t minute);
void rtc_get_daily_alarm(uint8_t &hour, uint8_t &minute, uint8_t &mode);

#endif
