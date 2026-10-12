# Electricity price schedule

Download day-ahead electricity prices from the ENTSO-E Transparency Platform over Wi-Fi, as a first step to charging with the charge controller (EVSE-C-3x32A) when power is cheapest.

## What it does

- Connects to Wi-Fi and gets the time from an NTP server (fixed offset of +1 hour, `NTPClient timeClient(ntpUDP, 3600L)`; no daylight saving time).
- Once at start-up, and then once a day after 13:00 (`GET_DATA_TIME`), it requests the day-ahead price document from `https://web-api.tp.entsoe.eu/api`:
  - document type `A44` (price document), contract type `A01` (daily),
  - bidding zone `10YNL----------L` (the Netherlands),
  - `periodStart` = tomorrow's date at 22:00 UTC, `periodEnd` = the day after at 22:00 UTC (see `entsoe_make_url()` in `ENTSOe.cpp`).
- If the request fails it retries an hour later.
- Parses the XML response with a small built-in parser (`VerySimpleXMLParser.cpp`) and stores up to 2 days (`PAGES`) of 96 quarter-hour prices (`ENTREES`) in `pricing[][]`, in EUR/MWh. Missing positions are stored as `-1`.
- Prints every price, then the highest and the lowest price with their day and quarter-hour index, on the serial monitor (115200 baud).

It does not control the charger yet. See [How it talks to the charge controller](#how-it-talks-to-the-charge-controller).

## What you need

- Raspberry Pi Pico 2 W (Wi-Fi).
- A free ENTSO-E security token (API key). How to get one: <https://transparencyplatform.zendesk.com/hc/en-us/articles/12845911031188-How-to-get-security-token>
- A 2.4 GHz Wi-Fi network.
- To control a charger as well: an RS485 interface and the cable described in [charger-control-basic](../charger-control-basic/) (3 × 0.75 mm² stranded multi-core, GND/A/B, short branches).

## Wiring

No wiring for this example: it only uses Wi-Fi and USB. To add charger control, wire an RS485 interface as in the [charger-control-basic Pico 2 W variant](../charger-control-basic/README.md#raspberry_pico2w_2ch_rs485) (UART1: TX = GP4, RX = GP5).

## Build and upload

- Arduino IDE 2.x (or `arduino-cli`).
- Board package: "Raspberry Pi Pico/RP2040/RP2350" (arduino-pico, by Earle Philhower), board "Raspberry Pi Pico 2W".
- Libraries (versions from the sketch header):
  - WiFi 1.0.0 and HTTPClient 1.2: included with the arduino-pico core
  - NTPClient 3.2.1
  - RTClib 2.1.4
- The sketch waits until the serial monitor is open (`while (!Serial)`), so open the serial monitor after uploading.

## Configure

**Secrets.** Copy `Pico_WiFi_GetPricing/Secrets.example.h` to `Pico_WiFi_GetPricing/Secrets.h` and fill it in:

```cpp
#define WIFI_SSID             "your-network"
#define WIFI_PASSWORD         "your-password"
#define ETSOE_SECURITY_TOKEN  "your-entsoe-token"
```

`Secrets.h` is listed in `.gitignore`, so your own values stay on your computer. **Never commit real values.** The template has unquoted placeholders, so the sketch does not compile until you have made `Secrets.h` with quoted strings as above.

**Other settings:**

| Setting | Where | Default | Notes |
|---|---|---|---|
| `DOMAIN` | `ENTSOe.h` | `10YNL----------L` | EIC code of your bidding zone. |
| `HTTP_ENDPOINT` | `ENTSOe.h` | `https://web-api.tp.entsoe.eu/api` | |
| `DOCUMENT_TYPE` | `ENTSOe.h` | `A44` | Price document. |
| `CONTRACT_MARKET_AGREEMENT_TYPE` | `ENTSOe.h` | `A01` | Daily. |
| `PAGES`, `ENTREES` | `ENTSOe.h` | 2, 96 | Days and 15-minute slots stored. |
| `GET_DATA_TIME` | `.ino` | 13 | Hour after which the next day's prices are fetched. |
| Time offset | `.ino`, `NTPClient(ntpUDP, 3600L)` | 3600 s | Change for your time zone or summer time. |

**TLS certificate.** The sketch does not verify the server's TLS certificate (`https.setInsecure()`, "do not check the certificate"). That is fine for a test, but anyone between you and the server could send fake prices. For real use, add the server's root certificate and remove `setInsecure()`.

## How it talks to the charge controller

It doesn't yet: this example only gets the prices. To turn it into a charge schedule:

1. Add the Modbus code from [charger-control-basic](../charger-control-basic/) (`ModBus.cpp/.h`, `SmartEVSE.cpp/.h`, `Board.h`).
2. Each quarter-hour, look up the price in `pricing[][]`.
3. In cheap slots write the wanted current with `smart_evse_set_max_current()` (register 0x0201); in expensive slots write 0 A.

See the [Modbus register map](../../docs/en/modbus-registers.md). Set the Modbus address of your charge controller (`ADDRESS`; boards ship set to 10, the firmware default is 1).
