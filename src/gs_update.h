#ifndef GS_UPDATE_H_
#define GS_UPDATE_H_

// google sheet  update data fields
typedef struct GS_DATA_ {
  int sensorReading;
  int sensorThreshold;
  int onTimeSeconds;
  float batteryVoltage;
  float superCapVoltage;
} GS_DATA;


bool gs_init();
bool gs_update(GS_DATA &data);

#endif
