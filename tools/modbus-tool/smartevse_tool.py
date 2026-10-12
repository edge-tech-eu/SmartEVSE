#!/usr/bin/env python3
"""EdgeTech charge controller tool: read and change the registers of the EdgeTech
charge controller (EVSE-C-3x32A) over Modbus.

Register map, scaling and state numbering: docs/en/modbus-registers.md
"""
import sys
import time
import os
import inspect
from pymodbus.client import ModbusTcpClient, ModbusSerialClient

try:
    from pymodbus import __version__ as pymodbus_version
except ImportError:
    pymodbus_version = "unknown"

# =================================================================
# --- ARGUMENT PARSING & CONFIGURATION ---
# =================================================================

USAGE = """EdgeTech charge controller tool

Usage: python3 smartevse_tool.py <IP_or_SerialPort> [modbus_address]
  RTU example: python3 smartevse_tool.py /dev/ttyUSB0
  RTU example: python3 smartevse_tool.py COM3 2
  TCP example: python3 smartevse_tool.py 10.1.2.24

modbus_address defaults to 1. The charger leaves the factory at address 1. At address 1
it does not answer after boot, only after a state change (press the button on the board,
or plug in a car). For your own controller, give the charger address 2 or higher via
register 0x0200 (and save it).

Modbus TCP goes through a TCP-to-RTU gateway. That works for testing from a PC, but the
response is slower. EdgeTech site controllers need tight responses from the charger, so
they must be on a wired RS485 bus, not over Ethernet."""

if len(sys.argv) < 2 or sys.argv[1] in ('-h', '--help'):
    print(USAGE)
    sys.exit(1)

TARGET = sys.argv[1]

# Simple heuristic to detect if the target is a serial port
if TARGET.startswith('/dev/') or TARGET.lower().startswith('com') or TARGET.lower().startswith('tty'):
    MODE = 'RTU'
    SERIAL_PORT = TARGET
    IP = None
else:
    MODE = 'TCP'
    IP = TARGET
    SERIAL_PORT = None

# Modbus address of the charger (default 1, see USAGE above)
SLAVE = 1
if len(sys.argv) >= 3:
    try:
        SLAVE = int(sys.argv[2], 0)
    except ValueError:
        print(f"Invalid Modbus address: {sys.argv[2]}")
        sys.exit(1)
    if not 1 <= SLAVE <= 247:
        print(f"Modbus address must be 1..247, got {SLAVE}")
        sys.exit(1)
PORT = 502
BAUDRATE = 9600

# =================================================================
# --- PYMODBUS VERSION COMPATIBILITY ---
# =================================================================
# pymodbus renamed the unit-id keyword from slave= to device_id= in 3.10.0.
#   <= 3.9.x : read_input_registers(..., slave=1)
#   >= 3.10  : read_input_registers(..., device_id=1)
# Detect which name the installed version actually accepts, once, at startup.
#
# NOTE: this must inspect the signature rather than try/except TypeError.
# pymodbus 3.6.x still takes **kwargs, so an unknown device_id= is silently
# swallowed there and the unit id quietly falls back to the default - no
# exception is raised to catch. Checking explicit parameter names, newest
# first, is the only reliable way.

_ID_KW = None

def unit_kw(client):
    """Return the unit-id keyword argument as a dict, e.g. {'device_id': 1}."""
    global _ID_KW
    if _ID_KW is None:
        try:
            params = inspect.signature(client.read_input_registers).parameters
        except (TypeError, ValueError):
            params = {}
        _ID_KW = next(
            (n for n in ("device_id", "slave", "unit") if n in params),
            "device_id",
        )
    return {_ID_KW: SLAVE}

# =================================================================
# --- DECODING FUNCTIONS ---
# =================================================================

def decode_fw(val):
    major = (val >> 12) & 0xF
    minor = (val >> 8) & 0xF
    patch = val & 0xFF
    return f"v{major}.{minor}.{patch}"

def decode_split_bytes(val, name_h, name_l, decoder_h=str, decoder_l=str):
    h = (val >> 8) & 0xFF
    l = val & 0xFF
    return f"{name_h}: {decoder_h(h)} | {name_l}: {decoder_l(l)}"

def decode_error(val):
    # 0x0103 high byte: bitmask, bits combine
    if val == 0: return "No error"
    errors = {
        1: "Temperature too high",
        2: "Stuck relay",
        4: "Ground fault",
        8: "Max CP too low",
        16: "Min CP too high",
        32: "DC leakage detected",
        64: "I2C initialization failed",
        128: "I2C communication error"
    }
    return ", ".join(name for bit, name in errors.items() if val & bit)

def decode_state(val):
    # 0x0103 low byte: IEC 61851 state, 1-indexed (1 = A, 2 = B, 3 = C, 4 = D)
    states = {1: "A", 2: "B", 3: "C", 4: "D"}
    return states.get(val, f"Unknown ({val})")

def decode_current(val):
    # 1/256 A
    return f"{val / 256.0:.2f} A"

