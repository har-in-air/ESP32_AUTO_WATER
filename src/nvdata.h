#ifndef NVDATA_H_
#define NVDATA_H_


#define SENSOR_THRESHOLD_MIN  		500
#define SENSOR_THRESHOLD_MAX  		1000

#define ON_TIME_MIN_SECONDS       5
#define ON_TIME_MAX_SECONDS       30

// schedule to save in preferences
typedef struct SCHEDULE_ {
  uint32_t hour;
  uint32_t minute;
  uint32_t sensorThreshold;    // desired dry threshold, if reading > threshold, water
  uint32_t onTimeSeconds; // watering time in seconds
  } SCHEDULE;

typedef struct GS_CONFIG_ {
  uint32_t update;
  String wifiSSID;
  String wifiPassword;
} GS_CONFIG;

extern SCHEDULE Schedule;
extern GS_CONFIG GSConfig;

void schedule_store(SCHEDULE &schedule);
void schedule_load(SCHEDULE &schedule);

void gs_config_store(GS_CONFIG &gsConfig);
void gs_config_load(GS_CONFIG &gsConfig);
 
#endif
