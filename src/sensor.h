#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>

#define CHAN_SENSOR  ADC1_CHANNEL_0 //((adc1_channel_t)0)
#define CHAN_VBAT    ADC1_CHANNEL_3 //((adc1_channel_t)3)
#define CHAN_VSUPCAP ADC1_CHANNEL_4 //((adc1_channel_t)4)

#define CLAMP(x, minimum, maximum)  {if (x > maximum) x = maximum; else if (x < minimum) x = minimum;}

void adc_init();
int sensor_reading();
float battery_voltage();
float supercap_voltage();

#endif
