#include <Arduino.h>
#include <math.h>
#include <time.h>
#include "Secrets.h"
#include "ENTSOe.h"
#include "VerySimpleXMLParser.h"


// explanation:
// https://documenter.getpostman.com/view/7009892/2s93JtP3F6

// how to get a security token:
// https://transparencyplatform.zendesk.com/hc/en-us/articles/12845911031188-How-to-get-security-token

// document type
// https://eepublicdownloads.entsoe.eu/clean-documents/EDI/Library/Core/entso-e-code-list-v36r0.pdf

// domain
// https://eepublicdownloads.entsoe.eu/clean-documents/nc-tasks/Day-ahead_Annex3-MRC%20cost%20coefficients_NEMOS_update.pdf


// Root certificate (self-signed root of the chain served by web-api.tp.entsoe.eu, valid until
// March 2046). If the server ever moves to another certificate authority, the TLS check fails
// with an error: replace this certificate with the new root. See the README.
const char entsoe_root_ca[] =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIFijCCA3KgAwIBAgIQdY39i658BwD6qSWn4cetFDANBgkqhkiG9w0BAQwFADBf\n"
  "MQswCQYDVQQGEwJHQjEYMBYGA1UEChMPU2VjdGlnbyBMaW1pdGVkMTYwNAYDVQQD\n"
  "Ey1TZWN0aWdvIFB1YmxpYyBTZXJ2ZXIgQXV0aGVudGljYXRpb24gUm9vdCBSNDYw\n"
  "HhcNMjEwMzIyMDAwMDAwWhcNNDYwMzIxMjM1OTU5WjBfMQswCQYDVQQGEwJHQjEY\n"
  "MBYGA1UEChMPU2VjdGlnbyBMaW1pdGVkMTYwNAYDVQQDEy1TZWN0aWdvIFB1Ymxp\n"
  "YyBTZXJ2ZXIgQXV0aGVudGljYXRpb24gUm9vdCBSNDYwggIiMA0GCSqGSIb3DQEB\n"
  "AQUAA4ICDwAwggIKAoICAQCTvtU2UnXYASOgHEdCSe5jtrch/cSV1UgrJnwUUxDa\n"
  "ef0rty2k1Cz66jLdScK5vQ9IPXtamFSvnl0xdE8H/FAh3aTPaE8bEmNtJZlMKpnz\n"
  "SDBh+oF8HqcIStw+KxwfGExxqjWMrfhu6DtK2eWUAtaJhBOqbchPM8xQljeSM9xf\n"
  "iOefVNlI8JhD1mb9nxc4Q8UBUQvX4yMPFF1bFOdLvt30yNoDN9HWOaEhUTCDsG3X\n"
  "ME6WW5HwcCSrv0WBZEMNvSE6Lzzpng3LILVCJ8zab5vuZDCQOc2TZYEhMbUjUDM3\n"
  "IuM47fgxMMxF/mL50V0yeUKH32rMVhlATc6qu/m1dkmU8Sf4kaWD5QazYw6A3OAS\n"
  "VYCmO2a0OYctyPDQ0RTp5A1NDvZdV3LFOxxHVp3i1fuBYYzMTYCQNFu31xR13NgE\n"
  "SJ/AwSiItOkcyqex8Va3e0lMWeUgFaiEAin6OJRpmkkGj80feRQXEgyDet4fsZfu\n"
  "+Zd4KKTIRJLpfSYFplhym3kT2BFfrsU4YjRosoYwjviQYZ4ybPUHNs2iTG7sijbt\n"
  "8uaZFURww3y8nDnAtOFr94MlI1fZEoDlSfB1D++N6xybVCi0ITz8fAr/73trdf+L\n"
  "HaAZBav6+CuBQug4urv7qv094PPK306Xlynt8xhW6aWWrL3DkJiy4Pmi1KZHQ3xt\n"
  "zwIDAQABo0IwQDAdBgNVHQ4EFgQUVnNYZJX5khqwEioEYnmhQBWIIUkwDgYDVR0P\n"
  "AQH/BAQDAgGGMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAC9c\n"
  "mTz8Bl6MlC5w6tIyMY208FHVvArzZJ8HXtXBc2hkeqK5Duj5XYUtqDdFqij0lgVQ\n"
  "YKlJfp/imTYpE0RHap1VIDzYm/EDMrraQKFz6oOht0SmDpkBm+S8f74TlH7Kph52\n"
  "gDY9hAaLMyZlbcp+nv4fjFg4exqDsQ+8FxG75gbMY/qB8oFM2gsQa6H61SilzwZA\n"
  "Fv97fRheORKkU55+MkIQpiGRqRxOF3yEvJ+M0ejf5lG5Nkc/kLnHvALcWxxPDkjB\n"
  "JYOcCj+esQMzEhonrPcibCTRAUH4WAP+JWgiH5paPHxsnnVI84HxZmduTILA7rpX\n"
  "DhjvLpr3Etiga+kFpaHpaPi8TD8SHkXoUsCjvxInebnMMTzD9joiFgOgyY9mpFui\n"
  "TdaBJQbpdqQACj7LzTWb4OE4y2BThihCQRxEV+ioratF4yUQvNs+ZUH7G6aXD+u5\n"
  "dHn5HrwdVw1Hr8Mvn4dGp+smWg9WY7ViYG4A++MnESLn/pmPNPW56MORcr3Ywx65\n"
  "LvKRRFHQV80MNNVIIb/bE/FmJUNS0nAiNs2fxBx1IK1jcmMGDw4nztJqDby1ORrp\n"
  "0XZ60Vzk50lJLVU3aPAaOpg+VBeHVOmmJ1CJeyAvP/+/oYtKR5j/K3tJPsMpRmAY\n"
  "QqszKbrAKbkTidOIijlBO8n9pu0f9GBj39ItVQGL\n"
  "-----END CERTIFICATE-----\n"
  ;


