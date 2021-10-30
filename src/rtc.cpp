#include <Arduino.h>
#include <Wire.h>
#include "rtc.h"
#include "sensor.h"

RTC Clock;
RTC ClockSet; // used for changing RTC clock time

const char szDayOfWeek[7][4] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

static const uint8_t      RTC_ADDRESS  = 0x68; 

static uint8_t rtc_write_byte(const uint8_t address, const uint8_t data);
static uint8_t rtc_read_byte(const uint8_t address, uint8_t &data);

static uint8_t rtc_write_byte(const uint8_t address, const uint8_t data) {
	Wire.beginTransmission(RTC_ADDRESS);
	Wire.write(address);
	Wire.write(data);
	return Wire.endTransmission();
	}
  
static uint8_t rtc_read_byte(const uint8_t address, uint8_t &data){
	Wire.beginTransmission(RTC_ADDRESS);
	Wire.write(address);
	Wire.endTransmission();  
	Wire.requestFrom(RTC_ADDRESS,(uint8_t) 1);
	data = Wire.read();
	return 1;
	}
    
void rtc_init() {
	// Setup the clock to make sure that it is running, that the oscillator and 
	// square wave are disabled, and that alarm interrupts are disabled
	rtc_write_byte(0x0E, 0x04);
  rtc_write_byte(0x0F, 0x00);// clear flags
	}

  
void rtc_get_clock(RTC &clock){
	uint8_t  b; 
	Wire.beginTransmission(RTC_ADDRESS);
	Wire.write(0x00);
	Wire.endTransmission();
	if(Wire.requestFrom(RTC_ADDRESS,(uint8_t) 7) == 7){
		b = Wire.read();
		clock.second = (b&0x0F) + 10*((b>>4)&0x07);
		b = Wire.read();
		clock.minute = (b&0x0F) + 10*((b>>4)&0x07);
		b = Wire.read();
		clock.hour = (b&0x0F) + 10*((b>>4)&0x03);
		if (b & 0x40) {
			Serial.printf("Error : RTC is in 12 hour mode!\n");
			}
    b = Wire.read();
    clock.dayOfWeek = (b&0x07);
    b = Wire.read();
    clock.dayOfMonth = (b&0x0F) + 10*((b>>4)&0x03);
    b = Wire.read();
    clock.month = (b&0x0F) + 10*((b>>4)&0x01);
    b = Wire.read();
    clock.year = (b&0x0F) + 10*((b>>4)&0x0F);
		}
else {
  Serial.println("unable to read RTC clock 7bytes");   
  }
	}

void rtc_set_clock(RTC &clock){
	uint8_t b;
	Wire.beginTransmission(RTC_ADDRESS);
	Wire.write(0x00);
	CLAMP(clock.second, 0,59);
  b = (clock.second%10) | ((clock.second/10)<<4);
	Wire.write(b);
	CLAMP(clock.minute, 0,59);
  b = (clock.minute%10) | ((clock.minute/10)<<4);
  Wire.write(b);
  CLAMP(clock.hour, 0,23); // 0 - 23
  b = (clock.hour%10) | ((clock.hour/10)<<4);
  Wire.write(b);
  b = clock.dayOfWeek; // 1- 7
  Wire.write(b);
  b = (clock.dayOfMonth%10) | ((clock.dayOfMonth/10)<<4); // 1-31
  Wire.write(b);
  b = (clock.month%10) | ((clock.month/10)<<4); // 1-12
  Wire.write(b);
  b = (clock.year%10) | ((clock.year/10)<<4); // 0 -99
  Wire.write(b);
  Wire.endTransmission();
	}

// set daily alarm
void rtc_set_daily_alarm(uint8_t hour, uint8_t minute) {
	uint8_t b;
  // set A2M2,A2M3 = 0, A2M4 = 1, DYDT_ = 0 (don't care)
  Wire.beginTransmission(RTC_ADDRESS);
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

void rtc_get_daily_alarm(uint8_t &hour, uint8_t &minute, uint8_t &mode) {
    uint8_t b;
    mode = 0;
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0x0B);
  Wire.endTransmission();
  if(Wire.requestFrom(RTC_ADDRESS,(uint8_t) 3) == 3){
    b = Wire.read();
    minute = (b&0x0F) + 10*((b>>4)&0x07);
      if (b & 0x80) {
        mode |= 0x01; // A2M2
        }
    b = Wire.read();
    hour = (b&0x0F) + 10*((b>>4)&0x03);
      if (b & 0x80) {
        mode |= 0x02; // A2M3
        }
    b = Wire.read();
      if (b & 0x80) {
        mode |= 0x04; // A2M4
        }
    }
else {
  Serial.println("unable to read RTC alarm2 3 bytes");   
  }
    
    }

/*
void rtc_set_alarm1_date_time(uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  uint8_t b;
  // alarm1
  // set A1M1,A1M2,A1M3,A1M4,DYDT_ = 0
  Wire.beginTransmission(RTC_ADDRESS);
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
  Wire.beginTransmission(RTC_ADDRESS);
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
*/

  	
  
