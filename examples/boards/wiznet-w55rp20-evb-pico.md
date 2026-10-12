# WIZnet W55RP20-EVB-Pico

## About this board

The WIZnet W55RP20-EVB-Pico is a Pico-sized board built around the W55RP20, which combines an RP2040 with a wired Ethernet controller. In the examples it reads a DSMR 5 smart meter through the P1 port. Ethernet is not used yet.

Program it with the Arduino IDE and the "Raspberry Pi Pico/RP2040/RP2350" core (arduino-pico, by Earle Philhower). The `W55RP20lwIP.h` header comes with that core.

## Used in

- [smart-meter-reader / WIZnetPico_Read_SmartMeter](../smart-meter-reader/)

## RS485 / pins

This board has no RS485 interface, and the example does not talk to the charge controller.

| What | Value in the code |
|---|---|
| P1 data in | GP1, UART0 RX (`Serial1`) |
| UART settings | 115200 baud, RX inverted in hardware (`Serial1.setInvertRX(true)`), 1024-byte receive buffer |
| GP0 (UART0 TX) | not used |
| Debug output | USB serial, 115200 baud |

Because the RP2040 can invert the UART input, the P1 signal can go straight to GP1 without an inverter. P1 wiring: see the [smart-meter-reader README](../smart-meter-reader/README.md#wiring).

To also control the charge controller from this board, add an RS485 interface on a free UART and the Modbus code from [charger-control-basic](../charger-control-basic/). Pins for that: to be chosen by you; not in the code.

## Notes

- The sketch waits 10 s at start-up so you can open the serial monitor.
- If no DSMR 5.0 meter is found, the sketch prints the reason code and stops (`while(1) {}`). See the [smart-meter-reader README](../smart-meter-reader/README.md#what-it-does).
- Register map for the charge controller: [docs/en/modbus-registers.md](../../docs/en/modbus-registers.md).
