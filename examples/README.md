# Examples

These examples show ways to use the charge controller (EVSE-C-3x32A) from your own hardware. Use them as inspiration: they are small Arduino sketches that read the charger over Modbus RTU, show its state on a screen, or set the charge current. Copy what you need and build your own.

| Example | What it does | Boards | Difficulty |
|---|---|---|---|
| [charger-control-basic](charger-control-basic/) | Reads serial number, firmware version and charger state, and prints them on the serial monitor | Raspberry Pi Pico 2 W with Waveshare 2-channel RS485 module, Waveshare industrial ESP32-S3 board, Waveshare RP2350 Touch LCD 4, LilyGo T-Panel S3 | Easy |
| [touch-display-control](touch-display-control/) | Shows charger state, currents, voltages and energy on a touch screen, with a current slider and a 1/3-phase button | LilyGo T-Panel S3, M5Stack CoreS3 SE with COMMU module | Medium |
| [smart-meter-load-balancing](smart-meter-load-balancing/) | Reads the house current from a Dutch DSMR 5 smart meter (P1 port) and lowers the charge current so the house stays below a set limit | M5Stack CoreS3 SE with COMMU module | Advanced |
| [smart-meter-reader](smart-meter-reader/) | Reads a DSMR 5 smart meter through its P1 port and prints the current per phase | WIZnet W55RP20-EVB-Pico (any RP2040/RP2350 board should work) | Easy |
| [electricity-price-schedule](electricity-price-schedule/) | Downloads day-ahead electricity prices from ENTSO-E over Wi-Fi and finds the cheapest and most expensive quarter-hours | Raspberry Pi Pico 2 W | Medium |

Board notes: [Waveshare RP2350 LCD](boards/waveshare-rp2350-lcd.md), [WIZnet W55RP20-EVB-Pico](boards/wiznet-w55rp20-evb-pico.md).

A Python tool to read and change registers from a PC is in [tools/modbus-tool](../tools/modbus-tool/).

## Before you start

**Wiring.** The charge controller is a Modbus RTU slave on RS485. Connect your board's RS485 interface to the three terminals on the charge controller: **GND**, **A** and **B**. Use 3 × 0.75 mm² stranded multi-core cable and keep branches short.

**Address and speed.** The examples talk at 9600 baud, 8N1, and use Modbus address 1 (`#define ADDRESS 1` in each sketch).

- The charger leaves the factory at address **1**. This is a special address: after boot the charger only answers after a state change, for example after you press the button on the board. If you get "failed to read over mb", press the button and try again.
- For continuous polling, give the charger its own address: after a state change, write 2 or higher to register 0x0200, save it, and set `ADDRESS` in the sketch to match. An EdgeTech site controller does this for each new charger by itself.
- When an EVSE-SC-02 site controller drives the bus, the bus runs at 19200 baud and the site controller is the master. These examples are also a master, so run them on their own bus, not on a bus that a site controller already drives.

**Registers.** All registers are described in the [Modbus register map](../docs/en/modbus-registers.md).

**Safety.** The examples only talk Modbus at low voltage. Installing or changing anything on the mains side of a charge point (supply, protection, relays, the charging cable) is work for a qualified electrician.
