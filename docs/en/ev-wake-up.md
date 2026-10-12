English · Dutch version to follow

# EV wake-up (CP disconnect)

Some cars go into deep sleep when a charging session is paused (IEC 61851 state B: connected, not charging), for example while load balancing gives the available current to the house. Once asleep, they do not react when the charger offers current again, so charging never resumes by itself.

The charge controller can wake them with a **CP disconnect**: a short, deliberate interruption or change of the control pilot (CP) that makes the car believe the cable was unplugged and plugged in again. The car restarts its connection sequence and wakes up. Supported from firmware 2.10.17.

## The three variants

| Variant | What happens on CP | Hardware |
|---|---|---|
| 1. Physical interruption | CP is disconnected for a few seconds (default 3 s), then restored. To the car this is the same as unplugging. | Needs the optional CP-disconnect relay board on the lock port ([ask us](mailto:support@edgetech.eu)) |
| 2. Charger absent ("EV40") | PWM stops and CP is held at −12 V ("charging station not available", state F) for a set time (default 3 s), then the signal returns. | None |
| 3. Unplug/replug sequence ("343S") | Fixed pattern of about 10 s: +12 V for 3 s (idle, no car), −12 V for 4 s (state F), +12 V for 3 s, then normal signalling. For cars that need a fuller unplug signature. | None |

## When it happens

A CP disconnect is only done while the car is in state B (connected, not charging).

- **Automatically:** every time the session enters state B (at plug-in, or when charging pauses) a timer starts, default 30 s. If the car is still in state B when it runs out, one disconnect is done. It does not repeat while the car stays in state B; the next entry into state B arms it again.
- **On command:** a controller can ask for a disconnect at any time. It is only carried out in state B, and never while a disconnect is already running.

Both the automatic trigger and the variant are settings, and both are off by default.

## Configuration over Modbus

See the [register map](modbus-registers.md) for all registers. Modbus RTU over RS485, 9600 baud 8N1. At the factory address 0x01 the board only answers after a state change, for example the button on the board; give it its own address first (register 0x0200).

### 1. Choose the mechanism: settings register 0x0204 (fc 3 / fc 6)

Read the register, change only these bits, and write it back: the same register holds the phase, lock and DC-leakage settings.

| Bit | Mask | Name | Effect |
|---|---|---|---|
| 3 | 0x0008 | MISUSE_LOCK_PORT_AS_CP_DISCONNECT | Variant 1: the cable-lock port drives the CP-disconnect relay. The lock registers (0x0010, 0x0015, 0x0016) then return 0xFFFF |
| 4 | 0x0010 | AUTO_CP_DISCONNECT | Automatic trigger after entering state B. Without it, only commanded disconnects happen |
| 5 | 0x0020 | EV40_CP_DISCONNECT | Software disconnect: variant 2 or 3, chosen below |

If bits 3 and 5 are both set, bit 3 wins: the relay is used and the software variant is ignored. Set only one of the two.

### 2. Choose the software variant: high byte of 0x020A

Register 0x020A holds two bytes: low byte CPType, high byte CPDisconnectType. Read, change only the high byte, write back. High byte 0 = variant 2 (EV40, default), 1 = variant 3 (343S).

### 3. Timing: 0x0208 and 0x0209

- **CPDisconnectTime** (0x0208): length of the disconnect in ms for variants 1 and 2, default 3000. Variant 3 always uses 3 s / 4 s / 3 s.
- **TimeBeforeCPDisconnect** (0x0209): wait in ms after entering state B before the automatic disconnect, default 30000.

### 4. Save the settings

Register writes work at once but are lost at a restart until saved:

1. Write the magic code (default 0x1234) to MagicResetCode (0x0202, fc 6).
2. In the very next transaction, read SaveEEPROM (input register 0x000A, fc 4). It returns 0x0000 when saved, or 0xFFFF if the unlock was missed. The unlock is valid for that one next transaction only.

### 5. Ask for a disconnect

Read input register Disconnect CP (0x0017, fc 4). It returns 1 when the disconnect is done, and 0xFFFF when the charger is not set up for CP disconnect (neither bit 3 nor bit 5 set). It is only carried out in state B, and never while a disconnect is already running.

## Example: automatic wake-up with variant 3

1. Read 0x0204, OR in 0x0030 (bits 4 and 5), write it back.
2. Read 0x020A, set the high byte to 1 and keep the low byte, write it back.
3. Optional: change 0x0209 (wait before the disconnect, ms).
4. Write 0x1234 to 0x0202, then read 0x000A and expect 0x0000.
