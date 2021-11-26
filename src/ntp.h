#ifndef NTP_H_
#define NTP_H_

#include <time.h>

bool ntp_get_local_time(struct tm &localTime);
int32_t ntp_rtc_diff(struct tm &ntp, struct tm &rtc);

#endif