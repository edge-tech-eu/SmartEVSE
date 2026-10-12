/* 
    This is example code to show how to get the day-ahead electricity prices from ENTSO-e
    Use this code to make your own charge controller and charge your car at lowest cost

    You need a security key from ENTSO-e, see ENTSOe.cpp

    Board used:
    - Raspberrry Pi Pico 2W - from Raspberry Pi Pico/RP2040/RP2350

    Libraries used (all included with the RP2040/RP2350 board package):
    - WiFi v1.0.0      - Wi-Fi and the NTP client (NTP)
    - HTTPClient v1.2  - HTTPS requests

    Time: NTP gives UTC; the local time zone is Europe/Amsterdam with summer time
    (LOCAL_TIMEZONE in ENTSOe.h).

    TLS: the server certificate is checked against the root certificate in ENTSOe.cpp.

    Contact us if you help controlling our evse: https://edgetech.eu/

    Edge Technologies
*/
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <math.h>
#include "Secrets.h"
#include "VerySimpleXMLParser.h"
#include "ENTSOe.h"

WiFiMulti WiFiMulti;

// new data on entso-e server around noon (local time); fetch tomorrow's prices after this hour
#define GET_DATA_TIME   13

// local day (day of the year) of the last successful download, -1 = none yet
int last_yday = -1;
// did the last download have prices for tomorrow?
bool have_tomorrow = false;
// earliest time of the next attempt (retry after a minute at start-up, else after an hour)
time_t next_attempt = 0;

bool tomorrow_has_prices() {

  for (int index = 0; index < ENTREES; index++) {
    if (!isnan(pricing[1][index])) {
      return (true);
    }
  }
  return (false);
}

void wait_for_time() {

  Serial.printf("NTP starting...\r\n");

  NTP.begin("pool.ntp.org", "time.nist.gov");
  while (!NTP.waitSet()) {
    Serial.printf("still waiting for NTP...\r\n");
  }

  time_t now = time(nullptr);
  struct tm local;
  localtime_r(&now, &local);

  Serial.printf("NTP started (it is %02d:%02d:%02d %s)\r\n\r\n",
                local.tm_hour, local.tm_min, local.tm_sec, local.tm_isdst > 0 ? "CEST" : "CET");
}

void setup() {

  // wait for serial

  Serial.begin(115200);
  while (!Serial) {
  }
  delay(1000);

  Serial.printf("\r\n\r\n\r\nGetting ENTSO-e pricing with Pico\r\n\r\n");


  // time zone: Europe/Amsterdam, including summer time

  setenv("TZ", LOCAL_TIMEZONE, 1);
  tzset();


  // wifi

  Serial.printf("WiFi connecting...\r\n");
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  Serial.printf("WiFi connected!\r\n\r\n");


  // network time protocol (also needed to check the TLS certificate)

  wait_for_time();
}


void loop() {

  if ((WiFiMulti.run() == WL_CONNECTED)) {

    time_t now = time(nullptr);
    struct tm local;
    localtime_r(&now, &local);

    // at start-up, and after GET_DATA_TIME (local time) until tomorrow's prices are in
    bool wanted = (last_yday < 0) ||
                  ((local.tm_hour >= GET_DATA_TIME) && ((local.tm_yday != last_yday) || !have_tomorrow));

    if (wanted && (now >= next_attempt)) {

      // just in case this fails or tomorrow is not published yet: retry later
      next_attempt = now + ((last_yday < 0) ? 60 : 3600);

      HTTPClient https;

      // check the server certificate against the embedded root certificate
      https.setCACert(entsoe_root_ca);

      char url[320];
      entsoe_make_url(url, sizeof(url), now);
      Serial.printf("[HTTPS] getting %s\r\n", url);

      if (https.begin(url)) {

        Serial.printf("[HTTPS] GET...\r\n");

        int httpCode = https.GET();

        Serial.printf("[HTTPS] got data (code: %d)\r\n\r\n", httpCode);

        if (httpCode > 0) {

          if ((httpCode == HTTP_CODE_OK) || (httpCode == HTTP_CODE_MOVED_PERMANENTLY)) {

            entsoe_parse_xml(https.getString().c_str());

            entsoe_simple_stats();

            last_yday = local.tm_yday;
            have_tomorrow = tomorrow_has_prices();
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
