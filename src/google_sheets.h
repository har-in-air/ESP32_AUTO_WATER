#ifndef GOOGLE_SHEETS_H_
#define GOOGLE_SHEETS_H_

typedef struct GS_DATA_ {
  int sensorReading;
  int sensorThreshold;
  int onTimeSeconds;
  float batteryVoltage;
  float superCapVoltage;
} GS_DATA;


void gs_init();
void gs_update(GS_DATA &data);

#endif
