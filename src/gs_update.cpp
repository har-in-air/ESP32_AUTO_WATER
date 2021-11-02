#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_wifi.h>
#include "nvdata.h"
#include "gs_update.h"

static const char* SzGSHost = "script.google.com";
static const int HttpsPort = 443;

static String GS_ID = "your google sheets id goes here";
static String GS_Sheet = "AutoWater";

WiFiClientSecure client;

GS_DATA GSData;

bool gs_init() {
  esp_wifi_start();
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(GSConfig.wifiSSID.c_str(), GSConfig.wifiPassword.c_str());
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return false;
    }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  client.setInsecure();
  return true;
  }


bool gs_update(GS_DATA &data) {
  Serial.printf("connecting to %s\n", SzGSHost);
  if (!client.connect(SzGSHost, HttpsPort)) {
    Serial.println("connection failed");
    return false;
    }
  // Post Data
  String url;
  url += "/macros/s/" + GS_ID + "/exec?";
  url += "id=" + String(GS_Sheet);
  url += "&SensorReading=" + String(data.sensorReading);
  url += "&SensorThreshold=" + String(data.sensorThreshold);
  url += "&OnTimeSeconds=" + String(data.onTimeSeconds);
  url += "&BatteryVoltage=" + String(data.batteryVoltage, 1);
  url += "&SuperCapVoltage=" + String(data.superCapVoltage, 1);
  Serial.print("requesting URL: "); Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + SzGSHost + "\r\n" +
    "User-Agent: ESP32\r\n" +
    "Connection: close\r\n\r\n");
  Serial.println("request sent");
  // Wait Echo
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
      }
    }
  String line = client.readStringUntil('\n');
  Serial.print("reply was : ");
  Serial.println(line);
  return true;
  }
