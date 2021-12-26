#include <Arduino.h>
#include <driver/adc.h>
#include "sensor.h"

void adc_init() {
    adc1_config_width(ADC_WIDTH_BIT_12); // no 10-bit resolution option !?
    adc1_config_channel_atten(CHAN_SENSOR, ADC_ATTEN_DB_0);
    adc1_config_channel_atten(CHAN_VBAT, ADC_ATTEN_DB_0);
    adc1_config_channel_atten(CHAN_VSUPCAP, ADC_ATTEN_DB_0);
    }

// VSENSOR -- 390K --+-- 150K -- GND
// capacitive sensor reading, raw value is enough
int sensor_reading() {
  int sample = 0;
  for (int inx = 0; inx < 4; inx++) {
    sample += adc1_get_raw(CHAN_SENSOR);
    delay(1);
    }
  sample /= 16; // 10-bit resolution  
  return sample;
  }


// VBAT -- 470K --+-- 68K -- GND
// max voltage is ~6V 
// Two-point calibration of slope-intercept (10-bit resolution)
#define BAT_ADC1    539
#define BAT_V1      3.323f
#define BAT_ADC2    1007
#define BAT_V2      6.17f

float battery_voltage() {
    int sample = 0;
    for (int inx = 0; inx < 4; inx++) {
        sample += adc1_get_raw(CHAN_VBAT);
        delay(1);
        }
    sample /= 16;  // 10-bit resolution
    float slope = (BAT_V2 - BAT_V1)/(float)(BAT_ADC2 - BAT_ADC1);
    float voltage = slope*(float)(sample - BAT_ADC1) + BAT_V1;
    return voltage;
    }


// VSCAP -- 1M --+-- 39K -- GND
// max voltage is ~19V
// Two-point calibration of slope-intercept (10-bit resolution)
#define SCAP_ADC1   157
#define SCAP_V1     3.328f
#define SCAP_ADC2   299
#define SCAP_V2     6.21f

float supercap_voltage() {
    int sample = 0;
    for (int inx = 0; inx < 4; inx++) {
        sample += adc1_get_raw(CHAN_VSUPCAP);
        delay(1);
        }
    sample /= 16; // 10 bit resolution  
    float slope = (SCAP_V2 - SCAP_V1)/(float)(SCAP_ADC2 - SCAP_ADC1);
    float voltage = slope*(float)(sample - SCAP_ADC1) + SCAP_V1;
    return voltage;
    }

