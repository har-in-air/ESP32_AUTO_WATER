#include <Arduino.h>
#include <Preferences.h>
#include "nvdata.h"

Preferences Prefs;
SCHEDULE Schedule;
WIFI_CREDENTIALS WiFiCredentials;

#define DEFAULT_SSID "ssid"
#define DEFAULT_PASSWORD "password"

void  wifi_credentials_load(WIFI_CREDENTIALS &cred){
  Prefs.begin("wifi", true); // read-only
  cred.ssid = Prefs.getString("ssid", DEFAULT_SSID);
  cred.password = Prefs.getString("password", DEFAULT_PASSWORD);
  Prefs.end();
}

void  wifi_credentials_store(WIFI_CREDENTIALS &cred){
  Prefs.begin("wifi", false); // read/write
  Prefs.clear();
  Prefs.putString("ssid", cred.ssid); 
  Prefs.putString("password", cred.password); 
  Prefs.end();
}

void schedule_store(SCHEDULE &schedule) {
  Serial.printf("Storing Schedule\n\ttime = %02d:%02d\n\tsensor threshold = %d\n\twatering time = %d secs\n", 
    schedule.hour, schedule.minute, schedule.sensorThreshold, schedule.onTimeSeconds);
  Prefs.begin("schedule", false); // read/write mode
  Prefs.clear();
  Prefs.putUInt("hour", schedule.hour);
  Prefs.putUInt("minute", schedule.minute);
  Prefs.putUInt("sensorThreshold", schedule.sensorThreshold);
  Prefs.putUInt("onTimeSeconds", schedule.onTimeSeconds);
  Prefs.end();
  }


void schedule_load(SCHEDULE &schedule) {
  Prefs.begin("schedule", true); // read-only
  schedule.hour = Prefs.getUInt("hour", DEFAULT_HOUR);
  schedule.minute = Prefs.getUInt("minute", DEFAULT_MINUTE);
  schedule.sensorThreshold = Prefs.getUInt("sensorThreshold", DEFAULT_SENSOR_THRESHOLD);
  schedule.onTimeSeconds = Prefs.getUInt("onTimeSeconds", DEFAULT_ON_TIME_SECONDS);
  Prefs.end();
  Serial.printf("Loaded Schedule\n\ttime = %02d:%02d\n\tsensor threshold = %d\n\tton time = %d secs\n", 
    schedule.hour, schedule.minute,schedule.sensorThreshold, schedule.onTimeSeconds);
  }
  
