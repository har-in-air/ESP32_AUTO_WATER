#include <Arduino.h>
#include <Wire.h>  
#include <SPIFFS.h>
#include "rtc.h"
#include "nvdata.h"
#include "sensor.h"
#include "wificonfig.h"
#include "tone.h"
#include "gs_update.h"

#define pinBuzzer       13  // piezo buzzer
#define pinPumpSwitch   26  // controls the pump
#define pinConfigBtn    0   // wifi configuration
#define pinSDA          27
#define pinSCL          14

const char* FirmwareRevision = "1.10";

void setup() {
  pinMode(pinPumpSwitch, OUTPUT);
  digitalWrite(pinPumpSwitch, LOW);
  pinMode(pinConfigBtn, INPUT);
  pinMode(pinBuzzer, OUTPUT);
  digitalWrite(pinBuzzer, LOW);
  // I2C interface to DS3231 RTC
  Wire.begin(pinSDA,pinSCL); 

  Serial.begin(115200);
  Serial.println();
  Serial.printf("EspWaterTimer v%s compiled on %s at %s\n\n", FirmwareRevision, __DATE__, __TIME__);

  // load non-volatile data stored in preferences
  // google sheet update yes/no, internet access point credentials
  gs_config_load(GSConfig); 
  // daily scheduled hour/minute, soil moisture threshold, pump on-time
  schedule_load(Schedule); 
  // buffer of unsent google sheet data records
  log_buffer_load(LogBuffer); 
  Serial.printf("Number unsent records = %d\n", LogBuffer.numEntries);

  Serial.printf("GS Update required = %s\n", GSConfig.update ? "true" : "false");

  Serial.printf("Loaded Schedule\n\ttime = %02d:%02d\n\tsensor threshold = %d\n\tton time = %d secs\n", 
    Schedule.hour, Schedule.minute, Schedule.sensorThreshold, Schedule.onTimeSeconds);
  rtc_init();
  adc_init();

  if (rtc_get_clock(Clock) == true) {
    Serial.printf("\nRTC Clock : %s 20%02d-%02d-%02d %02d:%02d:%02d\n", szDayOfWeek[Clock.dayOfWeek-1],Clock.year, Clock.month, Clock.dayOfMonth, Clock.hour, Clock.minute, Clock.second);
    }
  else {
    Serial.println("Error reading RTC Clock");
    }
  
  RTC_ALARM alarm;
  bool result = rtc_get_daily_alarm(alarm);
  Serial.printf("RTC Alarm2 : %s @ %02d:%02d\n",  result == true ? "Daily" : "Error", alarm.hour, alarm.minute);
  if ((alarm.hour != Schedule.hour) || (alarm.minute != Schedule.minute) || (result != true)) {
    Serial.println("RTC Alarm setting error, resetting");
    alarm.hour = Schedule.hour;
    alarm.minute = Schedule.minute;
    rtc_set_daily_alarm(alarm);
    }
  
  Serial.println("\nFor WiFi AP mode, press and hold button (GPIO0) until you hear a long tone");
  // beep 10 times to allow enough time for pressing GPIO0 button if required
  beep(pinBuzzer,1000, 100, 100,10); 
    
  if (digitalRead(pinConfigBtn) == 0) {
    // GPIO0 button was pressed
    Serial.println("\n====== Access Point Configuration Mode ======");
    Serial.println("SSID=EspTimer Password=123456789  URL=http://192.168.4.1");
    // 3 second long 800Hz tone to indicate unit is now in standalone Access Point and 
    // web server configuration mode.
    tone_generate(pinBuzzer, 800, 3000);
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
      }
    wifi_access_point_init(); 
    }   
  else {
    Serial.println("\n====== Normal Watering Mode ======");
    GS_DATA_t gsData;
    analogRead(pinADCSensor); // dummy sensor read, throwaway sample
    delay(50);
    // date& time stamp. sensor and threshold data 
    gsData.month = Clock.month;
    gsData.day = Clock.dayOfMonth;
    gsData.hour = Clock.hour;
    gsData.minute = Clock.minute;
    gsData.sensorReading = sensor_reading();
    gsData.batteryVoltage = battery_voltage();
    gsData.superCapVoltage = supercap_voltage();
    gsData.sensorThreshold = Schedule.sensorThreshold;
    Serial.printf("Battery Voltage %.1fV\n", gsData.batteryVoltage);
    Serial.printf("SuperCap Voltage %.1fV\n", gsData.superCapVoltage);
    Serial.printf("Sensor Threshold %d\n", gsData.sensorThreshold);
    Serial.printf("Sensor Reading %d\n", gsData.sensorReading); 
    
    if (gsData.sensorReading > (int)Schedule.sensorThreshold) {
      // soil is dry, needs watering
      gsData.onTimeSeconds = Schedule.onTimeSeconds;
      Serial.printf("On Time %d secs\n", gsData.onTimeSeconds);
	    digitalWrite(pinPumpSwitch, HIGH);
      // beep 0.5secs on, 0.5secs off while pump is on 
    	beep(pinBuzzer,1000, 500, 500, Schedule.onTimeSeconds); 
    	digitalWrite(pinPumpSwitch, LOW);
      Serial.println("Watering done");
    	}
    else {
      // soil is still damp
      gsData.onTimeSeconds = 0;
      Serial.printf("On Time %d secs\n", gsData.onTimeSeconds);
      Serial.println("Watering not required");
    	}
    Serial.println("Setting alarm for next day");
    alarm.hour = Schedule.hour;
    alarm.minute = Schedule.minute;
    //rtc_set_daily_alarm(alarm);
    // update Google Sheet if required
    if (GSConfig.update){
      Serial.println("Updating google sheet - AutoWater");
      if (gs_init() == true) {
        // connected to Internet access point
        // if there are unsent data records, upload them first starting from the oldest
        while (LogBuffer.numEntries > 0) {
          GS_DATA_t logData;
          log_buffer_dequeue(LogBuffer, logData);
          gs_update(logData);
          delay(500);
          }
        // upload today's data
        gs_update(gsData);
        }
      else {
        // unable to connect to internet, append today's data to the buffer queue
        // if buffer is full, oldest entry is over-written
        log_buffer_enqueue(LogBuffer, gsData);
        }
     log_buffer_store(LogBuffer);
     }

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


  
