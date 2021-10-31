#include <Arduino.h>
#include <Wire.h>  
#include <SPIFFS.h>
#include "rtc.h"
#include "nvdata.h"
#include "sensor.h"
#include "wificonfig.h"
#include "tone.h"
#include "google_sheets.h"

#define LOG_TO_GOOGLE_SHEET true

#define pinBuzzer       13  // piezo buzzer
#define pinControl      26   // controls the pump
#define pinConfigBtn    0   // wifi configuration
#define pinSDA          27
#define pinSCL          14

const char* FirmwareRevision = "1.00";

void setup() {
  pinMode(pinControl, OUTPUT);
  digitalWrite(pinControl, LOW);
  pinMode(pinConfigBtn, INPUT);
  pinMode(pinBuzzer, OUTPUT);
  Wire.begin(pinSDA,pinSCL); 

  Serial.begin(115200);
  Serial.println();
  Serial.printf("EspWaterTimer v%s compiled on %s at %s\n\n", FirmwareRevision, __DATE__, __TIME__);

  // load non-volatile data stored in preferences
  wifi_credentials_load(WiFiCredentials);
  schedule_load(Schedule);
  rtc_init();
  adc_init();

  rtc_get_clock(Clock);
  Serial.printf("\nRTC Clock : %s 20%02d-%02d-%02d %02d:%02d:%02d\n", szDayOfWeek[Clock.dayOfWeek-1],Clock.year, Clock.month, Clock.dayOfMonth, Clock.hour, Clock.minute, Clock.second);
  
  uint8_t alarmHour, alarmMinute, alarmMode;
  rtc_get_daily_alarm(alarmHour, alarmMinute, alarmMode);
  Serial.printf("RTC Alarm2 : %s @ %02d:%02d\n",  alarmMode == 0x04 ? "Daily" : "Error", alarmHour, alarmMinute);
  if ((alarmHour != Schedule.hour) || (alarmMinute != Schedule.minute) || (alarmMode != 0x04)) {
    Serial.println("RTC Alarm setting error, resetting");
    rtc_set_daily_alarm(Schedule.hour, Schedule.minute);
    }
  
  Serial.println("\nFor WiFi AP mode, press and hold button (GPIO0) until you hear a long tone");
  beep(pinBuzzer,1000, 100, 100,10); // beep 10 times, so you have enough time to press the wifi config button
    
  if (digitalRead(pinConfigBtn) == 0) {
    Serial.println("\n====== Access Point Configuration Mode ======");
    Serial.println("SSID=EspTimer Password=123456789  URL=http://192.168.4.1");
    // 3 second long 800Hz tone to indicate unit is now in web server configuration mode.
    tone_generate(pinBuzzer, 800, 3000);
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
      }
    wifi_access_point_init(); 
    }   
  else {
    Serial.println("\n====== Watering Mode ======");
    GS_DATA GSData;
    analogRead(pinADCSensor); // dummy sensor read, throwaway sample
    delay(50);
    // data to send to Google Docs sheet
    GSData.sensorReading = sensor_reading();
    GSData.batteryVoltage = battery_voltage();
    GSData.superCapVoltage = supercap_voltage();
    GSData.sensorThreshold = Schedule.sensorThreshold;
    Serial.printf("Battery Voltage %.1fV\n", GSData.batteryVoltage);
    Serial.printf("SuperCap Voltage %.1fV\n", GSData.superCapVoltage);
    Serial.printf("Sensor Threshold %d\n", GSData.sensorThreshold);
    Serial.printf("Sensor Reading %d\n", GSData.sensorReading); 
    
    if (GSData.sensorReading > (int)Schedule.sensorThreshold) {
      // soil is dry
      GSData.onTimeSeconds = Schedule.onTimeSeconds;
      Serial.printf("On Time %d secs\n", GSData.onTimeSeconds);
	    digitalWrite(pinControl, HIGH);
    	beep(pinBuzzer,1000, 500, 500, Schedule.onTimeSeconds); // beep .5secs on, 0.5secs off for d seconds while control pin is high
    	digitalWrite(pinControl, LOW);
      Serial.println("Watering done");
    	}
    else {
      // soil is still damp
      GSData.onTimeSeconds = 0;
      Serial.printf("On Time %d secs\n", GSData.onTimeSeconds);
      Serial.println("Watering not required");
    	}
    schedule_store(Schedule);
    Serial.println("Setting alarm for next day");
    rtc_set_daily_alarm(Schedule.hour, Schedule.minute);
#if (LOG_TO_GOOGLE_SHEET == true)    
    Serial.println("Updating google sheet - AutoWater");
    if (gs_init() == true) {
      gs_update(GSData);
      }
#endif    
    delay(100);    
    // for testing google sheets update, use esp32 wakeup timer for wakeup after 60seconds
    //Serial.println("Set next wakeup time for +1 minute");
    //esp_sleep_enable_timer_wakeup(60 * 1000000ULL);

    // reduce power consumption to minimum before going to sleep. The DS3231 RTC will generate
    // a reset pulse the next day at the scheduled time to wake up the ESP32
    wifi_off();
    esp_deep_sleep_start();
    }
  }


void loop() {
  // nothing to do here, if in WiFi AP configuration mode, the Async Server  runs in its own task thread
  }


  
