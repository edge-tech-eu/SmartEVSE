#include <Arduino.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <RTClib.h>
#include "Secrets.h"
#include "VerySimpleXMLParser.h"
#include "ENTSOe.h"

WiFiMulti WiFiMulti;
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, 3600L);

int last_day=0;

// new data on entso-e server at noon
#define GET_DATA_TIME   13
int get_time;

void setup() {

  // wait for serial

  Serial.begin(115200);
  while (!Serial) {
  }
  delay(1000);

  Serial.printf("\r\n\r\n\r\nGetting ENTSO-e pricing with Pico\r\n\r\n");


  // wifi

  Serial.printf("WiFi connecting...\r\n");
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  Serial.printf("WiFi connected!\r\n\r\n");


  // network time protocol

  Serial.printf("NTP starting...\r\n");
  timeClient.begin();
  while (!timeClient.isTimeSet()) {
    timeClient.update();
  }
  Serial.printf("NTP started (it is %s)\r\n\r\n", timeClient.getFormattedTime().c_str());

  last_day = 0;
  get_time = GET_DATA_TIME;
}


void loop() {

  if ((WiFiMulti.run() == WL_CONNECTED)) {
    
    while (!timeClient.isTimeSet()) {
      Serial.printf("Time is updating...\r\n");
      timeClient.update();
      Serial.printf("Time is updated (it is %s)\r\n\r\n", timeClient.getFormattedTime().c_str());
    }

    if((last_day == 0) || ((timeClient.getDay() != last_day) && (timeClient.getHours() > get_time))) {

      // just in case this fails, we retry in an hour
      get_time ++;

      HTTPClient https;
      // do not check the certificate
      https.setInsecure();

      char url[256];
      entsoe_make_url(url, timeClient.getEpochTime());
      Serial.printf("[HTTPS] getting %s\r\n", url);

      if (https.begin(url)) {

        Serial.printf("[HTTPS] GET...\r\n");

        int httpCode = https.GET();

        Serial.printf("[HTTPS] got data (code: %d)\r\n\r\n", httpCode);

        if (httpCode > 0) {

          if ((httpCode == HTTP_CODE_OK) || (httpCode == HTTP_CODE_MOVED_PERMANENTLY)) {

            entsoe_parse_xml(https.getString().c_str());

            entsoe_simple_stats();

            last_day = timeClient.getDay();
            get_time = GET_DATA_TIME;
          }

        } else {

          Serial.printf("[HTTPS] GET... failed, error: %s\r\n", https.errorToString(httpCode).c_str());
        }

        https.end();

      } else {

        Serial.printf("[HTTPS] Unable to connect\r\n");
      }
    }
  }

  // other tasks
}