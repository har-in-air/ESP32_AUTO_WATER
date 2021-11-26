#include <Arduino.h>
#include <Preferences.h>
#include "nvdata.h"

Preferences Prefs;
SCHEDULE_t Schedule;
GS_CONFIG_t GSConfig;
LOG_BUFFER_t LogBuffer;

#define MODE_READ_WRITE  false
#define MODE_READ_ONLY   true

#define DEFAULT_SCHEDULE_HOUR     12
#define DEFAULT_SCHEDULE_MINUTE   0
#define DEFAULT_SENSOR_THRESHOLD  400
#define DEFAULT_ON_TIME_SECONDS   20

#define DEFAULT_GS_UPDATE        1
#define DEFAULT_GS_ID           "google_sheet_id"  
#define DEFAULT_GS_SHEET        "sheet_name"  
#define DEFAULT_WIFI_SSID       "ssid"
#define DEFAULT_WIFI_PASSWORD   "password"
#define DEFAULT_UTC_OFFSET_SECS    (330*60)
#define DEFAULT_DAYLIGHT_OFFSET_SECS    (0)


void  gs_config_load(GS_CONFIG_t &gsConfig){
  if (Prefs.begin("gsconfig", MODE_READ_ONLY) == false) {
    Serial.println("Preferences 'gsconfig' namespace not found, creating with defaults.");
    Prefs.end();
    gs_config_reset(gsConfig);
    } 
  else {
    gsConfig.update = Prefs.getUInt("update", DEFAULT_GS_UPDATE);
    gsConfig.gsID = Prefs.getString("gsID", DEFAULT_GS_ID);
    gsConfig.gsSheet = Prefs.getString("gsSheet", DEFAULT_GS_SHEET);
    gsConfig.wifiSSID = Prefs.getString("wifiSSID", DEFAULT_WIFI_SSID);
    gsConfig.wifiPassword = Prefs.getString("wifiPassword", DEFAULT_WIFI_PASSWORD);
    gsConfig.utcOffsetSeconds = Prefs.getInt("utcOffset", DEFAULT_UTC_OFFSET_SECS);
    gsConfig.daylightOffsetSeconds = Prefs.getInt("daylightOffset", DEFAULT_DAYLIGHT_OFFSET_SECS);
    Prefs.end();
    }
}

void gs_config_reset(GS_CONFIG_t &gsConfig) {
  gsConfig.update = DEFAULT_GS_UPDATE;
  gsConfig.gsID = DEFAULT_GS_ID; 
  gsConfig.gsSheet = DEFAULT_GS_SHEET;
  gsConfig.wifiSSID = DEFAULT_WIFI_SSID;
  gsConfig.wifiPassword = DEFAULT_WIFI_PASSWORD;
  gsConfig.utcOffsetSeconds = DEFAULT_UTC_OFFSET_SECS;
  gsConfig.daylightOffsetSeconds = DEFAULT_DAYLIGHT_OFFSET_SECS;
  gs_config_store(gsConfig);
  }

void gs_config_store(GS_CONFIG_t &gsConfig){
  Prefs.begin("gsconfig", MODE_READ_WRITE);
  Prefs.clear(); 
  Prefs.putUInt("update", gsConfig.update); 
  Prefs.putString("gsID", gsConfig.gsID); 
  Prefs.putString("gsSheet", gsConfig.gsSheet); 
  Prefs.putString("wifiSSID", gsConfig.wifiSSID); 
  Prefs.putString("wifiPassword", gsConfig.wifiPassword); 
  Prefs.putInt("utcOffset", gsConfig.utcOffsetSeconds);
  Prefs.putInt("daylightOffset", gsConfig.daylightOffsetSeconds);
  Prefs.end();
  }


