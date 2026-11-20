# NFC card reader

Read the NFC card reader (EVSE-X-NFC) over Modbus RTU and print the ID of every card that is held against it.

## What it does

1. At start-up: reads the model and versions of the reader at address 50, to check that it answers.
2. Every second: reads the new-card flag, the UID length and the UID in one request (12 holding registers from 999).
3. When a card was read: prints its UID in hex and clears the flag (coil 1015), so the reader reads the next card.

Put your own logic where the comment says so: compare the UID with a list, and let the charger start.

## Variants

| Folder | Board | Language |
|---|---|---|
| [Raspberry_Pico2W_2CH_RS485](Raspberry_Pico2W_2CH_RS485/) | Raspberry Pi Pico 2 W with the Waveshare 2-channel RS485 module | Arduino (arduino-pico core) |
| [python](python/) | PC or Raspberry Pi with a USB-RS485 adapter | Python 3, pymodbus 3.x |

Python: `pip install pymodbus pyserial`, then `python3 nfc_reader.py /dev/ttyUSB0` (or `COM3`). Optional: address and baud rate, e.g. `python3 nfc_reader.py /dev/ttyUSB0 50 19200`.

## Before you start

- **Wiring and supply**: 12 V DC on + and − (or 5 V DC on the 5V header pin), A and B on the Modbus line, − is GND. See [docs/en/nfc-reader.md](../../docs/en/nfc-reader.md#wiring).
- **Address and baud rate**: the examples use address 50 and 9600 baud, 8N1. They must match the reader's settings (set with the maker's configuration program over USB).
- **One master per bus**: these examples are a Modbus master. Don't run them on a bus that an EdgeTech site controller already drives.
- All registers: [docs/en/nfc-reader.md](../../docs/en/nfc-reader.md#register-map).
