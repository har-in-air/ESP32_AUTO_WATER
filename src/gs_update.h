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
} GS_DATA_t;


bool gs_init();
bool gs_update(GS_DATA_t &data);

#endif