double pricing[PAGES][ENTREES];
time_t page_start[PAGES];
static time_t request_end;

#define QUARTER_SECONDS 900

// days since 1970-01-01 for a date in the proleptic Gregorian calendar (UTC)
static long days_from_civil(int y, int m, int d) {

  y -= (m <= 2);
  long era = (y >= 0 ? y : y - 399) / 400;
  long yoe = y - era * 400;
  long doy = (153L * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097L + doe - 719468L;
}

// "2026-10-11T22:00Z" (UTC) to epoch; returns -1 if the text is not a time
static time_t parse_utc(const char *text) {

  int y, mo, d, h, mi;

  if (sscanf(text, "%d-%d-%dT%d:%d", &y, &mo, &d, &h, &mi) != 5) {
    return (time_t)-1;
  }

  return (time_t)(days_from_civil(y, mo, d) * 86400L + h * 3600L + mi * 60L);
}

// "PT15M" / "PT60M" to minutes; returns 0 if unknown
static int parse_resolution(const char *text) {

  int minutes;

  if (sscanf(text, "PT%dM", &minutes) == 1) {
    return minutes;
  }

  return 0;
}

// store one price for the quarter-hour that starts at slot (UTC epoch)
static void store_price(time_t slot, double price) {

  for (int page = PAGES - 1; page >= 0; page--) {

    if (slot >= page_start[page]) {

      time_t page_end = (page + 1 < PAGES) ? page_start[page + 1] : request_end;

      if (slot < page_end) {

        long index = (long)((slot - page_start[page]) / QUARTER_SECONDS);

        if ((index >= 0) && (index < ENTREES)) {
          pricing[page][index] = price;
        }
      }
      return;
    }
  }
}

// store a price for one position of a period (covers resolution/15 quarter-hours)
static void store_position(time_t period_start, int resolution, int position, double price) {

  time_t slot = period_start + (time_t)(position - 1) * resolution * 60;

  for (int q = 0; q < resolution / 15; q++) {
    store_price(slot + (time_t)q * QUARTER_SECONDS, price);
  }
}

// Request today and tomorrow (local days, Europe/Amsterdam). The API wants the period in UTC:
// local midnight is 23:00 UTC the day before in winter and 22:00 UTC in summer.
// Needs the TZ environment variable set to LOCAL_TIMEZONE (see the .ino).
void entsoe_make_url(char *url, size_t size, time_t now) {

  struct tm local;
  localtime_r(&now, &local);

  for (int page = 0; page <= PAGES; page++) {

    struct tm midnight = local;
    midnight.tm_hour = 0;
    midnight.tm_min = 0;
    midnight.tm_sec = 0;
    midnight.tm_mday += page;
    midnight.tm_isdst = -1;   // let mktime work out summer time

    time_t t = mktime(&midnight);

    if (page < PAGES) {
      page_start[page] = t;
    } else {
      request_end = t;
    }
  }

  // forget the previous prices
  for (int page = 0; page < PAGES; page++) {
    for (int index = 0; index < ENTREES; index++) {
      pricing[page][index] = NAN;
    }
  }

  struct tm start, stop;
  gmtime_r(&page_start[0], &start);
  gmtime_r(&request_end, &stop);

  snprintf(url, size,
           "%s?documentType=%s&contract_MarketAgreement.type=%s&periodStart=%04d%02d%02d%02d%02d&periodEnd=%04d%02d%02d%02d%02d&out_Domain=%s&in_Domain=%s&securityToken=%s",
           HTTP_ENDPOINT, DOCUMENT_TYPE, CONTRACT_MARKET_AGREEMENT_TYPE,
           start.tm_year + 1900, start.tm_mon + 1, start.tm_mday, start.tm_hour, start.tm_min,
           stop.tm_year + 1900, stop.tm_mon + 1, stop.tm_mday, stop.tm_hour, stop.tm_min,
           ENTSOE_DOMAIN, ENTSOE_DOMAIN, ETSOE_SECURITY_TOKEN);
}

void entsoe_parse_xml(const char *xml) {

  xml_buffer(xml);

  char tag_text[MAX_TAG_TEXT];
  const char *tags[] = { "TimeSeries", "Point" };
  int tag_index;

  if (xml_goto_tag("TimeSeries") < 0) {
    Serial.printf("no prices in the reply\r\n");
    return;
  }

  do {

    xml_goto_tag("start");
    xml_tag_text(tag_text, sizeof(tag_text));
    time_t period_start = parse_utc(tag_text);

    Serial.printf("start: %s\r\n", tag_text);

    xml_goto_tag("end");
    xml_tag_text(tag_text, sizeof(tag_text));
    time_t period_end = parse_utc(tag_text);

    Serial.printf("end:  %s\r\n", tag_text);

    xml_goto_tag("resolution");
    xml_tag_text(tag_text, sizeof(tag_text));
    int resolution = parse_resolution(tag_text);

    Serial.printf("resolution: %s\r\n", tag_text);

    bool valid = (period_start != (time_t)-1) && (period_end > period_start) &&
                 (resolution >= 15) && (resolution % 15 == 0);

    if (!valid) {
      Serial.printf("period not understood, skipping data\r\n");
    }

    int positions = valid ? (int)((period_end - period_start) / (resolution * 60)) : 0;
    int last_position = 0;
    double last_price = NAN;

    do {
      tag_index = xml_goto_one_of_tags(tags, 2);

      if ((tag_index == 1) && valid) {

        xml_goto_tag("position");
        int position = xml_tag_int();

        xml_goto_tag("price.amount");
        double price = xml_tag_double();   // EUR/MWh, can be negative

        if ((position <= last_position) || (position > positions)) {

          Serial.printf("%d is not a valid position\r\n", position);

        } else {

          // curve type A03: a position that is left out has the same price as the one before
          while (++last_position < position) {
            store_position(period_start, resolution, last_position, last_price);
          }

          store_position(period_start, resolution, position, price);
          last_price = price;
        }
      }
    } while (tag_index == 1);

    // positions left out at the end of the period also repeat the last price
    if (valid) {
      while (++last_position <= positions) {
        store_position(period_start, resolution, last_position, last_price);
      }
    }

  } while (tag_index == 0);

  for (int page = 0; page < PAGES; page++) {
    for (int index = 0; index < ENTREES; index++) {
      if (!isnan(pricing[page][index])) {
        Serial.printf("pricing[%d][%d] = %f\r\n", page, index, pricing[page][index]);
      }
    }
  }

  Serial.printf("Processed xml\r\n");
}

void entsoe_simple_stats() {

  bool found = false;
  double highest = 0;
  double lowest = 0;
  int page_highest = 0;
  int index_highest = 0;
  int page_lowest = 0;
  int index_lowest = 0;

  for (int i = 0; i < PAGES; i++) {
    for (int j = 0; j < ENTREES; j++) {

      double price = pricing[i][j];

      // NAN = no price; negative prices are valid
      if (isnan(price)) {
        continue;
      }

      if (!found || (price > highest)) {
        highest = price;
        page_highest = i;
        index_highest = j;
      }

      if (!found || (price < lowest)) {
        lowest = price;
        page_lowest = i;
        index_lowest = j;
      }

      found = true;
    }
  }

  if (!found) {
    Serial.printf("No prices found\r\n");
    return;
  }

  struct tm t;
  time_t slot;

  slot = page_start[page_highest] + (time_t)index_highest * QUARTER_SECONDS;
  localtime_r(&slot, &t);
  Serial.printf("Highest price found at page %d and index %d (%02d-%02d %02d:%02d): %f euro/MWh\r\n",
                page_highest, index_highest, t.tm_mday, t.tm_mon + 1, t.tm_hour, t.tm_min, highest);

  slot = page_start[page_lowest] + (time_t)index_lowest * QUARTER_SECONDS;
  localtime_r(&slot, &t);
  Serial.printf("Lowest price found at page %d and index %d (%02d-%02d %02d:%02d): %f euro/MWh\r\n",
                page_lowest, index_lowest, t.tm_mday, t.tm_mon + 1, t.tm_hour, t.tm_min, lowest);
}