def decode_voltage(val):
    # 1/256 V
    return f"{val / 256.0:.1f} V"

def decode_settings_bits(val):
    bits = []
    if val & (1 << 5): bits.append("EV40_CP_DISC")
    if val & (1 << 4): bits.append("AUTO_CP_DISC")
    if val & (1 << 3): bits.append("MISUSE_LOCK")
    if val & (1 << 2): bits.append("DCL_MUST_PRESENT")
    if val & (1 << 1): bits.append("LOCK_STATE")
    if val & (1 << 0): bits.append("PHASES")
    return ", ".join(bits) if bits else "None"

def decode_energy(val):
    # Energy in 1/256 KWh
    return f"{val / 256.0:.2f} kWh"

# =================================================================
# --- REGISTER MAPS ---
# =================================================================

# Input Registers (fc=4) - Sensors & Actions
INPUT_REGISTERS = {
    # System / Actions
    0: ("SerialNumber (0x0000)", str), 
    5: ("FirmwareVersion (0x0005)", decode_fw),
    6: ("MsSinceBoot (0x0006)", str), 
    10: ("SaveEEPROM (0x000A)", str),
    11: ("Factory Defaults (0x000B)", str),
    12: ("Reset (0x000C)", str),
    13: ("EnterBootloader (0x000D)", str),
    14: ("Challenge Response (0x000E)", str),
    15: ("Enable TestMode (0x000F)", str),
    16: ("ExternalLock (0x0010)", str),
    17: ("I2C errors (0x0011)", str),
    18: ("Quiet reset (0x0012)", str),
    19: ("Conditional answer (0x0013)", str),
    20: ("Conditional address (0x0014)", str),
    21: ("Lock lock (0x0015)", str),
    22: ("Unlock lock (0x0016)", str),
    23: ("Disconnect CP (0x0017)", str),
    
    # Sensors and Status
    256: ("MinPilot (0x0100)", str),
    257: ("MaxPilot (0x0101)", str),
    258: ("Current Rating (0x0102)", lambda x: decode_split_bytes(x, "Adv", "Cable")),
    259: ("Status & Error (0x0103)", lambda x: decode_split_bytes(x, "Err", "State", decode_error, decode_state)),
    260: ("Temperature (0x0104)", lambda x: f"{x} °C"),
    261: ("CurrentL1 (0x0105)", decode_current),
    262: ("CurrentL2 (0x0106)", decode_current),
    263: ("CurrentL3 (0x0107)", decode_current),
    264: ("EnergySession (0x0108)", decode_energy),
    265: ("VoltageL1 (0x0109)", decode_voltage),
    266: ("VoltageL2 (0x010A)", decode_voltage),
    267: ("VoltageL3 (0x010B)", decode_voltage),
    268: ("Options (0x010C)", lambda x: f"32A/16A: {(x>>1)&1}, DCL: {x&1}"),
    # 32-bit value: (reg[0x010E] << 16) | reg[0x010D], read as 2 registers
    269: ("Energy (0x010D-0x010E)", decode_energy),
    271: ("EnergySessionL1 (0x010F)", decode_energy),
    272: ("EnergySessionL2 (0x0110)", decode_energy),
    273: ("EnergySessionL3 (0x0111)", decode_energy),
    274: ("Earth quality (0x0112)", str),
    275: ("UBRR (0x0113)", str)
}

# registers that are read as a 32-bit value (low word first)
DOUBLE_REGISTERS = {269}

# Holding Registers (fc=3/5/6) - Configuration
HOLDING_REGISTERS = {
    512: ("ModbusAddr & Disc (0x0200)", lambda x: decode_split_bytes(x, "Disc", "Addr")),
    513: ("RemoteMaxCurrent (0x0201)", lambda x: decode_split_bytes(x, "NextSec", "Cur")),
    514: ("MagicResetCode (0x0202)", lambda x: "Unlocked (0x1234)" if x == 4660 else "Locked"),
    515: ("Challenge (0x0203)", str),
    516: ("Settings (0x0204)", decode_settings_bits),
    517: ("Led (0x0205)", lambda x: "GRB (0)" if x == 0 else "NEO (1)"),
    518: ("Condition (0x0206)", str),
    519: ("UBRR (0x0207)", str),
    520: ("CPDisconnectTime (0x0208)", lambda x: f"{x} ms"),
    521: ("TimeBefCPDiscon (0x0209)", lambda x: f"{x} ms"),
    522: ("CP Disconnect Typ (0x020A)", lambda x: decode_split_bytes(x, "DiscTyp", "CPTyp"))
}

# =================================================================

def clear_screen():
    os.system('clear' if os.name == 'posix' else 'cls')

def get_client():
    if MODE == 'TCP':
        return ModbusTcpClient(IP, port=PORT, timeout=2)
    elif MODE == 'RTU':
        return ModbusSerialClient(
            port=SERIAL_PORT,
            baudrate=BAUDRATE,
            timeout=1,
            parity='N',
            stopbits=1,
            bytesize=8
        )
    print(f"❌ Unknown mode: {MODE}")
    sys.exit(1)

