# Electricity price schedule

Download day-ahead electricity prices from the ENTSO-E Transparency Platform over Wi-Fi, as a first step to charging with the charge controller (EVSE-C-3x32A) when power is cheapest.

## What it does

- Connects to Wi-Fi and gets the time from an NTP server (the `NTP` client of the arduino-pico core). The clock runs in UTC; local time is Europe/Amsterdam with summer time, set with the POSIX time zone string `CET-1CEST,M3.5.0,M10.5.0/3` (`LOCAL_TIMEZONE` in `ENTSOe.h`, applied with `setenv("TZ", ...)` and `tzset()`).
- At start-up, and then each day after 13:00 local time (`GET_DATA_TIME`) until tomorrow's prices are in, it requests the day-ahead price document from `https://web-api.tp.entsoe.eu/api`:
  - document type `A44` (price document), contract type `A01` (daily),
  - bidding zone `10YNL----------L` (the Netherlands),
  - the window is **today and tomorrow, local time**: `periodStart` = local midnight today, `periodEnd` = local midnight the day after tomorrow, both in UTC as the API wants them (23:00 UTC the day before in winter, 22:00 UTC in summer; see `entsoe_make_url()` in `ENTSOe.cpp`). Before about 13:00 the reply only has today.
- If the request fails, or tomorrow is not published yet, it retries an hour later (a minute later at start-up).
- Parses the XML response with a small built-in parser (`VerySimpleXMLParser.cpp`) and stores the prices per local day in `pricing[page][index]`: page 0 = today, page 1 = tomorrow, index = quarter-hour since local midnight (up to 100 on the day summer time ends), in EUR/MWh. Prices can be negative; slots without a price are `NAN` (test with `isnan()`). Positions that the reply leaves out repeat the price before them (curve type A03). Hourly data (`PT60M`) is spread over four quarter-hours.
- Prints every price, then the highest and the lowest price with their day, quarter-hour index and local time, on the serial monitor (115200 baud).

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
- Libraries: none to install. WiFi (with the `NTP` client) and HTTPClient are included with the arduino-pico core.
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
| `ENTSOE_DOMAIN` | `ENTSOe.h` | `10YNL----------L` | EIC code of your bidding zone. |
| `HTTP_ENDPOINT` | `ENTSOe.h` | `https://web-api.tp.entsoe.eu/api` | |
| `DOCUMENT_TYPE` | `ENTSOe.h` | `A44` | Price document. |
| `CONTRACT_MARKET_AGREEMENT_TYPE` | `ENTSOe.h` | `A01` | Daily. |
| `PAGES`, `ENTREES` | `ENTSOe.h` | 2, 100 | Days and 15-minute slots stored (100 covers the day summer time ends). |
| `GET_DATA_TIME` | `.ino` | 13 | Local hour after which the next day's prices are fetched. |
| `LOCAL_TIMEZONE` | `ENTSOe.h` | `CET-1CEST,M3.5.0,M10.5.0/3` | POSIX time zone string: Europe/Amsterdam with summer time. Change it if your bidding zone has another time zone. |

**TLS certificate.** The sketch checks the server's TLS certificate, so nobody between you and the server can send fake prices. The arduino-pico core has no built-in certificate bundle, so the root certificate of the API server is embedded in `ENTSOe.cpp` (`entsoe_root_ca`, valid until March 2046) and passed with `https.setCACert()`. The check also needs the correct time, which is why the sketch waits for NTP first. If the server ever moves to another certificate authority, the request fails with a TLS error. Then look up the new root (for example `openssl s_client -connect web-api.tp.entsoe.eu:443 -showcerts` shows the chain; take the last certificate) and replace the PEM text in `ENTSOe.cpp`. Do not switch to `setInsecure()` for real use.

## How it talks to the charge controller

It doesn't yet: this example only gets the prices. To turn it into a charge schedule:

1. Add the Modbus code from [charger-control-basic](../charger-control-basic/) (`ModBus.cpp/.h`, `SmartEVSE.cpp/.h`, `Board.h`).
2. Each quarter-hour, look up the price in `pricing[][]`.
3. In cheap slots write the wanted current with `smart_evse_set_max_current()` (register 0x0201); in expensive slots write 0 A.

See the [Modbus register map](../../docs/en/modbus-registers.md). Set the Modbus address of your charge controller (`ADDRESS`). The charger leaves the factory at address 1. Address 1 is special: after boot the charger does not answer there, only after a state change (press the button on the board, or plug in a car). For your own controller, give the charger an address of 2 or higher via register 0x0200 (and save it). Write the current at least once per second: the charger falls back to its static limit when the writes stop.
