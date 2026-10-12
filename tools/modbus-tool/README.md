# Modbus tool

`smartevse_tool.py` is a small Python program to read and change the registers of the charge controller (EVSE-C-3x32A) from a PC. It shows all status and configuration registers in one table and has a menu for a few common actions.

## What it does

Each time it shows the screen, it:

1. reads the **input registers** (function 0x04) one by one and decodes them: serial number, firmware version, time since boot, pilot min/max, cable and advertised current, state and error, temperature, current and voltage per phase, energy, options, earth quality and more;
2. reads the **holding registers** (function 0x03) and decodes them: Modbus address, remote max current, magic/unlock code, settings bits, LED type, CP disconnect settings and more.

The action registers 0x000A to 0x0017 (save, factory defaults, reset, bootloader, and so on) are **skipped** in this overview, because on the charge controller a read of these registers triggers the action.

Then it shows a menu:

| Key | Action | Register |
|---|---|---|
| 1 | Write 25 (0x0019) to the settings register | 0x0204 (516). 0x0019 sets bits 0 (PHASES), 3 (MISUSE_LOCK) and 4 (AUTO_CP_DISC). |
| 2 | Write a value you type to the settings register | 0x0204 (516) |
| 3 | Save settings to EEPROM (by reading the register) | input 0x000A (10) |
| 4 | Reset the charge controller (by reading the register) | input 0x000C (12) |
| 5 | Disconnect CP (by reading the register) | input 0x0017 (23) |
| r | Reload the screen | |
| q | Quit | |

Writes go to the settings in RAM. Use action 3 to save them. The example sketches (`smart_evse_persist_settings()` in `SmartEVSE.cpp`) note that saving fails unless the settings were first unlocked by writing the magic value 0x1234 to holding register 0x0202. The tool does not write that value; whether it is needed for action 3 is to be confirmed in the [Modbus register map](../../docs/en/modbus-registers.md).

## Install

Python 3 and pymodbus 3.x:

```sh
pip install pymodbus
```

For a serial connection pymodbus also needs pyserial (`pip install pyserial`, or `pip install "pymodbus[serial]"`).

The code does not pin a pymodbus version. It works with versions that use `slave=` (3.9 and older; the code comments cover 3.6) and with `device_id=` (3.10 and newer): it checks which keyword the installed version accepts at start-up and shows it in the header.

## Usage

```sh
python3 smartevse_tool.py <IP address or serial port>
```

The tool decides by the argument: a value that starts with `/dev/`, `COM` or `tty` is a serial port (Modbus RTU); anything else is an IP address (Modbus TCP).

**Serial (Modbus RTU)** with a USB-to-RS485 adapter wired to GND, A and B of the charge controller (3 × 0.75 mm² stranded multi-core, short branches):

```sh
python3 smartevse_tool.py /dev/ttyUSB0      # Linux
python3 smartevse_tool.py COM3              # Windows
```

Settings: 9600 baud, 8N1, timeout 1 s.

**TCP (Modbus TCP)** through a Modbus TCP-to-RTU gateway on the RS485 bus:

```sh
python3 smartevse_tool.py 192.168.1.50
```

Settings: port 502, timeout 2 s.

## Configure

Fixed values at the top of the script:

| Constant | Default | Notes |
|---|---|---|
| `SLAVE` | 1 | Modbus address of the charge controller. Boards ship set to 10: change this to 10 for a new board. At address 1 the charge controller only answers after a state change (press the button on the board). |
| `BAUDRATE` | 9600 | Serial speed. A bus driven by an EVSE-SC-02 site controller runs at 19200 baud; don't connect the tool to that bus while the site controller is running, because a Modbus bus has one master. |
| `PORT` | 502 | TCP port. |

## Registers

Registers read (decimal / hex):

- Input: 0 (serial, first word), 5 (firmware), 6 (ms since boot), 256–269 (pilot, currents, state, temperature, phase currents and voltages, energy, options), 271–275 (session energy per phase, earth quality, UBRR).
- Holding: 512–522 (address, remote max current, magic code, challenge, settings, LED, condition, UBRR, CP disconnect timing and type).

Decoding in the tool differs in places from the example sketches, for example the scaling of phase currents (the tool divides by 10, the sketches by 256) and the state numbering (the tool maps 0 = A, the sketches 1 = A). Check against the [Modbus register map](../../docs/en/modbus-registers.md) when you rely on a value.
