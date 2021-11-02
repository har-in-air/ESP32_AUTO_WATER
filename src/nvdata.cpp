#include <Arduino.h>
#include <Preferences.h>
#include "nvdata.h"

Preferences Prefs;
SCHEDULE Schedule;
GS_CONFIG GSConfig;

#define DEFAULT_SCHEDULE_HOUR     11
#define DEFAULT_SCHEDULE_MINUTE   0
#define DEFAULT_SENSOR_THRESHOLD  600
#define DEFAULT_ON_TIME_SECONDS   20

#define DEFAULT_GS_UPDATE     0  
#define DEFAULT_WIFI_SSID     "ssid"
#define DEFAULT_WIFI_PASSWORD "password"

void  gs_config_load(GS_CONFIG &gsConfig){
  // open in read only mode
  if (Prefs.begin("gs_config", true) == false) {
    Prefs.end();
    gsConfig.update = DEFAULT_GS_UPDATE;
    gsConfig.wifiSSID = DEFAULT_WIFI_SSID;
    gsConfig.wifiPassword = DEFAULT_WIFI_PASSWORD;
    gs_config_store(gsConfig);
    } 
  else {
    gsConfig.update = Prefs.getUInt("update", DEFAULT_GS_UPDATE);
    gsConfig.wifiSSID = Prefs.getString("wifiSSID", DEFAULT_WIFI_SSID);
    gsConfig.wifiPassword = Prefs.getString("wifiPassword", DEFAULT_WIFI_PASSWORD);
    Prefs.end();
    }
}

void gs_config_store(GS_CONFIG &gsConfig){
  Prefs.begin("gs_config", false); // read/write
  Prefs.clear();
  Prefs.putUInt("update", gsConfig.update); 
  Prefs.putString("wifiSSID", gsConfig.wifiSSID); 
  Prefs.putString("wifiPassword", gsConfig.wifiPassword); 
  Prefs.end();
  }


void schedule_load(SCHEDULE &schedule) {
  // open in read-only mode
  if (Prefs.begin("schedule", true) == false) {
    Prefs.end();
    schedule.hour = DEFAULT_SCHEDULE_HOUR;
    schedule.minute = DEFAULT_SCHEDULE_MINUTE;
    schedule.sensorThreshold = DEFAULT_SENSOR_THRESHOLD;
    schedule.onTimeSeconds = DEFAULT_ON_TIME_SECONDS;
    schedule_store(schedule);
    } 
  else {
    schedule.hour = Prefs.getUInt("hour", DEFAULT_SCHEDULE_HOUR);
    schedule.minute = Prefs.getUInt("minute", DEFAULT_SCHEDULE_MINUTE);
    schedule.sensorThreshold = Prefs.getUInt("sensorThreshold", DEFAULT_SENSOR_THRESHOLD);
    schedule.onTimeSeconds = Prefs.getUInt("onTimeSeconds", DEFAULT_ON_TIME_SECONDS);
    Prefs.end();
    }
  }  


void schedule_store(SCHEDULE &schedule) {
  Prefs.begin("schedule", false); // read/write mode
  Prefs.clear();
  Prefs.putUInt("hour", schedule.hour);
  Prefs.putUInt("minute", schedule.minute);
  Prefs.putUInt("sensorThreshold", schedule.sensorThreshold);
  Prefs.putUInt("onTimeSeconds", schedule.onTimeSeconds);
  Prefs.end();
  }


