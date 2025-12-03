#ifndef ENTSOE_H
#define ENTSOE_H

// PT15M data (24/60/15 = 96) for 2 days
#define PAGES 2
#define ENTREES 96
// production server
#define HTTP_ENDPOINT "https://web-api.tp.entsoe.eu/api"
// EIC code of a Bidding Zone, the netherlands
#define DOMAIN "10YNL----------L"
// A44 - Price Document The document is used to provide market spot price information.
#define DOCUMENT_TYPE "A44"
// A01 - Daily
#define CONTRACT_MARKET_AGREEMENT_TYPE "A01"

void entsoe_make_url(char *url, unsigned long epochtime);
void entsoe_parse_xml(const char *xml);
void entsoe_simple_stats(void);

#endif