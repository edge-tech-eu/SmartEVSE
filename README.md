# SmartEVSE

ModBusRegisters.pdf lists the ModBus-RTU/RS485 registers. The SmartEVSE operates by default at 9600 BAUD (8 bits, no parity, 1 stop bit). Be aware that a state change (e.g. pressing the button on the board) is needed before communication is possible with the SmartEVSE with default address 1.

Note that important registers are protected by "magic" (first write the magic code, input register 0x0202) and settings are only persistent after writing them to EEPROM (input register 0x000a)

Installation.png is an quick overview on how to install the SmartEVSE

SmartEVSE mounts can be found in the '3d' folder

Are you looking for the guidelines on building your own charge point? Then continue [here](DHZ-laadpaal.md) (that document is in Dutch)
