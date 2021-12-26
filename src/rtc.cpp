#include <Arduino.h>
#include <Wire.h>
#include "rtc.h"
#include "sensor.h"

struct tm Clock;
struct tm ClockSet; // used for changing RTC clock time

const char SzDayOfWeek[7][4] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

static const uint8_t      RTC_I2C_ADDRESS  = 0x68; 

static uint8_t rtc_write_byte(const uint8_t address, const uint8_t data);
static uint8_t rtc_read_byte(const uint8_t address, uint8_t &data);

static uint8_t rtc_write_byte(const uint8_t address, const uint8_t data) {
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(address);
	Wire.write(data);
	return Wire.endTransmission();
	}
  
static uint8_t rtc_read_byte(const uint8_t address, uint8_t &data){
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(address);
	Wire.endTransmission();  
	Wire.requestFrom(RTC_I2C_ADDRESS,(uint8_t) 1);
	data = Wire.read();
	return 1;
	}


// Setup the clock to make sure that it is running, that the oscillator and 
// square wave are disabled, and that alarm interrupts are disabled
void rtc_init() {
	rtc_write_byte(0x0E, 0x04);
	rtc_write_byte(0x0F, 0x00);// clear flags
	}

  
bool rtc_get_clock(struct tm &clock){
	uint8_t  b; 
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0x00);
	Wire.endTransmission();
	if(Wire.requestFrom(RTC_I2C_ADDRESS,(uint8_t) 7) == 7){
		b = Wire.read();
		clock.tm_sec = (b&0x0F) + 10*((b>>4)&0x07);
		b = Wire.read();
		clock.tm_min = (b&0x0F) + 10*((b>>4)&0x07);
		b = Wire.read();
		clock.tm_hour = (b&0x0F) + 10*((b>>4)&0x03);
		if (b & 0x40) {
			Serial.printf("Error : RTC is in 12 hour mode!\n");
			}
		b = Wire.read();
		clock.tm_wday = (b&0x07) - 1; // RTC uses 1-7
		b = Wire.read();
		clock.tm_mday = (b&0x0F) + 10*((b>>4)&0x03);
		b = Wire.read();
		clock.tm_mon = ((b&0x0F) + 10*((b>>4)&0x01)) - 1; // RTC uses 1-12
		b = Wire.read();
		clock.tm_year = ((b&0x0F) + 10*((b>>4)&0x0F)); 
		clock.tm_year += 100; // RTC uses 00-99, tm uses years since 1900
		//Serial.printf("RTC Clock : %04d-%02d-%02d %s %02d:%02d:%02d\n",  1900+clock.tm_year, 1+clock.tm_mon, clock.tm_mday, SzDayOfWeek[clock.tm_wday], clock.tm_hour, clock.tm_min, clock.tm_sec);
		return true;
		}
	else {
		Serial.println("unable to read RTC clock 7bytes");   
		return false;
		}
	}


void rtc_set_clock(struct tm &clock){
	uint8_t b;
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0x00);
	CLAMP(clock.tm_sec, 0, 59);
	b = (uint8_t)((clock.tm_sec%10) | ((clock.tm_sec/10)<<4));
	Wire.write(b);
	CLAMP(clock.tm_min, 0, 59);
	b = (uint8_t)((clock.tm_min%10) | ((clock.tm_min/10)<<4));
	Wire.write(b);
	CLAMP(clock.tm_hour, 0, 23); 
	b = (uint8_t)((clock.tm_hour%10) | ((clock.tm_hour/10)<<4));
	Wire.write(b);
	CLAMP(clock.tm_wday, 0, 6); 
	b = (uint8_t)(clock.tm_wday+1); // RTC uses 1-7
	Wire.write(b);
	CLAMP(clock.tm_mday, 1, 31); 
	b = (uint8_t)((clock.tm_mday%10) + ((clock.tm_mday/10)<<4));
	Wire.write(b);
	CLAMP(clock.tm_mon, 0, 11); 
	b = (uint8_t)(1 + ((clock.tm_mon%10) + ((clock.tm_mon/10)<<4))); // RTC uses 1-12
	Wire.write(b);
	CLAMP(clock.tm_year, 121, 199); // 2021 - 2099 (years since 1900)
	uint8_t y = (uint8_t) (clock.tm_year - 100); // RTC uses 0-99
	b = (y%10) | ((y/10)<<4); 
	//Serial.printf("tm_year = %d writing RTC year = %d\n", b);
	Wire.write(b);
	Wire.endTransmission();
	}


void rtc_set_daily_alarm(RTC_ALARM &alarm) {
	uint8_t b;
	// set A2M2,A2M3 = 0, A2M4 = 1, DYDT_ = 0 (don't care)
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0x0B);
	b = (alarm.minute%10) | ((alarm.minute/10)<<4);
	Wire.write(b); // 0x0B : minute
	b = (alarm.hour%10) | ((alarm.hour/10)<<4);
	Wire.write(b); // 0x0C : hour
	Wire.write(0x80); // 0x0D  : DYDT_ = 0, A2M4 = 1
	Wire.write(0x06);  // 0x0E : INTCN = 1, A2IE = 1 
	Wire.write(0x00); // 0x0F  : clear status bits
	Wire.endTransmission();
	}


bool rtc_get_daily_alarm(RTC_ALARM &alarm) {
	uint8_t b;
	uint8_t mode = 0;
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0x0B);
	Wire.endTransmission();
	if(Wire.requestFrom(RTC_I2C_ADDRESS,(uint8_t) 3) == 3){
		b = Wire.read();
		alarm.minute = (b&0x0F) + 10*((b>>4)&0x07);
		if (b & 0x80) {
			mode |= 0x01; // A2M2
			}
		b = Wire.read();
		alarm.hour = (b&0x0F) + 10*((b>>4)&0x03);
		if (b & 0x80) {
			mode |= 0x02; // A2M3
			}
		b = Wire.read();
		if (b & 0x80) {
			mode |= 0x04; // A2M4
			}
		// check A2M2,A2M3 = 0, A2M4 = 1
		return mode == 0x04 ? true : false; 
		}
	else {
		Serial.println("unable to read RTC alarm2 3 bytes");   
		return false;
		}    
	}


#if 0
void rtc_set_alarm1_date_time(uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
	uint8_t b;
	// alarm1
	// set A1M1,A1M2,A1M3,A1M4,DYDT_ = 0
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0x0B);
	b = (minute%10) | ((minute/10)<<4);
	Wire.write(b); // 0x0B : minute
	b = (hour%10) | ((hour/10)<<4);
	Wire.write(b); // 0x0C : hour
	Wire.write(0x80); // 0x0D  : DYDT_ = 0, A2M4 = 1
	Wire.write(0x06);  // 0x0E : INTCN = 1, A2IE = 1 
	Wire.write(0x00); // 0x0F  : clear status bits
	Wire.endTransmission();
	}

void rtc_set_alarm_weekday_time(uint8_t weekday, uint8_t hour, uint8_t minute, uint8_t second) {
	uint8_t b;
	// set A2M2,A2M3,A2M4 = 0, DYDT_ = 1
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0x0B);
	b = (minute%10) | ((minute/10)<<4);
	Wire.write(b); // 0x0B : minute
	b = (hour%10) | ((hour/10)<<4);
	Wire.write(b); // 0x0C : hour
	Wire.write(0x40 | weekday); // 0x0D  : DYDT_ = 1, A2M4 = 0
	Wire.write(0x06);  // 0x0E : INTCN = 1, A2IE = 1 
	Wire.write(0x00); // 0x0F  : clear status bits
	Wire.endTransmission();
	}
#endif

  	
  
