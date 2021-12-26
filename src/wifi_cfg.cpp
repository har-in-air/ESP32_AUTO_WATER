#include <Arduino.h>
#include <WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <driver/timer.h>
#include <esp_wifi.h>
#include "rtc.h"
#include "nv_data.h"
#include "sensor.h"
#include "wifi_cfg.h"

static const char* TAG = "wificfg";

extern const char* FirmwareRevision;

//#define STATION

const char* szAPSSID = "EspC3WaterTimer";
const char* szAPPassword = "123456789";

AsyncWebServer* pServer = NULL;

static int SensorReading;
static float BatteryVoltage;
static float SuperCapVoltage;

static String string_processor(const String& var);

static void not_found_handler(AsyncWebServerRequest *request);
static void index_page_handler(AsyncWebServerRequest *request);
static void set_defaults_handler(AsyncWebServerRequest *request);
static void css_handler(AsyncWebServerRequest *request);
static void get_handler(AsyncWebServerRequest *request);

// Replace %txt% placeholder 
static String string_processor(const String& var){
	if(var == "FIRMWARE_REVISION"){
		return FirmwareRevision;
		}
	else
	// Google Sheet update checkbox
	if(var == "GS_UPDATE_OFF"){
		return GSConfig.update ? "" : "checked";
		}
	else
	if(var == "GS_UPDATE_ON"){
		return GSConfig.update ? "checked" : "";
		}
	else
	if(var == "DISPLAY_INTERNET"){
		return GSConfig.update ? "" : "none";
		}
	else
	// Google Sheet ID, Internet access credentials
	if(var == "GS_ID"){
		return GSConfig.gsID;
		}
	else
	if(var == "GS_SHEET"){
		return GSConfig.gsSheet;
		}
	else
	if(var == "WIFI_SSID"){
		return GSConfig.wifiSSID;
		}
	else
	if(var == "WIFI_PASSWORD"){
		return GSConfig.wifiPassword;
		}
	else
	if(var == "UTC_OFFSET"){
		return String(GSConfig.utcOffsetSeconds/60);
		}
	else
	if(var == "DAYLIGHT_OFFSET"){
		return String(GSConfig.daylightOffsetSeconds/60);
		}
	else
	// current sensor readings
	if(var == "BATTERY_VOLTAGE"){
		return String(BatteryVoltage, 1);
		}
	else
	if(var == "SUPERCAP_VOLTAGE"){
		return String(SuperCapVoltage, 1);
		}
	else
	if(var == "SENSOR_READING"){
		return String(SensorReading);
		}
	else
	// desired schedule
	if(var == "SCHEDULE_HOUR"){
		return String(Schedule.hour);
		}
	else
	if(var == "SCHEDULE_MINUTE"){
		return String(Schedule.minute);
		}
	else
	if(var == "SCHEDULE_SENSOR_THRESHOLD"){
		return String(Schedule.sensorThreshold);
		}
	else
	if(var == "ON_TIME_SECONDS"){
		return String(Schedule.onTimeSeconds);
		}
	else
	// RTC current clock 
	if(var == "RTC_YEAR"){
		return String(1900+Clock.tm_year);
		}
	else
	if(var == "RTC_MONTH"){
		return  Clock.tm_mon < 9 ? "0"+String(1+Clock.tm_mon) : String(1+Clock.tm_mon);
		}
	else
	if(var == "RTC_DOM"){
		return Clock.tm_mday < 10 ? "0"+String(Clock.tm_mday) : String(Clock.tm_mday);
		}
	else
	if(var == "RTC_DOW"){
		return String(SzDayOfWeek[Clock.tm_wday]);
		}
	else
	if(var == "RTC_HOUR"){
		return Clock.tm_hour < 10 ? "0"+String(Clock.tm_hour) : String(Clock.tm_hour);
		}
	else
	if(var == "RTC_MINUTE"){
		return Clock.tm_min < 10 ? "0"+String(Clock.tm_min) : String(Clock.tm_min);
		}
	else
	if(var == "RTC_SECOND"){
		return Clock.tm_sec < 10 ? "0"+String(Clock.tm_sec) : String(Clock.tm_sec);
		}
	else return "?";
	}


