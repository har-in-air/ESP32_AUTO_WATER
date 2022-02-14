#ifndef NVDATA_H_
#define NVDATA_H_

#include "gs_update.h"

#define SENSOR_THRESHOLD_MIN  		500
#define SENSOR_THRESHOLD_MAX  		1000

#define ON_TIME_MIN_SECONDS       5
#define ON_TIME_MAX_SECONDS       30

// schedule to save in preferences
typedef struct {
  uint32_t hour;
  uint32_t minute;
  uint32_t sensorThreshold;    // desired dry threshold, if reading > threshold, water
  uint32_t onTimeSeconds; // watering time in seconds
  } SCHEDULE_t;

typedef struct {
  uint32_t update;
  String gsID;
  String gsLogSheet;
  String gsControlSheet;
  String wifiSSID;
  String wifiPassword;
  int32_t utcOffsetSeconds;
  int32_t daylightOffsetSeconds;
} GS_CONFIG_t;


#define NUM_LOG_RECORDS 30

typedef struct {
  uint8_t oldestIndex;
  uint8_t numEntries;
  GS_DATA_t log[NUM_LOG_RECORDS];
} LOG_BUFFER_t;

extern SCHEDULE_t Schedule;
extern GS_CONFIG_t GSConfig;
extern LOG_BUFFER_t LogBuffer;

void schedule_store(SCHEDULE_t &schedule);
void schedule_load(SCHEDULE_t &schedule);
void schedule_reset(SCHEDULE_t &schedule);

void gs_config_store(GS_CONFIG_t &gsConfig);
void gs_config_load(GS_CONFIG_t &gsConfig);
void gs_config_reset(GS_CONFIG_t &gsConfig);

void log_buffer_store(LOG_BUFFER_t &logBuffer);
void log_buffer_load(LOG_BUFFER_t &logBuffer);
void log_buffer_reset(LOG_BUFFER_t &logBuffer);
bool log_buffer_enqueue(LOG_BUFFER_t &logBuffer, GS_DATA_t &gsData);
bool log_buffer_dequeue(LOG_BUFFER_t &logBuffer, GS_DATA_t &gsData);
 
#endif
