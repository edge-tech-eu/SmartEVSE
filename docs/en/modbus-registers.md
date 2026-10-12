English · Dutch version to follow

# Modbus register map — charge controller EVSE-C-3x32A

> Same source as the register page on edgetech.eu. Firmware 2.10.17 and later.

Applies to EVSE-C-3x32A firmware 2.10.17 and later. Modbus RTU over RS485, 9600 baud 8N1 (19200 when driven by an EVSE-SC-02), factory address 0x01.

**Address 0x01.** The charger leaves the factory at address 0x01. This is a special address: after boot the charger does not answer there, only after a state change (for example the button on the board, or plugging in a car). An EdgeTech site controller moves each new charger to its own address, so all chargers on the bus can be reached. With your own controller, do the same: after a state change, write a new address of 0x02 or higher to register 0x0200 and save it.

**Unit convention.** All analog values use 1/256 units: voltage in 1/256 V, current in 1/256 A, energy in 1/256 kWh. Divide by 256.

**Timing.** The board expects a response window of 40 ms. Use a wired RS485 bus. Modbus over Ethernet (an RS485-to-Ethernet gateway, Modbus TCP) works in principle, but the response is slower: fine for testing from a PC, but not with EdgeTech site controllers, which need tight responses from the charger. Wireless Modbus does not work.

## The two registers you need

| Register | fc | Meaning |
|---|---|---|
| `0x0103` | 4 | Low byte: IEC 61851 state (1 = A, 2 = B, 3 = C, 4 = D). High byte: error bitmask. |
| `0x0201` low byte | 6 | RemoteMaxCurrent in amps. Write at least once per second. `0` stops charging. |

Everything else is monitoring and configuration.

## Read-only registers (fc 4)

### System registers 0x0000–0x0017

| Address | Name | Size | Description |
|---|---|---|---|
| `0x0000` | SerialNumber | 5 | Device serial number, 5 consecutive registers |
| `0x0005` | FirmwareVersion | 1 | Application firmware version |
| `0x0006` | MsSinceBoot | 4 | Milliseconds since boot, 64-bit |
| `0x000A` | SaveEEPROM | 1 | Persist settings to EEPROM. Protected |
| `0x000B` | FactoryDefaults | 1 | Reset all settings. Protected |
| `0x000C` | Reset | 1 | Restart the application. Protected |
| `0x000D` | EnterBootloader | 1 | Enter bootloader. Protected |
| `0x000E` | ChallengeResponse | 1 | Verify magic code |
| `0x000F` | EnableTestMode | 1 | Not used |
| `0x0010` | ExternalLock | 1 | Cable lock: `0` unlocked, `1` locked, `0xFFFF` lock port used as CP disconnect |
| `0x0011` | I2cErrors | 1 | Read and reset internal I²C error count |
| `0x0012` | QuietReset | 1 | Reset without a CP glitch |
| `0x0013` | ConditionalAnswer | 1 | Conditional response |
| `0x0014` | ConditionalAddress | 1 | Conditional response |
| `0x0015` | LockLock | 1 | Engage the cable lock. Returns `1`, or `0xFFFF` if the port is used as CP disconnect |
| `0x0016` | UnlockLock | 1 | Disengage the cable lock. Returns `0`, or `0xFFFF` |
| `0x0017` | DisconnectCP | 1 | Momentary CP disconnect (EV wake-up). Returns `1`, or `0xFFFF` if not configured |

Protected registers need the magic code (default `0x1234`) written to `0x0202` first.

### Status registers 0x0100–0x0113

| Address | Name | Size | Unit | Description |
|---|---|---|---|---|
| `0x0100` | MinPilot | 1 | raw | Minimum measured pilot level |
| `0x0101` | MaxPilot | 1 | raw | Maximum measured pilot level |
| `0x0102` L | MaxCableCurrent | ½ | A | Cable rating from the PP resistor |
| `0x0102` H | MaxCurrent | ½ | A | Currently advertised charge current |
| `0x0103` L | State | ½ | enum | IEC 61851 state, see below |
| `0x0103` H | Error | ½ | bitmask | Error bits, see below |
| `0x0104` | Temperature | 1 | °C | Board temperature |
| `0x0105` | CurrentL1 | 1 | 1/256 A | Current on L1 |
| `0x0106` | CurrentL2 | 1 | 1/256 A | Current on L2 |
| `0x0107` | CurrentL3 | 1 | 1/256 A | Current on L3 |
| `0x0108` | EnergySession | 1 | 1/256 kWh | Session energy, all phases |
| `0x0109` | VoltageL1 | 1 | 1/256 V | Output voltage L1, non-zero only in state C/D |
| `0x010A` | VoltageL2 | 1 | 1/256 V | Output voltage L2 |
| `0x010B` | VoltageL3 | 1 | 1/256 V | Output voltage L3 |
| `0x010C` | Options | 1 | bits | Bit 0: DC leakage module enabled. Bit 1: 32 A mode (0 = 16 A) |
| `0x010D` | Energy | 2 | 1/256 kWh | Lifetime energy, 32-bit: `(reg[0x010E] << 16) \| reg[0x010D]` |
| `0x010F` | EnergySessionL1 | 1 | 1/256 kWh | Session energy L1 |
| `0x0110` | EnergySessionL2 | 1 | 1/256 kWh | Session energy L2 |
| `0x0111` | EnergySessionL3 | 1 | 1/256 kWh | Session energy L3 |
| `0x0112` | EarthQuality | 1 | raw | Earth quality, experimental |
| `0x0113` | UBRR | 1 | raw | Current baud-rate register value |

