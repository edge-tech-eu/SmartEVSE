#ifndef ENTSOE_H
#define ENTSOE_H

#include <time.h>

// PT15M data for 2 local days (today and tomorrow, Europe/Amsterdam).
// A local day has 96 quarter-hours, 92 on the day summer time starts and 100 on the day it ends.
#define PAGES 2
#define ENTREES 100
// production server
#define HTTP_ENDPOINT "https://web-api.tp.entsoe.eu/api"
// EIC code of a Bidding Zone, the netherlands
#define ENTSOE_DOMAIN "10YNL----------L"
// A44 - Price Document The document is used to provide market spot price information.
#define DOCUMENT_TYPE "A44"
// A01 - Daily
#define CONTRACT_MARKET_AGREEMENT_TYPE "A01"

// local time zone: Europe/Amsterdam, CET (UTC+1) with summer time CEST (UTC+2)
#define LOCAL_TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"

// root certificate of the ENTSO-E API server, used to check the TLS connection
extern const char entsoe_root_ca[];

// pricing[page][index]: page 0 = today, page 1 = tomorrow (local days),
// index = quarter-hour since local midnight, EUR/MWh. Prices can be negative.
// Slots without a price are NAN (check with isnan()).
extern double pricing[PAGES][ENTREES];
// UTC epoch of local midnight at the start of each page
extern time_t page_start[PAGES];

void entsoe_make_url(char *url, size_t size, time_t now);
void entsoe_parse_xml(const char *xml);
void entsoe_simple_stats(void);

#endif
