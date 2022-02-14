#ifndef GS_UPDATE_H_
#define GS_UPDATE_H_

// google sheet row update record
typedef struct {
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint16_t sensorReading;
  uint16_t sensorThreshold;
  uint8_t onTimeSeconds;
  float batteryVoltage;
  float superCapVoltage;
  int32_t rtcError;
} GS_DATA_t;

#define RTC_ERROR_NOT_CALC 9999

bool gs_init();
bool gs_update(GS_DATA_t &data, String& control);

#endif