void schedule_load(SCHEDULE_t &schedule) {
  if (Prefs.begin("schedule", MODE_READ_ONLY) == false) {
    Serial.println("Preferences 'schedule' namespace not found, creating with defaults.");
    Prefs.end();
    schedule_reset(schedule);
    } 
  else {
    schedule.hour = Prefs.getUInt("hour", DEFAULT_SCHEDULE_HOUR);
    schedule.minute = Prefs.getUInt("minute", DEFAULT_SCHEDULE_MINUTE);
    schedule.sensorThreshold = Prefs.getUInt("sensorThreshold", DEFAULT_SENSOR_THRESHOLD);
    schedule.onTimeSeconds = Prefs.getUInt("onTimeSeconds", DEFAULT_ON_TIME_SECONDS);
    Prefs.end();
    }
  }  


void schedule_reset(SCHEDULE_t &schedule){
  schedule.hour = DEFAULT_SCHEDULE_HOUR;
  schedule.minute = DEFAULT_SCHEDULE_MINUTE;
  schedule.sensorThreshold = DEFAULT_SENSOR_THRESHOLD;
  schedule.onTimeSeconds = DEFAULT_ON_TIME_SECONDS;
  schedule_store(schedule);
  }


void schedule_store(SCHEDULE_t &schedule) {
  Prefs.begin("schedule", MODE_READ_WRITE); 
  Prefs.clear();
  Prefs.putUInt("hour", schedule.hour);
  Prefs.putUInt("minute", schedule.minute);
  Prefs.putUInt("sensorThreshold", schedule.sensorThreshold);
  Prefs.putUInt("onTimeSeconds", schedule.onTimeSeconds);
  Prefs.end();
  }


void log_buffer_load(LOG_BUFFER_t &logBuffer) {
  if (Prefs.begin("logbuffer", MODE_READ_ONLY) == false) {
    Serial.println("Preferences 'logbuffer' namespace not found, creating with defaults.");
    Prefs.end();
    log_buffer_reset(logBuffer);
    } 
  else {
    if (Prefs.getBytesLength("logbuffer") != sizeof(LOG_BUFFER_t)) {
      Serial.println("Prefs : logbuffer<->LOG_BUFFER_t size mismatch, resetting.");
      Prefs.end();
      log_buffer_reset(logBuffer);
      }
    else {
      Prefs.getBytes("logbuffer", (void*)&logBuffer, sizeof(LOG_BUFFER_t));
      Prefs.end();
      // sanity check
      if (logBuffer.numEntries > NUM_LOG_RECORDS) {
        Serial.println("Prefs : logbuffer numEntries > NUM_LOG_RECORDS, resetting.");
        log_buffer_reset(logBuffer);
        }
      }
    }
  }  


void log_buffer_store(LOG_BUFFER_t &logBuffer) {
  Prefs.begin("logbuffer", MODE_READ_WRITE);
  Prefs.clear();
  Prefs.putBytes("logbuffer", (const void*)&logBuffer, sizeof(LOG_BUFFER_t));
  Prefs.end();
  }


void log_buffer_reset(LOG_BUFFER_t &logBuffer){
  memset((void*)&logBuffer, 0 , sizeof(LOG_BUFFER_t));
  log_buffer_store(logBuffer);
  }

bool log_buffer_enqueue(LOG_BUFFER_t &logBuffer, GS_DATA_t &gsData){
  // if buffer is full (logBuffer.numEntries == NUM_LOG_RECORDS)
  // oldest entry is over-written with new record 
  int index = logBuffer.oldestIndex - logBuffer.numEntries;
  if (index < 0) index += NUM_LOG_RECORDS;
  memcpy(&(logBuffer.log[index]), &gsData, sizeof(GS_DATA_t));
  logBuffer.numEntries++;
  if (logBuffer.numEntries > NUM_LOG_RECORDS) {
    logBuffer.numEntries = NUM_LOG_RECORDS; 
    }
  return true;
  }


bool log_buffer_dequeue(LOG_BUFFER_t &logBuffer, GS_DATA_t &gsData){
  if (logBuffer.numEntries == 0) {
    Serial.println("Log Buffer empty, cannot dequeue record");
    return false;
    }
  int index = logBuffer.oldestIndex;
  memcpy(&gsData, &(logBuffer.log[index]), sizeof(GS_DATA_t));
  index--;
  if (index < 0) index += NUM_LOG_RECORDS;
  logBuffer.oldestIndex = index;
  logBuffer.numEntries--;
  return true;
  }  

