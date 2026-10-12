# Charger control: basic

Read the charge controller (EVSE-C-3x32A) over Modbus RTU and print what it reports on the serial monitor.

## What it does

At start-up the sketch reads, once:

- the serial number,
- the firmware version,
- the cable current and the advertised current.

Then, every 1, 5 or 10 seconds (depending on the board), it reads the charger state and prints:

- state and error flags,
- board temperature and the cable current (from the PP resistor),
- current per phase (L1, L2, L3),
- voltage per phase,
- energy of the current session and total energy.

The shared code in `SmartEVSE.cpp` also has functions to **write**: `smart_evse_set_max_current()`, `smart_evse_set_phases()` and `smart_evse_set_magic()`. The basic sketches don't call them; they only read. Use them as a starting point for your own control logic.

All four variants use the same `SmartEVSE.cpp/.h` and `ModBus.cpp/.h`. Only `Board.h` and the `.ino` file differ. `ModBus.cpp` picks the serial port and pins with `#ifdef` per board family.

## What you need

- One of the boards under [Variants](#variants).
- An RS485 interface: built in on the Waveshare ESP32-S3 board, the Waveshare RP2350 Touch LCD 4 and the LilyGo T-Panel S3 (RS485 version). For the Raspberry Pi Pico 2 W, use the Waveshare 2-channel RS485 module for Pico.
- Cable: 3 × 0.75 mm² stranded multi-core (GND, A, B). Keep branches short.
- A USB cable to program the board and read the serial monitor.

## Wiring

Connect the board's RS485 terminals to the charge controller:

| Board | Board terminal | Charge controller terminal |
|---|---|---|
| Raspberry Pi Pico 2 W + 2-channel RS485 module | Channel 1: G | GND |
| | Channel 1: A | A |
| | Channel 1: B | B |
| Waveshare RP2350 Touch LCD 4 | not connected (per sketch comment) | GND |
| | A | A |
| | B | B |
| LilyGo T-Panel S3 (RS485) | SGND | GND |
| | L | A |
| | H | B |
| Waveshare industrial ESP32-S3 board | A / B terminals: to be confirmed (not in the code) | A / B |

The terminal names above come from the comments in each `.ino` file. The microcontroller pins are listed under [Variants](#variants).

## Build and upload

- Arduino IDE 2.x (or `arduino-cli`).
- No extra libraries for the Pico, Waveshare ESP32-S3 and RP2350 variants. The Modbus master code is in the sketch folder (`ModBus.cpp`).
- Board packages:
  - Raspberry Pi Pico 2 W and Waveshare RP2350 Touch LCD 4: the "Raspberry Pi Pico/RP2040/RP2350" core (arduino-pico, by Earle Philhower).
  - Waveshare ESP32-S3 board and LilyGo T-Panel S3: the "esp32" core by Espressif, board "ESP32S3 Dev Module".
- Settings per board are under [Variants](#variants).
- Open the serial monitor at 115200 baud. The sketches wait a few seconds (2.5 to 10 s) before they start, so you can open the monitor in time.

## Configure

| Setting | Where | Default | Notes |
|---|---|---|---|
| `ADDRESS` | `.ino` | `1` | Modbus address of the charge controller. The charger leaves the factory at address 1. Address 1 is special: after boot the charger does not answer there, only after a state change (press the button on the board, or plug in a car). EdgeTech site controllers move each new charger to its own address so all chargers can be reached. For your own controller, give the charger an address of 2 or higher via register 0x0200 (and save it), then set `ADDRESS` to match. On a failed read at address 1 the sketch prints a hint. |
| Baud rate | `SmartEVSE.cpp`, `smart_evse_init()` → `mb.begin(9600)` | 9600 | 8N1, fixed in `ModBus.cpp`. |
| Poll interval | `.ino`, `next_time` / `INTERVAL` | 1000, 5000 or 10000 ms | |
| Response timeout | `ModBus.h`, `ku8MBResponseTimeout` | 200 ms | |
| RS485 pins | `ModBus.cpp` | per board | See [Variants](#variants). |

## How it talks to the charge controller

The sketch is the Modbus master; the charge controller is the slave. Functions used (all in `SmartEVSE.cpp`):

| Function | Modbus function | Register(s) | What |
|---|---|---|---|
| `smart_evse_get_serial()` | 0x04 read input | 0x0000, 5 registers | Serial number |
| `smart_evse_get_fw_version()` | 0x04 read input | 0x0005 | Firmware version |
| `smart_evse_get_max_currents()` | 0x04 read input | 0x0102 | Low byte: max cable current; high byte: advertised current |
| `smart_evse_get_charger_state()` | 0x04 read input | 0x0102, 13 registers | Cable current (0x0102 low byte), state (low byte) and error flags (high byte) of 0x0103, temperature (0x0104), current L1–L3 (0x0105–0x0107), session energy (0x0108), voltage L1–L3 (0x0109–0x010B), options (0x010C), total energy (32 bit, 0x010D low word, 0x010E high word) |
| `smart_evse_is_32_amp()` | 0x04 read input | 0x010C | Bit 1 set = 32 A board, clear = 16 A |
| `smart_evse_persist_settings()` | 0x04 read input | 0x000A | Saves settings; returns 0 on success. Unlock first with the magic value. |
| `smart_evse_set_max_current()` | 0x06 write single | 0x0201 | Low byte: max current in A (0 stops charging); high byte: max current for the next second. Write at least once per second. |
| `smart_evse_set_magic()` | 0x06 write single | 0x0202 | Unlock code |
| `smart_evse_set_phases()` | 0x03 read, then 0x06 write single | 0x0204 | Bit 0: 1 = three phases, 0 = one phase. Read-modify-write: the other bits (cable lock, DC leakage, CP disconnect) are kept. |

State values (0x0103 low byte, 1-indexed), as printed by `smart_evse_get_state()`: 1 = A (no car), 2 = B (car connected, not charging), 3 = C (charging), 4 = D (charging, ventilation required).

Error bits (high byte of 0x0103): 0x01 temperature too high, 0x02 stuck relay, 0x04 ground fault, 0x08 CP max too low, 0x10 CP min too high, 0x20 DC leak detected, 0x40 I2C initialise error, 0x80 I2C communication error.

Scaling of currents, voltages and energy: 1/256 units, see the [Modbus register map](../../docs/en/modbus-registers.md). The code divides currents, voltages and energy by 256, the same as the [Modbus tool](../../tools/modbus-tool/).

## Variants

### Raspberry_Pico2W_2CH_RS485

- Board: Raspberry Pi Pico 2 W with the Waveshare 2-channel RS485 module for Pico.
- Board package: arduino-pico, board "Raspberry Pi Pico 2W".
- RS485 on UART1 (`Serial2`): TX = GP4, RX = GP5. No direction pin is driven by the code.
- Uses channel 1 of the RS485 module.
- Polls every 5 s and only prints when a read succeeded.

### WaveShare_Industrial_ESP32-S3_Control_Board

- Board: Waveshare industrial ESP32-S3 board with RS485. The sketch comments link to two Waveshare products (ESP32-S3-RS485-CAN and ESP32-S3-Relay-1CH); which exact model was tested is to be confirmed.
- Board package: esp32, "ESP32S3 Dev Module", Flash Size 8MB, Partition Scheme "8M with spiffs (3M APP/1.5MB SPIFFS)".
- RS485 on UART1: TX = GPIO17, RX = GPIO18, driver enable (DE) = GPIO21. The UART runs in hardware RS485 half-duplex mode.
- Debug output uses `Serial.printf` (115200 baud).
- Polls once a second and only prints when a read succeeded.

### WaveShare_RP2350_Touch_LCD_4

- Board: Waveshare RP2350 Touch LCD 4. The screen is not used in this sketch. See also [the board note](../boards/waveshare-rp2350-lcd.md).
- Board package: arduino-pico. The sketch comment names the board "WaveShare RP2350 LCD 0.96"; the correct board entry for the Touch LCD 4 is to be confirmed.
- RS485 on UART1 (`Serial2`): TX = GP4, RX = GP5, defined once in `Board.h` (`TXD1`/`RXD1`). `ModBus.cpp` only falls back to GP4/GP5 when a board does not define them.
- GND of the charge controller is not connected (per sketch comment).
- Polls every 10 s and only prints when a read succeeded.

### LilyGo_T-Panel-S3

- Board: LilyGo T-Panel S3, RS485 version (V1.2). The screen is not used in this sketch.
- Board package: esp32, "ESP32S3 Dev Module", USB CDC On Boot "Enabled", Flash Size 16MB, Partition Scheme "16M Flash (3MB APP/9.9MB FATFS)", PSRAM "QSPI PSRAM".
- RS485 on UART2: TX = GPIO16, RX = GPIO15 (`pin_config.h`, `T_Panel_V1_2_RS485`). For a V1.0 board, select `T_Panel_V1_0_RS485` in `pin_config.h` (TX = 15, RX = 16).
- The RS485 direction pin sits on the XL95x5 I/O expander (pin `XL95X5_RS485_CON` = 7). The `.ino` defines the expander bus object `bus` and starts it (`Wire.begin()`, `bus->begin()`) before the Modbus port; `ModBus.cpp` then sets the pin low through `bus`. `pin_config.h` and `XL95x5_Driver.*` are in the sketch folder.
- The sketch uses the I/O expander bus class from the "GFX Library for Arduino", so that library must be installed. The sketch compiles and links with the esp32 core 3.x and the current library version.
- Polls every 10 s and only prints when a read succeeded.
