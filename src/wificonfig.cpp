#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <driver/timer.h>
#include <esp_wifi.h>
#include "rtc.h"
#include "nvdata.h"
#include "sensor.h"
#include "wificonfig.h"

static const char* TAG = "wificonfig";

extern const char* FirmwareRevision;

//#define STATION

const char* szAPSSID = "Esp32WaterTimer";
const char* szAPPassword = "123456789";

AsyncWebServer server(80);

static int SensorReading;
static float BatteryVoltage;
static float SuperCapVoltage;

static void server_not_found(AsyncWebServerRequest *request);
static String server_string_processor(const String& var);
static void server_handle_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
static void server_handle_OTA_update(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
static void server_feed_watchdog();

static void server_feed_watchdog() {
    TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed = 1;
    TIMERG0.wdt_wprotect = 0;
    }

static void server_not_found(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
  }

// Replace %xx% placeholder 
static String server_string_processor(const String& var){
  
  if(var == "FIRMWARE_REVISION"){
    return FirmwareRevision;
    }
  else
  // Google Sheet update 
  if(var == "GS_UPDATE"){
    return GSConfig.update ? "checked" : "";
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
    return String(GSConfig.utcOffsetSeconds);
    }
  else
  if(var == "DAYLIGHT_OFFSET"){
    return String(GSConfig.daylightOffsetSeconds);
    }
  else
  // internal history
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
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  delay(100);
  }


void wifi_access_point_init() {
  esp_wifi_start();
  delay(100);
#ifdef STATION  
  WiFi.mode(WIFI_STA);
  WiFi.begin(GoogleSheet.ssid.c_str(), GoogleSheet.password.c_str());
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
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

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
#endif
  server.onNotFound(server_not_found);
  server.onFileUpload(server_handle_upload);
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    rtc_get_clock(Clock);
    SensorReading = sensor_reading();
    BatteryVoltage = battery_voltage();
    SuperCapVoltage = supercap_voltage();
    request->send(SPIFFS, "/index.html", String(), false, server_string_processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    bool bScheduleChange = false;
    bool bRTCChange = false;
    bool bGSConfigChange = false;
    int gsupdate = 0;
    // Google Sheet update option
    if (request->hasParam("gsUpdate")) {
      // gsUpdate is added to GET request only if box is checked
      inputMessage = request->getParam("gsUpdate")->value();
      bGSConfigChange = true; 
      gsupdate = 1;
      }
    // Internet Access Credentials  
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
      GSConfig.utcOffsetSeconds = inputMessage.toInt();
      }
    if (request->hasParam("daylightOffset")) {
      inputMessage = request->getParam("daylightOffset")->value();
      bGSConfigChange = true; 
      GSConfig.daylightOffsetSeconds = inputMessage.toInt();
      }
      
    // Desired Schedule 
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
    if (request->hasParam("scheduleSensorThreshold")) {
      inputMessage = request->getParam("scheduleSensorThreshold")->value();
      bScheduleChange = true; 
      Schedule.sensorThreshold = (uint32_t)inputMessage.toInt();
      }
    if (request->hasParam("scheduleOnTimeSeconds")) {
      inputMessage = request->getParam("scheduleOnTimeSeconds")->value();
      bScheduleChange = true; 
      Schedule.onTimeSeconds = (uint32_t)inputMessage.toInt();
      }    

    // RTC Clock
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
      Serial.println("Google Sheet update parameters changed");
      GSConfig.update = gsupdate;
      Serial.printf("update %d\n", GSConfig.update);
      gs_config_store(GSConfig);
      bGSConfigChange = false;
      }
    if (bScheduleChange == true) {
      Serial.printf("Schedule changed, saving schedule and resetting RTC daily alarm\n");
      schedule_store(Schedule);
      RTC_ALARM alarm;
      alarm.hour = Schedule.hour;
      alarm.minute = Schedule.minute;
      rtc_set_daily_alarm(alarm);
      bScheduleChange = false;
      }
    request->send(200, "text/html", "Input Received<br><a href=\"/\">Return to Home Page</a>");  
  });

  server.begin();
  }


static void server_handle_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (filename.endsWith(".bin") ) {
    // .bin files uploaded are processed as application firmware updates
    server_handle_OTA_update(request, filename, index, data, len, final);
    }
  else {
    request->send(200, "text/plain", "Not a firmware binary file");
    }
  }
      

// handles OTA firmware update
static void server_handle_OTA_update(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    ESP_LOGD(TAG,"Client: %s %s",request->client()->remoteIP().toString().c_str(), request->url().c_str());
    if (!index) {
      ESP_LOGD(TAG,"OTA Update Start : %s", filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
        }
      }

    if (len) {
      server_feed_watchdog();
      // flashing firmware to ESP
      if (Update.write(data, len) != len) {
        Update.printError(Serial);
        }      
      ESP_LOGD(TAG,"Writing : %s index = %d len = %d", filename.c_str(), index, len);
      }

    if (final) {
      if (Update.end(true)) { //true to set the size to the current progress
        ESP_LOGD(TAG,"OTA Complete : %s, size = %d", filename.c_str(), index + len);
        } 
      else {
        Update.printError(Serial);
        }
      delay(1000);
      ESP.restart(); // force reboot after firmware update
      }
  }
  