def read_and_print_block(client, name, reg_map, is_holding=False):
    print(f"\n[ {name} ]")
    print(f"{'Register name':<28} | {'Value':<42} | {'Register (Dec / Hex)'}")
    print("-" * 100)
    
    failures = 0

    for reg_addr, (desc, decoder) in sorted(reg_map.items()):
        # PROTECTION: Skip action registers (10 to 23) during dashboard read
        # to prevent accidental reboots or EEPROM wipes!
        if not is_holding and 10 <= reg_addr <= 23:
            continue

        count = 2 if (not is_holding and reg_addr in DOUBLE_REGISTERS) else 1

        if is_holding:
            res = client.read_holding_registers(address=reg_addr, count=count, **unit_kw(client))
        else:
            res = client.read_input_registers(address=reg_addr, count=count, **unit_kw(client))

        if res.isError():
            failures += 1
            print(f"{desc:<28} | {'❌ Error (Timeout / Not implemented)':<42} | Reg {reg_addr:<3} / 0x{reg_addr:04X}")
            continue

        val = res.registers[0]
        if count == 2:
            val = (res.registers[1] << 16) | res.registers[0]
        try:
            decoded_val = decoder(val)
        except Exception:
            decoded_val = f"Error decoding (Raw: {val})"
        
        print(f"{desc:<28} | {str(decoded_val):<42} | Reg {reg_addr:<3} / 0x{reg_addr:04X}")

    if failures and SLAVE == 1:
        print("\nNo answer: at address 1 the charger only answers after a state change")
        print("(press the button on the board, or plug in a car). For your own controller,")
        print("give the charger address 2 or higher via register 0x0200 (and save it).")

def run_dashboard():
    clear_screen()
    client = get_client()
    if not client.connect():
        print(f"\n❌ Connection to {TARGET} failed! (Detected Mode: {MODE})")
        return

    print("="*100)
    print(f" ⚡ EdgeTech charge controller tool - Target: {TARGET} ({MODE}), Modbus address {SLAVE}")
    if MODE == 'TCP':
        print(" Modbus TCP via a TCP-to-RTU gateway: fine for testing, but slower. Site controllers need a wired RS485 bus.")
    print(f" pymodbus {pymodbus_version} - using {list(unit_kw(client))[0]}=")
    print("="*100)

    read_and_print_block(client, "METRICS & STATUS (Input Registers fc=4)", INPUT_REGISTERS, is_holding=False)
    read_and_print_block(client, "CONFIGURATION (Holding Registers fc=3/5/6)", HOLDING_REGISTERS, is_holding=True)

    print("="*100)
    client.close()

def set_config(reg, new_value):
    client = get_client()
    if client.connect():
        print(f"\nWriting value {new_value} (0x{new_value:04X}) to Register {reg} (0x{reg:04X})...")
        res = client.write_register(address=reg, value=new_value, **unit_kw(client))
        if res.isError():
            print("❌ Error writing.")
        else:
            print("✅ Successfully written to RAM.")
        client.close()
    else:
        print("❌ Connection error while writing.")
    time.sleep(1.5)

def execute_action(action_reg):
    client = get_client()
    if client.connect():
        print(f"\nSending read trigger to Input Register {action_reg} (0x{action_reg:04X})...")
        res = client.read_input_registers(address=action_reg, count=1, **unit_kw(client))
        if res.isError():
            print("❌ Error triggering action.")
        else:
            print("✅ Command sent successfully!")
        client.close()
    else:
        print("❌ Connection error.")
    time.sleep(1.5)

def main():
    while True:
        run_dashboard()
        
        print("\nACTIONS:")
        print(" [1] Set Reg 516 (0x0204) to AUTO + MISUSE (Value: 25 / 0x0019)")
        print(" [2] Set Reg 516 (0x0204) manually (Custom value)")
        print(" [3] Save to EEPROM (SaveEEPROM -> Read Reg 10 / 0x000A)")
        print(" [4] Reboot charger (Reset -> Read Reg 12 / 0x000C)")
        print(" [5] Disconnect CP Relay (Disconnect CP -> Read Reg 23 / 0x0017)")
        print(" [r] 🔄 Reload view")
        print(" [q] 🚪 Quit")
        
        choice = input("\nSelect an action: ").strip().lower()
        
        if choice == '1':
            set_config(516, 25)
        elif choice == '2':
            try:
                val = int(input("Enter the desired decimal value for Reg 516: "))
                set_config(516, val)
            except ValueError:
                print("❌ Invalid input.")
                time.sleep(1)
        elif choice == '3':
            execute_action(10)
        elif choice == '4':
            execute_action(12)
        elif choice == '5':
            execute_action(23)
        elif choice == 'r':
            continue
        elif choice == 'q':
            print("\nQuit.")
            break

if __name__ == '__main__':
    main()
