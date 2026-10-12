# Waveshare RP2350 Touch LCD 4

## About this board

The Waveshare RP2350 Touch LCD 4 is an RP2350 board with a 4-inch touch screen and an on-board RS485 interface with A and B terminals. The examples use it only as a Modbus master for the charge controller (EVSE-C-3x32A); the screen is not used yet.

Program it with the Arduino IDE and the "Raspberry Pi Pico/RP2040/RP2350" core (arduino-pico, by Earle Philhower). The sketch comment names the board entry "WaveShare RP2350 LCD 0.96"; the correct entry for the Touch LCD 4 is to be confirmed.

## Used in

- [charger-control-basic / WaveShare_RP2350_Touch_LCD_4](../charger-control-basic/README.md#waveshare_rp2350_touch_lcd_4)

## RS485 / pins

| What | Value in the code |
|---|---|
| UART | UART1 (`Serial2`) |
| TX / RX | GP4 / GP5 (`TXD1`, `RXD1`, defined once in `Board.h`; `ModBus.cpp` only uses GP4/GP5 as a fallback when a board does not define them) |
| Direction (DE) pin | none driven by the code |
| Baud rate | 9600, 8N1 |
| Debug output | USB serial, 115200 baud |

Wiring to the charge controller (from the sketch comment):

| Board terminal | Charge controller terminal |
|---|---|
| A | A |
| B | B |
| not connected | GND |

Use 3 × 0.75 mm² stranded multi-core cable and keep branches short.

## Notes

- The charger leaves the factory at Modbus address 1, and the sketch uses `ADDRESS 1`. Address 1 is special: after boot the charger does not answer there, only after a state change (press the button on the board, or plug in a car). EdgeTech site controllers move each new charger to its own address so all chargers can be reached. For your own controller, give the charger an address of 2 or higher via register 0x0200 (and save it), then set `ADDRESS` to match. On a failed read at address 1 the sketch prints a hint.
- Register map: [docs/en/modbus-registers.md](../../docs/en/modbus-registers.md).