Conversion: `reg × 1000 / 256` gives millivolts or milliamps. `0x086A` = 2154 → 8414 mA.

### State (0x0103 low byte)

| Value | IEC state | CP voltage | Relay | Meaning |
|---|---|---|---|---|
| 1 | A | +12 V | open | No vehicle |
| 2 | B | +9 V | open | Vehicle connected, not charging |
| 3 | C | +6 V | closed | Charging |
| 4 | D | +3 V | closed | Charging, ventilation required |

The state register is 1-indexed. Voltage and current registers are only meaningful in state C or D.

### Error bits (0x0103 high byte)

| Bit | Value | Error | Severity |
|---|---|---|---|
| 0 | 1 | Temperature too high | Critical |
| 1 | 2 | Stuck relay | Critical |
| 2 | 4 | Ground fault | Critical |
| 3 | 8 | Max CP too low | Warning, pilot out of range |
| 4 | 16 | Min CP too high | Warning, pilot out of range |
| 5 | 32 | DC leakage detected by the on-board DC-leakage sensor (EVSE-X-DCD) | Charging blocked |
| 6 | 64 | I²C initialisation failed | Internal |
| 7 | 128 | I²C communication error | Internal |

`0` means no error. Bits combine.

## Read/write registers (fc 3 read, fc 6 write)

### Configuration 0x0200–0x020A

| Address | Name | Size | Description |
|---|---|---|---|
| `0x0200` L | ModbusAddress | ½ | Slave address. Protected. Factory `0x01` (answers only after a state change); site controllers move each charger to its own address |
| `0x0200` H | Discovery | ½ | Not used |
| `0x0201` L | RemoteMaxCurrent | ½ | Advertised charge current limit in amps. `0` disables charging. The control register. |
| `0x0201` H | RemoteMaxCurrentNextSecond | ½ | One-second failsafe limit; falls back to RemoteMaxCurrent unless rewritten within 1 s. Not for normal control. |
| `0x0202` | MagicResetCode | 1 | Change the magic code |
| `0x0203` | Challenge | 1 | Challenge–response for protected writes |
| `0x0204` | Settings | 1 | Bitfield, see below |
| `0x0205` | Led | 1 | `0` onboard GRB LED, `1` addressable LED strip (36 LEDs) |
| `0x0206` | Condition | 1 | Conditional response |
| `0x0207` | UBRR | 1 | Baud-rate register (`0x33` = 19200) |
| `0x0208` | CPDisconnectTime | 1 | CP disconnect duration, ms |
| `0x0209` | TimeBeforeCPDisconnect | 1 | Wait after advertising current before checking state C, ms |
| `0x020A` L | CPType | ½ | `0` normal, `1` A1/B1 pilot levels |
| `0x020A` H | CPDisconnectType | ½ | `0` EV40, `1` extended EV40 |

### Settings bitfield (0x0204)

| Bit | Name | Meaning |
|---|---|---|
| 0 | PHASES | 1-phase (0) or 3-phase (1) |
| 1 | LOCK_STATE | Cable lock behaviour |
| 2 | DCL_MUST_BE_PRESENT | Block charging unless the DC leakage module is present |
| 3 | MISUSE_LOCK_PORT_AS_CP_DISCONNECT | Use the lock port for CP disconnect |
| 4 | AUTO_CP_DISCONNECT | Automatic CP disconnect after a pause (EV wake-up) |
| 5 | EV40_CP_DISCONNECT | EV40-style CP disconnect |

## Bootloader (address 0xF0)

| Address | fc | Name | Description |
|---|---|---|---|
| `0x0500` | 4 | BootloaderVersion | Bootloader version, `0` when read at the application address |
| variable | 0x10 | FlashPage | 64-register firmware page writes; low address byte = page, high byte set = last page |

The site controller uses this to update charge controller firmware in the field.

## Minimal controller loop

```text
every 1 s:
    state, error = read fc4 0x0103           # low byte state, high byte error
    if error & 0b111: limit = 0              # temperature, stuck relay, ground fault
    else:            limit = min(site_headroom_A, cable_A, 32)
    write fc6 0x0201 = limit                 # amps; 0 stops
```

Miss two consecutive writes and the board falls back to its static limit.
