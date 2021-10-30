#include <Arduino.h>
#include "sensor.h"



void adc_init() {
  analogReadResolution(10);
  analogSetAttenuation(ADC_0db); // range 0 to 0.8V, using external 500K potentiometer to scale down sensor voltage
  adcAttachPin(pinADCSensor);
  adcAttachPin(pinADCBattery);
  adcAttachPin(pinADCSuperCap);
}

// capacitive sensor  reading 
// 320 saturated soil
// +7days 331
// +10days 339
 
int sensor_reading() {
  int sample = 0;
  for (int inx = 0; inx < 4; inx++) {
    sample += analogRead(pinADCSensor); // 10bit resolution
    delay(10);
    }
  sample /= 4;  
  return sample;
  }

// 3.84v 444
// 6.2v  762
float battery_voltage() {
  int sample = 0;
  for (int inx = 0; inx < 4; inx++) {
    sample += analogRead(pinADCBattery); // 10bit resolution
    delay(10);
    }
  sample /= 4;  
  float slope = (6.2f-3.84f)/(762.0f-444.0f);
  float voltage = slope*(sample - 444.0f) + 3.84f;
  return voltage;
  }

// 5.74v 183
// 11.07v 420
float supercap_voltage() {
  int sample = 0;
  for (int inx = 0; inx < 4; inx++) {
    sample += analogRead(pinADCSuperCap); // 10bit resolution
    delay(10);
    }
  sample /= 4;  
  float slope = (11.07f-5.74f)/(420.0f-183.0f);
  float voltage = slope*(sample - 183.0f) + 5.74f;
  return voltage;
  }

