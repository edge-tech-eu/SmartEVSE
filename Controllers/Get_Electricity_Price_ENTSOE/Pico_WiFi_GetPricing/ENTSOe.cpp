#include <Arduino.h>
#include <RTClib.h>
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



double pricing[PAGES][ENTREES];

void entsoe_make_url(char *url, unsigned long epochtime) {

  // new data available at noon CET
  // request parameters in UTC
  DateTime start = epochtime - 3600L + 86400UL;
  DateTime stop = epochtime - 3600L + 2 * 86400UL;

  sprintf(url,
          "%s?documentType=%s&contract_MarketAgreement.type=%s&periodStart=%4d%02d%02d2200&periodEnd=%4d%02d%02d2200&out_Domain=%s&in_Domain=%s&securityToken=%s",
          HTTP_ENDPOINT, DOCUMENT_TYPE, CONTRACT_MARKET_AGREEMENT_TYPE,
          start.year(), start.month(), start.day(), stop.year(), stop.month(), stop.day(),
          DOMAIN, DOMAIN, ETSOE_SECURITY_TOKEN);
}

void entsoe_parse_xml(const char *xml) {

  xml_buffer(xml);

  char tag_text[MAX_TAG_TEXT];
  const char *tags[] = { "TimeSeries", "Point" };
  int tag_index;

  int page = 0;

  xml_goto_tag("TimeSeries");

  do {

    xml_goto_tag("start");
    xml_tag_text(tag_text);

    Serial.printf("start: %s\r\n", tag_text);

    xml_goto_tag("end");
    xml_tag_text(tag_text);

    Serial.printf("end:  %s\r\n", tag_text);

    xml_goto_tag("resolution");
    xml_tag_text(tag_text);

    Serial.printf("resolution: %s\r\n", tag_text);

    if (page >= PAGES) {

      Serial.printf("got more pages than expected, skipping data\r\n");

    } else {

      int last_position = 0;
      do {
        tag_index = xml_goto_one_of_tags(tags, 2);
        if (tag_index < 0) {

          // Serial.printf("tag not found\r\n");

        } else if (tag_index == 1) {

          xml_goto_tag("position");
          int position = xml_tag_int();

          if ((position < 0) || (position > ENTREES)) {

            Serial.printf("%d is not a valid position\r\n", position);

          } else {

            last_position++;

            while (last_position != position) {
              pricing[page][last_position - 1] = -1;
              Serial.printf("pricing[%d][%d] = unknown (-1)\r\n", page, last_position - 1);
              last_position++;
            }

            xml_goto_tag("price.amount");
            double price = xml_tag_double();
            pricing[page][position - 1] = price;
            Serial.printf("pricing[%d][%d] = %f\r\n", page, position - 1, pricing[page][position - 1]);
          }
        }
      } while (tag_index == 1);
    }

    page++;

  } while (tag_index == 0);

  Serial.printf("Processed xml\r\n");
}

void entsoe_simple_stats() {

  double highest = pricing[0][0];
  double lowest = pricing[0][0];
  int page_highest = 0;
  int index_highest = 0;
  int page_lowest = 0;
  int index_lowest = 0;

  for (int i = 0; i < PAGES; i++) {
    for (int j = 0; j < ENTREES; j++) {
      if (pricing[i][j] >= 0.0) {
        if (pricing[i][j] > highest) {
          highest = pricing[i][j];
          page_highest = i;
          index_highest = j;
        } else if (pricing[i][j] < lowest) {
          lowest = pricing[i][j];
          page_lowest = i;
          index_lowest = j;
        }
      }
    }
  }

  Serial.printf("Highest price found at page %d and index %d: %f euro/MWH\r\n", page_highest, index_highest, highest);
  Serial.printf("Lowest price found at page %d and index %d: %f euro/MWH\r\n", page_lowest, index_lowest, lowest);
}