# ESP32_AUTO_WATER

## Power supply

* Solar panel<br>
A 20V solar panel is used to charge the 6V lead acid battery via a CC-CV module. It is also used to charge up a bank of 10F 2.7V supercaps, as it's possible that at the scheduled watering time, it may be cloudy enough that the water pump will not run off the solar panel.

* Lead acid battery 6V 1.3Ah<br>
This is used to provide power for the ESP32 module via a 3.3V LDO regulator. You could use a 3.7V li-ion battery instead with a Li-ion MPPT solar charger module.<br>
I used an HT7333 LDO regulator because it draws very little quiescent current (< 5uA) and can handle higher input voltages. When the ESP32 module is active the current draw is approximately 25mA on average. <br>
When the ESP32 module is in deep sleep, the circuits drawing quiescent power are :
- HT7333 regulator
- DS3231 RTC
- The resistors providing the sensed voltages to the ESP32 ADC. I used 500K potentiometers to minimize the current draw, with a 100nF capacitor on the ADC pins to minimize noise due to the high source impedance.

* Supercapacitor bank<br>
I used eight 10F 2.7V supercapacitors in series to provide an energy storage bank to power the 12V water pump.<br>
Even if it's a cloudy day, the capacitor bank charges up to approximately 18V.<br>
I used a series 1N4007 diode from the solar panel so that the capacitor bank does not discharge back through the panel or the CC-CV module. Once charged there's enough juice in the supercapacitor bank to run the 12V water pump for 30seconds.

### DS3231 RTC 
This provides the daily alarm interrupt at the scheduled time to wake up the ESP32 from deep sleep. A 4.7uF capacitor in series between the DS3231 INT/SQW pin and the ESP32 EN pin generates the negative going reset pulse necessary for ESP32 wake up.

For the ESP32 EN pin, I used a 2K2 resistor pullup to VCC and a 1uF ceramic cap to ground. This is enough to avoid ESP32 reset problems while still allowing the DS3231 reset pulse to work.

A CR2032 coin cell battery provides backup for the DS3231 RTC. 

### Capacitive water sensor
[Ensure you have a working capacitive sensor module](https://www.youtube.com/watch?v=IGP38bz-K48). The version I have uses a 555 timer IC marked "NE555 20M". 

I sealed the electronics back-end of the module with silicone caulk and a heatshrink tube to prevent any corrosion of the electronics. A 1-meter shielded cable provides ground, 3.3V and sensor output interface to the ESP32.

It's possible for the top soil layer to dry out while the roots are still in damp soil. So the sensor is placed halfway down the side of the plant pot, 

### CC CV battery charger module
LM2596 module used to charge the 6V 1.3Ah lead acid battery. I set open circuit voltage (CV) to 7.2V, short-circuit current (CC) to 0.25A.