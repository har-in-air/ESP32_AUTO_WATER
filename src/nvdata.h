#ifndef NVDATA_H_
#define NVDATA_H_

#define DEFAULT_HOUR              11
#define DEFAULT_MINUTE            0
#define DEFAULT_SENSOR_THRESHOLD  600


#define SENSOR_THRESHOLD_MIN  		600
#define SENSOR_THRESHOLD_MAX  		1000

#define DEFAULT_ON_TIME_SECONDS   20
#define ON_TIME_MIN_SECONDS       5
#define ON_TIME_MAX_SECONDS       30

// schedule to save in preferences
typedef struct SCHEDULE_ {
  uint32_t hour;
  uint32_t minute;
  uint32_t sensorThreshold;    // desired dry threshold, if reading > threshold, water
  uint32_t onTimeSeconds; // watering time in seconds
  } SCHEDULE;


typedef struct WIFI_CREDENTIALS_ {
  String ssid;
  String password;
} WIFI_CREDENTIALS;

extern SCHEDULE Schedule;
extern WIFI_CREDENTIALS WiFiCredentials;

void schedule_store(SCHEDULE &schedule);
void schedule_load(SCHEDULE &schedule);

void wifi_credentials_store(WIFI_CREDENTIALS &cred);
void wifi_credentials_load(WIFI_CREDENTIALS &cred);
 
#endif
