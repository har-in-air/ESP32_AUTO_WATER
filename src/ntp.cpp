#include <WiFi.h>
#include <time.h>
#include "nv_data.h"
#include "rtc.h"

static const char* NTPServer = "pool.ntp.org";

// utc offset and daylight offset are used for converting NTP UTC 
// time to local time. This is then used to correct the DS3231 RTC
// clock time

bool ntp_get_local_time(struct tm &localTime) {
	//Serial.printf("UTC Offset seconds = %d, daylight Offset seconds = %d\n", GSConfig.utcOffsetSeconds, GSConfig.daylightOffsetSeconds);    
	configTime(GSConfig.utcOffsetSeconds, GSConfig.daylightOffsetSeconds, NTPServer);
	if(!getLocalTime(&localTime)){
		Serial.println("Failed to obtain NTP time");
		return false;
		}
	//Serial.printf("NTP local time =  %4d-%02d-%02d %s %02d:%02d:%02d\n",
	//1900+localTime.tm_year, 1+localTime.tm_mon, localTime.tm_mday, SzDayOfWeek[localTime.tm_wday], 
	//localTime.tm_hour, localTime.tm_min, localTime.tm_sec); 
	return true;
	}


int32_t ntp_rtc_diff(struct tm &ntp, struct tm &rtc) {
	int32_t ntpSeconds = ntp.tm_hour*3600 + ntp.tm_min*60 + ntp.tm_sec;
	int32_t rtcSeconds = rtc.tm_hour*3600 + rtc.tm_min*60 + rtc.tm_sec;
	return (ntpSeconds - rtcSeconds);
	}
