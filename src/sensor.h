#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>

#define pinADCSensor     34
#define pinADCBattery    35
#define pinADCSuperCap   36 

#define CLAMP(x, minimum, maximum) {if (x > maximum) x = maximum; else if (x < minimum) x = minimum;}

void adc_init();
int sensor_reading();
float battery_voltage();
float supercap_voltage();

#endif
