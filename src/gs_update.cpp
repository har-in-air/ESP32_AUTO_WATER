#include <WiFi.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <esp_wifi.h>
#include "nv_data.h"
#include "gs_update.h"

static String SzMonth[12]= {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};


#if 0

#include <WiFiClientSecure.h>

WiFiClientSecure client;

static const char* SzGSHost = "script.google.com";
static const int HttpsPort = 443;

bool gs_update(GS_DATA_t &data) {
	Serial.printf("connecting to %s\n", SzGSHost);
	if (!client.connect(SzGSHost, HttpsPort)) {
		Serial.println("connection failed");
		return false;
		}
	String url = "/macros/s/" + GSConfig.gsID + "/exec?";
	url += "id=" + GSConfig.gsSheet;
	url += "&Date=" + SzMonth[data.month-1] + String(data.day);
	url += "&Time="  + String(data.hour) + ":" + String(data.minute);
	url += "&SensorReading=" + String(data.sensorReading);
	url += "&SensorThreshold=" + String(data.sensorThreshold);
	url += "&OnTimeSeconds=" + String(data.onTimeSeconds);
	url += "&BatteryVoltage=" + String(data.batteryVoltage, 1);
	url += "&SuperCapVoltage=" + String(data.superCapVoltage, 1);
	url += "&NTPminusRTC=" + (data.rtcError == RTC_ERROR_NOT_CALC ? "N.A." : String(data.rtcError));
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

#else

// 15/11/2021
// Using Firefox browser,  open your Google spreadsheet document page
// Click on padlock icon -> Connection Secure -> More Information->View Certificate
// -> GTS CA 1C3 -> Miscellaneous -> PEM (cert)-> open with text editor
// Copy, add arduino multi-line string comment quotation and newlines 
// (in VSC use shift+alt then drag with mouse for vertical selection)

const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFljCCA36gAwIBAgINAgO8U1lrNMcY9QFQZjANBgkqhkiG9w0BAQsFADBHMQsw\n" \
"CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n" \
"MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMjAwODEzMDAwMDQyWhcNMjcwOTMwMDAw\n" \
"MDQyWjBGMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n" \
"Y2VzIExMQzETMBEGA1UEAxMKR1RTIENBIDFDMzCCASIwDQYJKoZIhvcNAQEBBQAD\n" \
"ggEPADCCAQoCggEBAPWI3+dijB43+DdCkH9sh9D7ZYIl/ejLa6T/belaI+KZ9hzp\n" \
"kgOZE3wJCor6QtZeViSqejOEH9Hpabu5dOxXTGZok3c3VVP+ORBNtzS7XyV3NzsX\n" \
"lOo85Z3VvMO0Q+sup0fvsEQRY9i0QYXdQTBIkxu/t/bgRQIh4JZCF8/ZK2VWNAcm\n" \
"BA2o/X3KLu/qSHw3TT8An4Pf73WELnlXXPxXbhqW//yMmqaZviXZf5YsBvcRKgKA\n" \
"gOtjGDxQSYflispfGStZloEAoPtR28p3CwvJlk/vcEnHXG0g/Zm0tOLKLnf9LdwL\n" \
"tmsTDIwZKxeWmLnwi/agJ7u2441Rj72ux5uxiZ0CAwEAAaOCAYAwggF8MA4GA1Ud\n" \
"DwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwEgYDVR0T\n" \
"AQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQUinR/r4XN7pXNPZzQ4kYU83E1HScwHwYD\n" \
"VR0jBBgwFoAU5K8rJnEaK0gnhS9SZizv8IkTcT4waAYIKwYBBQUHAQEEXDBaMCYG\n" \
"CCsGAQUFBzABhhpodHRwOi8vb2NzcC5wa2kuZ29vZy9ndHNyMTAwBggrBgEFBQcw\n" \
"AoYkaHR0cDovL3BraS5nb29nL3JlcG8vY2VydHMvZ3RzcjEuZGVyMDQGA1UdHwQt\n" \
"MCswKaAnoCWGI2h0dHA6Ly9jcmwucGtpLmdvb2cvZ3RzcjEvZ3RzcjEuY3JsMFcG\n" \
"A1UdIARQME4wOAYKKwYBBAHWeQIFAzAqMCgGCCsGAQUFBwIBFhxodHRwczovL3Br\n" \
"aS5nb29nL3JlcG9zaXRvcnkvMAgGBmeBDAECATAIBgZngQwBAgIwDQYJKoZIhvcN\n" \
"AQELBQADggIBAIl9rCBcDDy+mqhXlRu0rvqrpXJxtDaV/d9AEQNMwkYUuxQkq/BQ\n" \
"cSLbrcRuf8/xam/IgxvYzolfh2yHuKkMo5uhYpSTld9brmYZCwKWnvy15xBpPnrL\n" \
"RklfRuFBsdeYTWU0AIAaP0+fbH9JAIFTQaSSIYKCGvGjRFsqUBITTcFTNvNCCK9U\n" \
"+o53UxtkOCcXCb1YyRt8OS1b887U7ZfbFAO/CVMkH8IMBHmYJvJh8VNS/UKMG2Yr\n" \
"PxWhu//2m+OBmgEGcYk1KCTd4b3rGS3hSMs9WYNRtHTGnXzGsYZbr8w0xNPM1IER\n" \
"lQCh9BIiAfq0g3GvjLeMcySsN1PCAJA/Ef5c7TaUEDu9Ka7ixzpiO2xj2YC/WXGs\n" \
"Yye5TBeg2vZzFb8q3o/zpWwygTMD0IZRcZk0upONXbVRWPeyk+gB9lm+cZv9TSjO\n" \
"z23HFtz30dZGm6fKa+l3D/2gthsjgx0QGtkJAITgRNOidSOzNIb2ILCkXhAd4FJG\n" \
"AJ2xDx8hcFH1mt0G/FX0Kw4zd8NLQsLxdxP8c4CU6x+7Nz/OAipmsHMdMqUybDKw\n" \
"juDEI/9bfU1lcKwrmz3O2+BtjjKAvpafkmO8l7tdufThcV4q5O8DIrGKZTqPwJNl\n" \
"1IXNDw9bg1kWRxYtnCQ6yICmJhSFm/Y3m6xv+cXDBlHz4n/FsRC6UfTd\n" \
"-----END CERTIFICATE-----"; 


bool gs_update(GS_DATA_t &data, String& control) {
	if (WiFi.status() != WL_CONNECTED) {
		Serial.println("Error : no connection to internet!");
		return false;
		}
	HTTPClient http;
	String url = "https://script.google.com/macros/s/" + GSConfig.gsID + "/exec?";
	url += "logID=" + GSConfig.gsLogSheet;
	url += "&controlID=" + GSConfig.gsControlSheet;
	url += "&Date=" + SzMonth[data.month-1] + String(data.day);
	url += "&Time="  + String(data.hour) + ":" + (data.minute < 10 ? "0" + String(data.minute) : String(data.minute));
	url += "&SensorReading=" + String(data.sensorReading);
	url += "&SensorThreshold=" + String(data.sensorThreshold);
	url += "&OnTimeSeconds=" + String(data.onTimeSeconds);
	url += "&BatteryVoltage=" + String(data.batteryVoltage, 1);
	url += "&SuperCapVoltage=" + String(data.superCapVoltage, 1);
	url += "&NTPminusRTC=" + (data.rtcError == RTC_ERROR_NOT_CALC ? "N.A." : String(data.rtcError));
    const char *location = "Location";	
	http.begin(url, root_ca); 
    const char *headerKeys[] = {location};
    http.collectHeaders(headerKeys, 1);
	int httpCode = http.GET();
	// expecting redirect code (302)
	if (httpCode == 302) { 
		String redirectUrl = http.header(location);
		http.end();
		Serial.print("redirectUrl = ");
		Serial.println(redirectUrl);

		http.begin(redirectUrl, root_ca);
		httpCode = http.GET();
		Serial.printf("\r\nStatus code = %d\n", httpCode);
		bool result = false;
		if (httpCode == 200) {
			control = http.getString();
			result = true;
			}
		else {
			Serial.println("Error on redirect");
			}
		http.end();
		return result;
		} 
	else {
		Serial.println("Error on HTTP request");
		http.end();
		return false;
		}
	}
    
#endif


bool gs_init() {
	esp_wifi_start();
	delay(100);
	WiFi.mode(WIFI_STA);
	WiFi.begin(GSConfig.wifiSSID.c_str(), GSConfig.wifiPassword.c_str());

	// note : if the Access Point is not running, the timeout happens quickly. 
	// This timeout appears to be for issues with connecting to a running AP,
	// e.g. wrong password.
	// In this situation, not only is the default timeout very long (60s),
	// the average current drawn nearly doubles. So we reduce the timeout to 8s.
	if (WiFi.waitForConnectResult(8000UL) != WL_CONNECTED) {
		Serial.println("Connection to Internet AP failed!");
		return false;
		}
	Serial.print("Connected, IP Address : ");
	Serial.println(WiFi.localIP());
#if 0
	client.setInsecure();
#endif
	if (!MDNS.begin("esp32")) { // Use http://esp32.local for web server page
		Serial.println("Error setting up MDNS responder!");
	    }
	else {
  		Serial.println("mDNS responder started");
		}
	return true;
	}


#if 0
// Google Apps Script for Google Docs Sheet bidirectional update
// ESP32 uploads the log data to the sheet tab "Autowater"
// ESP32 downloads schedule changes from the sheet tab "Control"
// Any schedule parameters not  marked 'x' will override the currently configured values 
// and be stored in Preferences flash partition. Schedule parameters  marked with 'x'
// will retain their currently configured values.

// Name and save the Google Doc e.g. "Data_Logger"
// First column in spreadsheet should have header "Timestamp", 
// rest as per requirements e.g. "SensorReading", "Threshold" etc
// Name the sheet (bottom tag), "Autowater"

// Create a second sheet with tag "Control"
// The first row has titles "Hour", "Minute", "Threshold", "OnSecs"
// The second row has the control values for these fields, e.g. 12, 0, 425, 25

// Use Menu Extensions->Apps Script and create or edit the file "Code.gs" for the "doGet()" function
// and insert this function
// Use Deploy -> New Deployment after you create or make a change to this script.
// You will get the updated spreadsheet id key, make a copy for your upload code.

// The url to use in the upload code is as per the following example :
// https://script.google.com/macros/s/insert_spreadsheet_id_key/exec?logID=AutoWater&controlID=Control&Date=Nov15&Time=20:49&SensorReading=356&SensorThreshold=360&OnTimeSeconds=0&BatteryVoltage=3.8&SuperCapVoltage=17.9

// https://stackoverflow.com/questions/69685813/problem-esp32-send-data-to-google-sheet-through-google-app-script

function doGet(e) {
  try {
    var logSheet = SpreadsheetApp.getActive().getSheetByName(e.parameter["logID"]);
    var controlSheet = SpreadsheetApp.getActive().getSheetByName(e.parameter["controlID"]);
    var headers = logSheet.getRange(1, 1, 1, logSheet.getLastColumn()).getValues()[0];
    var d = new Date();
    var values = headers.map(h => h == "Timestamp" ? d.toDateString() + ", " + d.toLocaleTimeString() : e.parameter[h]);
    logSheet.getRange(logSheet.getLastRow() + 1, 1, 1, values.length).setValues([values]);

    var ctrlHour = controlSheet.getRange(2,1).getValue();
    var ctrlMinute = controlSheet.getRange(2,2).getValue();
    var ctrlThreshold = controlSheet.getRange(2,3).getValue();
    var ctrlOnSeconds = controlSheet.getRange(2,4).getValue();
    retSz = "ok" + "," + ctrlHour + "," + ctrlMinute + "," + ctrlThreshold + "," + ctrlOnSeconds;
    return ContentService.createTextOutput(retSz);
  } 
  catch (e) {
    return ContentService.createTextOutput(JSON.stringify(e));
    } 
}

#endif