void wifi_off() {
    WiFi.mode(WIFI_MODE_NULL);
    esp_wifi_stop();
    delay(100);
    }

static void not_found_handler(AsyncWebServerRequest *request) {
	request->send(404, "text/plain", "Not found");
	}

static void index_page_handler(AsyncWebServerRequest *request) {
    rtc_get_clock(Clock);
    SensorReading = sensor_reading();
    BatteryVoltage = battery_voltage();
    SuperCapVoltage = supercap_voltage();
    request->send(LittleFS, "/index.html", String(), false, string_processor);
    }

static void css_handler(AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
    }

static void set_defaults_handler(AsyncWebServerRequest *request) {
    gs_config_reset(GSConfig);
    schedule_reset(Schedule);
    log_buffer_reset(LogBuffer);
    request->send(200, "text/html", "Default options set<br><a href=\"/\">Return to Home Page</a>");  
    }

static void get_handler(AsyncWebServerRequest *request) {
    String inputMessage;
    bool bScheduleChange = false;
    bool bRTCChange = false;
    bool bGSConfigChange = false;
    // Google Sheet update option
    if (request->hasParam("gsUpdate")) {
        inputMessage = request->getParam("gsUpdate")->value();
        bGSConfigChange = true; 
        GSConfig.update = (inputMessage == "off" ? 0 : 1); 
        }
    // Google Sheet, Internet Access Credentials  
    if (request->hasParam("gsID")) {
        inputMessage = request->getParam("gsID")->value();
        bGSConfigChange = true; 
        GSConfig.gsID = inputMessage;
        }
    if (request->hasParam("gsSheet")) {
        inputMessage = request->getParam("gsSheet")->value();
        bGSConfigChange = true; 
        GSConfig.gsSheet = inputMessage;
        }
    if (request->hasParam("wifiSSID")) {
        inputMessage = request->getParam("wifiSSID")->value();
        bGSConfigChange = true; 
        GSConfig.wifiSSID = inputMessage;
        }
    if (request->hasParam("wifiPassword")) {
        inputMessage = request->getParam("wifiPassword")->value();
        bGSConfigChange = true; 
        GSConfig.wifiPassword = inputMessage;
        }
    if (request->hasParam("utcOffset")) {
        inputMessage = request->getParam("utcOffset")->value();
        bGSConfigChange = true; 
        GSConfig.utcOffsetSeconds = 60*inputMessage.toInt();
        }
    if (request->hasParam("daylightOffset")) {
        inputMessage = request->getParam("daylightOffset")->value();
        bGSConfigChange = true; 
        GSConfig.daylightOffsetSeconds = 60*inputMessage.toInt();
        }
      
    // wake-up time hour and minute 
    if (request->hasParam("scheduleHour")) {
        inputMessage = request->getParam("scheduleHour")->value();
        bScheduleChange = true; 
        Schedule.hour = (uint32_t)inputMessage.toInt();
        }
    if (request->hasParam("scheduleMinute")) {
        inputMessage = request->getParam("scheduleMinute")->value();
        bScheduleChange = true; 
        Schedule.minute = (uint32_t)inputMessage.toInt();
        }
    // moisture sensor dry threshold. if reading > threshold, turn on pump  
    if (request->hasParam("scheduleSensorThreshold")) {
        inputMessage = request->getParam("scheduleSensorThreshold")->value();
        bScheduleChange = true; 
        Schedule.sensorThreshold = (uint32_t)inputMessage.toInt();
        }
    // Pump on-time  
    if (request->hasParam("scheduleOnTimeSeconds")) {
        inputMessage = request->getParam("scheduleOnTimeSeconds")->value();
        bScheduleChange = true; 
        Schedule.onTimeSeconds = (uint32_t)inputMessage.toInt();
        }    

    // RTC Clock setting
    ClockSet = Clock;  
    if (request->hasParam("rtcYear")) {
        inputMessage = request->getParam("rtcYear")->value();
        bRTCChange = true; 
        ClockSet.tm_year = 100 + inputMessage.toInt();
        }
    if (request->hasParam("rtcMonth")) {
        inputMessage = request->getParam("rtcMonth")->value();
        bRTCChange = true; 
        ClockSet.tm_mon = inputMessage.toInt() - 1; // tm uses 0-11, not 1-12
        }
    if (request->hasParam("rtcDayOfMonth")) {
        inputMessage = request->getParam("rtcDayOfMonth")->value();
        bRTCChange = true; 
        ClockSet.tm_mday = inputMessage.toInt();
        }
    if (request->hasParam("rtcDayOfWeek")) {
        inputMessage = request->getParam("rtcDayOfWeek")->value();
        bRTCChange = true; 
        ClockSet.tm_wday = inputMessage.toInt();
        }
    if (request->hasParam("rtcHour")) {
        inputMessage = request->getParam("rtcHour")->value();
        bRTCChange = true; 
        ClockSet.tm_hour = inputMessage.toInt();
        }
    if (request->hasParam("rtcMinute")) {
        inputMessage = request->getParam("rtcMinute")->value();
        bRTCChange = true; 
        ClockSet.tm_min = inputMessage.toInt();
        }
    if (request->hasParam("rtcSecond")) {
        inputMessage = request->getParam("rtcSecond")->value();
        bRTCChange = true; 
        ClockSet.tm_sec = inputMessage.toInt();
        }

    if (bRTCChange == true) {
        Serial.println("RTC Clock changed");
        Serial.printf("Set RTC Clock to : %04d-%02d-%02d %s %02d:%02d:%02d\n", 
        1900+ClockSet.tm_year, 1+ClockSet.tm_mon, ClockSet.tm_mday, SzDayOfWeek[ClockSet.tm_wday], ClockSet.tm_hour, ClockSet.tm_min, ClockSet.tm_sec);
        rtc_set_clock(ClockSet);
        Clock = ClockSet;
        bRTCChange = false;
        }
    if (bGSConfigChange == true) {
        Serial.println("Google Sheet/Internet Access parameters changed");
        Serial.printf("update %d\n", GSConfig.update);
        gs_config_store(GSConfig);
        bGSConfigChange = false;
        }
    if (bScheduleChange == true) {
        Serial.printf("Watering options changed, saving schedule and resetting RTC daily alarm\n");
        schedule_store(Schedule);
        RTC_ALARM alarm;
        alarm.hour = Schedule.hour;
        alarm.minute = Schedule.minute;
        rtc_set_daily_alarm(alarm);
        bScheduleChange = false;
        }
    request->send(200, "text/html", "Input Processed<br><a href=\"/\">Return to Home Page</a>");  
    }


void wifi_access_point_init() {
    esp_wifi_start(); // necessary if esp_wifi_stop() was called before deep_sleep
    delay(100);
#ifdef STATION  
    WiFi.mode(WIFI_STA);
    WiFi.begin(GoogleSheet.ssid.c_str(), GoogleSheet.password.c_str());
    if (WiFi.waitForConnectResult(10000UL) != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return; 
    }
    Serial.println();
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
#else    
    Serial.print("Setting AP (Access Point)");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(szAPSSID, szAPPassword);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    Serial.print("Local IP address: ");
    Serial.println(WiFi.localIP());
#endif

    pServer = new AsyncWebServer(80);
    if (pServer == NULL) {
        ESP_LOGE(TAG, "Error creating AsyncWebServer!");
        while(1);
        }

    pServer->onNotFound(not_found_handler);
    pServer->on("/", HTTP_GET, index_page_handler);
    pServer->on("/style.css", HTTP_GET, css_handler);
    pServer->on("/defaults", HTTP_GET, set_defaults_handler);
    pServer->on("/get", HTTP_GET, get_handler);

    // add support for OTA firmware update
    AsyncElegantOTA.begin(pServer);  
    pServer->begin();   
    }


