#!/usr/bin/env python3
"""NFC card reader EVSE-X-NFC: print the ID of every card that is held against the reader.

Runs on a PC or a Raspberry Pi with a USB-RS485 adapter (A, B, GND to the reader).

Usage: python3 nfc_reader.py <serial_port> [modbus_address] [baud]
  example: python3 nfc_reader.py /dev/ttyUSB0
  example: python3 nfc_reader.py COM3 50 19200

Register map: docs/en/nfc-reader.md. Addresses below are as sent on the wire (0-based);
the maker's manual numbers them from 1 (holding register 1000 there is 999 here).
"""
import sys
import time

from pymodbus.client import ModbusSerialClient

REG_IS_NEW_TAG = 999     # 1 = a new card was read
REG_MODEL = 1016         # model ID, software version, hardware version
COIL_IS_NEW_TAG = 1015   # write False to clear the flag; the reader then reads the next card


def call(fn, *args, address, **kw):
    """pymodbus renamed slave= to device_id= in 3.10; support both."""
    try:
        return fn(*args, device_id=address, **kw)
    except TypeError:
        return fn(*args, slave=address, **kw)


def main():
    if len(sys.argv) < 2 or sys.argv[1] in ("-h", "--help"):
        print(__doc__)
        sys.exit(1)
    port = sys.argv[1]
    address = int(sys.argv[2]) if len(sys.argv) > 2 else 50
    baud = int(sys.argv[3]) if len(sys.argv) > 3 else 9600

    client = ModbusSerialClient(port=port, baudrate=baud, bytesize=8, parity="N", stopbits=1, timeout=0.5)
    if not client.connect():
        sys.exit(f"cannot open {port}")

    rr = call(client.read_holding_registers, REG_MODEL, count=3, address=address)
    if rr.isError():
        sys.exit(f"no reader at address {address}, {baud} baud: check power, A/B, GND, address and baud rate")
    model, sw, hw = rr.registers
    print(f"reader at address {address}: model {model:x}, software {sw:x}, hardware {hw:x}")

    call(client.write_coil, COIL_IS_NEW_TAG, False, address=address)
    print("hold a card against the reader (Ctrl-C to stop)")
    try:
        while True:
            rr = call(client.read_holding_registers, REG_IS_NEW_TAG, count=12, address=address)
            if not rr.isError() and rr.registers[0] == 1:
                length = max(0, min(10, rr.registers[1]))
                uid = "".join(f"{b & 0xFF:02X}" for b in rr.registers[2:2 + length])
                print(f"card: {uid}")
                # your own logic here (whitelist, start the charger), then clear the flag
                call(client.write_coil, COIL_IS_NEW_TAG, False, address=address)
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        client.close()


if __name__ == "__main__":
    main()
