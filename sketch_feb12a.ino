#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "sntp.h"
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>

const int maxRetries = 5;

#define RELAY_PIN_ON 14
#define RELAY_PIN_OFF 18

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define CUSTOM_CHIP_ID "Indunil's_ESP32"

#define API_KEY "AIzaSyCv33AAZH3xIDzfT_e6KBI-Iw3KlzRDoDI"
#define DATABASE_URL "https://rice-cooker-datalogger-2024-default-rtdb.asia-southeast1.firebasedatabase.app/"

const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const int RelayOnTime = 10000;
const int RelayOffTime = 10000;
int relayState = LOW;
unsigned long relayPreviousMillis = 0;

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 3600;

const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";

struct DateTime {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
};

DateTime dateTime;

DateTime getTimeInfo() {
  struct tm timeinfo;
  DateTime dateTime;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return dateTime;
  }

  dateTime.year = timeinfo.tm_year + 1900;
  dateTime.month = timeinfo.tm_mon + 1;
  dateTime.day = timeinfo.tm_mday;
  dateTime.hour = timeinfo.tm_hour;
  dateTime.minute = timeinfo.tm_min;
  dateTime.second = timeinfo.tm_sec;

  return dateTime;
}

String createFirebasePath(DateTime dateTime) {
  char timestampString[20];
  snprintf(timestampString, sizeof(timestampString), "%04d:%02d:%02d %02d:%02d:%02d",
           dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second);

  return "/energy_meter/LED_blinking_data/" + String(CUSTOM_CHIP_ID) + "/date/" +
         String(dateTime.year) + String(dateTime.month) + String(dateTime.day) +
         "/timestamp/" + String(timestampString);
}

void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
}

void printTimeDate(const DateTime& dateTime) {
  int hour = dateTime.hour;
  int minute = dateTime.minute;
  int second = dateTime.second;
  int year = dateTime.year;
  int month = dateTime.month;
  int day = dateTime.day;

  Serial.print("    Time: ");
  Serial.printf("%02d:%02d:%02d", hour, minute, second);
  Serial.print("    Date: ");
  Serial.printf("%04d-%02d-%02d", year, month, day);
}

void setup() {
  Serial.begin(115200);
  sensors.begin();
  pinMode(RELAY_PIN_ON, OUTPUT);
  pinMode(RELAY_PIN_OFF, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to WiFi Network");
  while (WiFi.status() != WL_CONNECTED) {
    for (int i = 0; i < 2; i = i + 1) {
      Serial.print("----Trying To Connect WiFi Network----");
      delay(500);
    }
    Serial.println();
  }

  Serial.println();
  Serial.println("Successfully Connected to WiFi Network");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign Up OK");
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  sntp_set_time_sync_notification_cb(timeavailable);
  sntp_servermode_dhcp(1);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
}

void controlRelayByTime() {
  unsigned long relayCurrentMillis = millis();

  if (relayState == LOW && relayCurrentMillis - relayPreviousMillis >= RelayOffTime) {
    relayState = HIGH;
    digitalWrite(RELAY_PIN_ON, relayState);
    relayPreviousMillis = relayCurrentMillis;
  } else if (relayState == HIGH && relayCurrentMillis - relayPreviousMillis >= RelayOnTime) {
    relayState = LOW;
    digitalWrite(RELAY_PIN_ON, relayState);
    relayPreviousMillis = relayCurrentMillis;
  }
}

void loop() {
  DateTime dateTime = getTimeInfo();
  String basePath = createFirebasePath(dateTime);

  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  String temperatureString = String(temperatureC) + "ºC";

  controlRelayByTime();

  bool uploadSuccess = false;
  for (int retryCount = 0; retryCount < maxRetries; ++retryCount) {
    if (Firebase.ready()) {
      if (Firebase.RTDB.setString(&fbdo, basePath + "/temperature", temperatureString) &&
          Firebase.RTDB.setInt(&fbdo, basePath + "/relayState", relayState)) {
        uploadSuccess = true;
        break;
      } else {
        Serial.println("Retry failed - Retrying...");
        delay(100);
      }
    } else {
      Serial.println("Firebase not ready - Retrying...");
      delay(500);
    }
  }

  if (uploadSuccess) {
    Serial.println("");
    Serial.print("Temperature: ");
    Serial.print(temperatureString);
    Serial.print("    ");
    Serial.print("Relay State: ");
    Serial.print(relayState);
    printTimeDate(dateTime);
    Serial.println(" Data Successfully Saved to Firebase RTDB");
  } else {
    Serial.println("Max retries reached - Upload failed.");
  }

  delay(1000);  // Upload temperature data every 1 second
}
