#include <Arduino.h>
#include <Preferences.h>
#include "nvdata.h"

Preferences Prefs;
SCHEDULE_t Schedule;
GS_CONFIG_t GSConfig;
LOG_BUFFER_t LogBuffer;

#define DEFAULT_SCHEDULE_HOUR     11
#define DEFAULT_SCHEDULE_MINUTE   0
#define DEFAULT_SENSOR_THRESHOLD  600
#define DEFAULT_ON_TIME_SECONDS   20

#define DEFAULT_GS_UPDATE     0  
#define DEFAULT_WIFI_SSID     "ssid"
#define DEFAULT_WIFI_PASSWORD "password"

void  gs_config_load(GS_CONFIG_t &gsConfig){
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

void gs_config_store(GS_CONFIG_t &gsConfig){
  Prefs.begin("gs_config", false); // read/write
  //Prefs.clear();
  Prefs.putUInt("update", gsConfig.update); 
  Prefs.putString("wifiSSID", gsConfig.wifiSSID); 
  Prefs.putString("wifiPassword", gsConfig.wifiPassword); 
  Prefs.end();
  }


void schedule_load(SCHEDULE_t &schedule) {
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


void schedule_store(SCHEDULE_t &schedule) {
  Prefs.begin("schedule", false); // read/write mode
  //Prefs.clear();
  Prefs.putUInt("hour", schedule.hour);
  Prefs.putUInt("minute", schedule.minute);
  Prefs.putUInt("sensorThreshold", schedule.sensorThreshold);
  Prefs.putUInt("onTimeSeconds", schedule.onTimeSeconds);
  Prefs.end();
  }

void log_buffer_load(LOG_BUFFER_t &logBuffer) {
  // open in read-only mode
  if (Prefs.begin("logbuffer", true) == false) {
    Prefs.end();
    log_buffer_clear(logBuffer);
    log_buffer_store(logBuffer);
    } 
  else {
    Prefs.getBytes("logbuffer", (void*)&logBuffer, sizeof(LOG_BUFFER_t));
    Prefs.end();
    // sanity check
    if (logBuffer.numEntries > NUM_LOG_RECORDS) {
      log_buffer_clear(logBuffer);
      log_buffer_store(logBuffer);
      }
    }
  }  


void log_buffer_store(LOG_BUFFER_t &logBuffer) {
  Prefs.begin("logbuffer", false); // read/write mode
  Prefs.putBytes("logbuffer", (const void*)&logBuffer, sizeof(LOG_BUFFER_t));
  Prefs.end();
  }

void log_buffer_clear(LOG_BUFFER_t &logBuffer) {
  memset((void*)&logBuffer, 0 , sizeof(LOG_BUFFER_t));
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

