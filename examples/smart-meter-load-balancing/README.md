# Smart meter load balancing

Read the house current from a Dutch DSMR 5 smart meter and lower the charge current of the charge controller (EVSE-C-3x32A) so the house connection is not overloaded.

## What it does

1. **Start-up check** (shown on screen, repeated until everything passes):
   - finds the charge controller and shows its serial number and firmware version,
   - reads whether the board is 16 A or 32 A (register 0x010C),
   - finds the smart meter on the P1 port, checks it is DSMR 5.0 (`1-3:0.2.8(50)`), and checks whether it is a single-phase or three-phase connection,
   - runs a communication test: 100 reads of serial number and firmware version, which must all succeed.
2. **Sync:** writes the board maximum (16 or 32 A) as the max current, and sets the charger to 3 phases.
3. **Every second** (`INTERVAL`):
   - reads the charger state (currents, voltages, energy, state),
   - computes the spare current per phase:
     `spare = MAX_HOME_CURRENT − house current + charger current`,
   - takes the lowest spare current over the three phases, capped at the board maximum,
   - sets it to 0 A if it is below 6 A,
   - writes the max current to the charger: right away when it changed, and again every second otherwise (the charger falls back to its static limit when the writes stop).
4. **Smart meter:** every telegram (once per second on DSMR 5) gives voltage and import/export power per phase. The house current per phase is computed as `1000 × (import kW − export kW) / V`. So with solar export the current can be negative and more is left for the car. A single-phase meter only reports L1; L2 and L3 are then 0 A (no division by zero).
5. **Screen:** shows charger and house values. You can also set the current with the slider and switch between 1 and 3 phases with the button. On a phase change the sketch first sets 0 A for 3 s (writing 0 A every 0.5 s so the charger does not fall back to its static limit), changes the phase bit with a read-modify-write, keeps 0 A for another 0.5 s, then restores the current.

## What you need

- M5Stack CoreS3 SE.
- M5Stack COMMU module (RS485).
- Optional, per `Board.h`: M5Stack LAN Module 13.2 and DIN base. The LAN code (`lan_init()`) is present but not called, so Ethernet is not used.
- A DSMR 5 smart meter with a P1 port, an RJ12 (6P6C) cable, and a 4.7 kΩ resistor.
- Cable to the charge controller: 3 × 0.75 mm² stranded multi-core (GND, A, B). Keep branches short.

## Wiring

**Charge controller (RS485 through the COMMU module):**

| COMMU terminal | Charge controller terminal |
|---|---|
| G (pin 4) | GND |
| A (pin 2) | A |
| B (pin 1) | B |

The COMMU module uses `Serial2` on the CoreS3 SE: RX = GPIO18, TX = GPIO17.

**Smart meter (P1 port, RJ12 6P6C):**

P1 pinout used in the sketch comments: 1 = +5 V, 2 = data request, 3 = data ground, 4 = not connected, 5 = data, 6 = power ground.

| CoreS3 SE ext port | P1 (RJ12) pin | Notes |
|---|---|---|
| Pin 1: GPIO1 (TX) | — | Not connected |
| Pin 2: GPIO2 (RX) | 5 (data) | 4.7 kΩ pull-up to **3.3 V** |
| Pin 3: 5 V | — | Not connected |
| Pin 4: GND | 3 (data ground) and 6 (power ground) | |
| — | 2 (data request) | Tie to pin 1 (+5 V) so the meter sends data |

The P1 data line is read on GPIO2 at 115200 baud, 8N1, with the UART input inverted in software (`Serial1.begin(115200, SERIAL_8N1, 2, 1, true)`), so no inverter is needed. The receive buffer of `Serial1` is enlarged to 2000 bytes before `begin()`, so a whole telegram fits.

GPIO2 is a 3.3 V pin: **do not pull it up to 5 V.** The meter's data output is open collector, so the pull-up resistor sets the logic level; connect it to 3.3 V. The ext port has no 3.3 V pin; take 3.3 V from the M5 bus (pin 12, 3V3). Only the data request line (P1 pin 2) goes to +5 V: it is an input of the meter and never reaches the CoreS3 SE.

**Which 5 V.** The data request draws almost no current, so tie it to P1 pin 1, the meter's own +5 V. P1 pin 1 can supply at most 250 mA. You may also power the board from it if everything you connect stays below that; if not, use a suitable external 5 V supply and connect its ground to GND.

## Build and upload

- Arduino IDE 2.x (or `arduino-cli`).
- Board package: M5Stack (esp32 core by M5Stack), board "M5CoreS3". The board "M5CoreS3" in the Espressif esp32 core 3.x also works (compile-checked).
- Libraries (versions to be confirmed):
  - M5Unified
  - M5GFX
  - M5Module-LAN-13.2 (`M5Module_LAN.h`) and its dependency M5-Ethernet, needed to compile even though Ethernet is not used
  - ArduinoHttpClient, included but not used
- Serial monitor at 115200 baud.

## Configure

| Setting | Where | Default | Notes |
|---|---|---|---|
| `ADDRESS` | `.ino` | `1` | Modbus address of the charge controller. The charger leaves the factory at address 1. Address 1 is special: after boot the charger does not answer there, only after a state change (press the button on the board, or plug in a car). EdgeTech site controllers move each new charger to its own address so all chargers can be reached. For your own controller, give the charger an address of 2 or higher via register 0x0200 (and save it), then set `ADDRESS` to match. On a failed read at address 1 the sketch prints a hint. |
| `MAX_HOME_CURRENT` | `.ino` | `25` | Maximum current per phase for the whole house, in A. Set this to your main fuse or less. |
| `INTERVAL` | `.ino` | 1000 ms | Control loop interval. |
| `ALLOW_MISSING` | `.ino` | 3 | Missed charger reads before the EV values on screen are cleared. |
| `INITIAL_PHASES` | `.ino` | 3 | Phase setting written at start-up. |
| Baud rate | `SmartEVSE.cpp`, `smart_evse_init()` | 9600 | 8N1 to the charge controller. |
| LAN pins | `lan_init()` in `.ino` | CS 13, RST 0, INT 10 | Only if you enable `lan_init()`. Set the DIP switches on the LAN module to match: CS on bus pin 23 (GPIO13), RST on bus pin 24 (GPIO0), INT on bus pin 2 (GPIO10). The other choices (GPIO1, GPIO7, GPIO14) are listed in the `.ino` header. MAC address is fixed in the code (`DE:AD:BE:EF:FE:89`). |

## How it talks to the charge controller

The CoreS3 SE is the Modbus master. Functions in `SmartEVSE.cpp`:

| Register | Access | Used for |
|---|---|---|
| 0x0000 (5 registers) | read input | Serial number (start-up and communication test) |
| 0x0005 | read input | Firmware version, shown as major.minor.patch (`v >> 12`, `(v >> 8) & 0x0F`, `v & 0xFF`) |
| 0x0102–0x010E | read input | Cable current, state and errors, temperature, current L1–L3, session energy, voltage L1–L3, options, total energy (32 bit) |
| 0x010C | read input | Bit 1: 32 A board (sets `board_max_current` to 32, otherwise 16) |
| 0x0201 | write single | Max current in A in the low byte, written every second. The high byte is the "next second" current, always 0 here. See the register map for the exact meaning. |
| 0x0204 | read holding, then write single | Bit 0: 1 = three phases, 0 = one phase. Read-modify-write: the other bits (cable lock, DC leakage, CP disconnect) are kept. |

Full details: [Modbus register map](../../docs/en/modbus-registers.md).
