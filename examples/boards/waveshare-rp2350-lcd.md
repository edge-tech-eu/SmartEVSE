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
| TX / RX in `Board.h` | GP8 / GP9 (`TXD1`, `RXD1`) |
| TX / RX actually used | GP4 / GP5: `ModBus.cpp` redefines `TXD1`/`RXD1` in its `ARDUINO_ARCH_RP2040` block, and that value wins |
| Direction (DE) pin | none driven by the code |
| Baud rate | 9600, 8N1 |
| Debug output | USB serial, 115200 baud |

Which RP2350 pins the board's RS485 transceiver really uses is to be confirmed. If GP8/GP9 is right, change the defines in `ModBus.cpp`.

Wiring to the charge controller (from the sketch comment):

| Board terminal | Charge controller terminal |
|---|---|
| A | A |
| B | B |
| not connected | GND |

Use 3 × 0.75 mm² stranded multi-core cable and keep branches short.

## Notes

- The charge controller ships set to Modbus address 10; the sketch uses `ADDRESS 1`. Change it to match your board. At address 1 the charge controller only answers after a state change (press the button on the board).
- The sketch has a commented-out call to `smart_evse_change_address()`. That function does not exist in `SmartEVSE.cpp`.
- Register map: [docs/en/modbus-registers.md](../../docs/en/modbus-registers.md).
