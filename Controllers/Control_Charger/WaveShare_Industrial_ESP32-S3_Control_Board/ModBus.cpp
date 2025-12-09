/**
@file
Arduino library for communicating with Modbus slaves over RS232/485 (via RTU protocol).
*/
/*
  ModbusMaster.cpp - Arduino library for communicating with Modbus slaves
  over RS232/485 (via RTU protocol).
  
  This file is part of ModbusMaster.
  
  ModbusMaster is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  ModbusMaster is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with ModbusMaster.  If not, see <http://www.gnu.org/licenses/>.
  
  Written by Doc Walker (Rx)
  Copyright Â© 2009-2013 Doc Walker <4-20ma at wvfans dot net>
  Adapted for Spark Core by Paul Kourany, March 14, 2014
  Further modifications and port to Particle Photon by Anurag Chugh, July 5, 2016 <lithiumhead at gmail dot com>
*/

/* _____PROJECT INCLUDES_____________________________________________________ */
#include "ModBus.h"

/* _____GLOBAL VARIABLES_____________________________________________________ */

// USARTSerial MBSerial = Serial1;		 ///< Pointer to Serial1 class object

// adapted and hardcoded for WaveShare ESP32_S3_Relay_1CH

// fixed port: HardwareSerial MBSerial(1);
// using automatic rs485 (UART_MODE_RS485_HALF_DUPLEX, MBUseEnablePin = 0)
// logging: printf

#ifndef ARDUINO_ESP32S3_DEV
#error ModBus.cpp adapted to ESP32S3, check out other examples for other architectures
#endif

#define TXD1              17
#define RXD1              18
#define TXD1EN            21

HardwareSerial MBSerial(1);
uint8_t MBTXEnablePin = TXD1EN;			
uint8_t MBUseEnablePin = 0; // disable because automatic

uint8_t u8ModbusADU[256];
char dumpBuffer[10];

// Fix to define word type conversion function
uint16_t word(uint8_t high, uint8_t low) {
	uint16_t ret_val = low;
	ret_val += (high << 8);
	return ret_val;
}



/* _____PROJECT INCLUDES_____________________________________________________ */
// functions to calculate Modbus Application Data Unit CRC
// fix for #include <util/crc16.h>
uint16_t _crc16_update(uint16_t crc, uint8_t a) {
	int i;
	
	crc ^= a;
	for (i = 0; i < 8; ++i)
	{
		if (crc & 1)
			crc = (crc >> 1) ^ 0xA001;
		else
			crc = (crc >> 1);
	}
	return crc;
}



/* _____PUBLIC FUNCTIONS_____________________________________________________ */
/**
Constructor.

Creates class object using default serial port 0, Modbus slave ID 1.

@ingroup setup
*/
Modbus::Modbus(void) {
	//_u8SerialPort = 1;
}


/**
Constructor.

Creates class object using specified serial port, Modbus slave ID.

@overload void ModbusMaster::ModbusMaster(uint8_t u8SerialPort, uint8_t u8MBSlave)
@param u8SerialPort serial port (Serial1, Serial2)
@param u8MBSlave Modbus slave ID (1..255)
@ingroup setup
*/
//Modbus::Modbus(uint8_t u8SerialPort) {
//	_u8SerialPort = ((u8SerialPort > 2) || (u8SerialPort < 1)) ? 1 : u8SerialPort;
//}


/**
Initialize class object.

Sets up the serial port using default 9600 baud rate.
Call once class has been instantiated, typically within setup().

@ingroup setup
*/
void Modbus::begin(void) {
	begin(9600);
}


/**
Initialize class object.

Sets up the serial port using specified baud rate.
Call once class has been instantiated, typically within setup().

@overload ModbusMaster::begin(uint16_t u16BaudRate, uint8_t txrxpin)
@param u16BaudRate baud rate, in standard increments (300..115200)
@ingroup setup
*/
void Modbus::begin(uint16_t u16BaudRate) {
	_u8TransmitBufferIndex = 0;
	u16TransmitBufferLength = 0;
	
	MBSerial.begin(u16BaudRate, SERIAL_8N1, RXD1, TXD1);

	if (!MBSerial.setPins(-1, -1, -1, TXD1EN)) {
    printf("Failed to set TXDEN pins\r\n");
  }

  if (!MBSerial.setMode(UART_MODE_RS485_HALF_DUPLEX)) {
    printf("Failed to set RS485 mode\r\n");
  }
}


/**
Enable printing of debug messages.

When called, it enables the printing of debug messages
and the sent and received Modbus frames.
The messages are printed on "Serial" interface which needs to be preconfigured beforehand.

Beware, enabling this messes up the timings for RS485 Transactions, causing them to fail.

@ingroup setup

void Modbus::enableDebug() {
	MBDebugSerialPrint = 1;
}
*/

/**
Disables printing od debug messages.

Does opposite of what enableDebug() does

@ingroup setup

void Modbus::disableDebug() {
	MBDebugSerialPrint = 0;
}
*/

/**
Enable and set the GPIO pin that is required by some RS485 Drivers to switch from receiving to transmitting mode

@ingroup setup
*/
//void Modbus::enableTXpin(uint8_t pin) {
//	pinMode(pin, OUTPUT);
//	MBTXEnablePin = pin;
//	MBUseEnablePin = 1; 
//}


/**
Disables the use of a GPIO pin to trigger RS485 driver to TX mode

Does opposite of what enableTXpin() does

@ingroup setup
*/
//void Modbus::disableTXpin() {
//	MBUseEnablePin = 0; 
//}
	

void Modbus::beginTransmission(uint16_t u16Address) {
	_u16WriteAddress = u16Address;
	_u8TransmitBufferIndex = 0;
	u16TransmitBufferLength = 0;
}

/*
// eliminate this function in favor of using existing MB request functions
uint8_t Modbus::requestFrom(uint16_t address, uint16_t quantity) {
	uint8_t read;
	// clamp to buffer length
	if(quantity > ku8MaxBufferSize) {
		quantity = ku8MaxBufferSize;
	}
	// set rx buffer iterator vars
	_u8ResponseBufferIndex = 0;
	_u8ResponseBufferLength = read;
	
	return read;
}
*/

void Modbus::sendBit(bool data) {
	uint8_t txBitIndex = u16TransmitBufferLength % 16;
	if ((u16TransmitBufferLength >> 4) < ku8MaxBufferSize)	 {
		if (0 == txBitIndex) {
			_u16TransmitBuffer[_u8TransmitBufferIndex] = 0;
		}
		bitWrite(_u16TransmitBuffer[_u8TransmitBufferIndex], txBitIndex, data);
		u16TransmitBufferLength++;
		_u8TransmitBufferIndex = u16TransmitBufferLength >> 4;
	}
}


void Modbus::send(uint16_t data) {
	if (_u8TransmitBufferIndex < ku8MaxBufferSize) {
		_u16TransmitBuffer[_u8TransmitBufferIndex++] = data;
		u16TransmitBufferLength = _u8TransmitBufferIndex << 4;
	}
}


void Modbus::send(uint32_t data) {
	send(lowWord(data));
	send(highWord(data));
}


void Modbus::send(uint8_t data) {
	send(word(0x00, data));	  //MSB = 0, LSB = data
}


uint8_t Modbus::available(void) {
	return _u8ResponseBufferLength - _u8ResponseBufferIndex;
}


uint16_t Modbus::receive(void) {
	if (_u8ResponseBufferIndex < _u8ResponseBufferLength) {
		return _u16ResponseBuffer[_u8ResponseBufferIndex++];
	}
	else {
		return 0xFFFF;
	}
}


/**
Retrieve data from response buffer.

@see ModbusMaster::clearResponseBuffer()
@param u8Index index of response buffer array (0x00..0x3F)
@return value in position u8Index of response buffer (0x0000..0xFFFF)
@ingroup buffer
*/
uint16_t Modbus::getResponseBuffer(uint8_t u8Index) {
	if (u8Index < ku8MaxBufferSize) {
		return _u16ResponseBuffer[u8Index];
	} else {
		return 0xFFFF;
	}
}


/**
Clear Modbus response buffer.

@see ModbusMaster::getResponseBuffer(uint8_t u8Index)
@ingroup buffer
*/
void Modbus::clearResponseBuffer() {
	uint8_t i;
  
	for (i = 0; i < ku8MaxBufferSize; i++) {
		_u16ResponseBuffer[i] = 0;
	}
}


/**
Place data in transmit buffer.

@see ModbusMaster::clearTransmitBuffer()
@param u8Index index of transmit buffer array (0x00..0x3F)
@param u16Value value to place in position u8Index of transmit buffer (0x0000..0xFFFF)
@return 0 on success; exception number on failure
@ingroup buffer
*/
uint8_t Modbus::setTransmitBuffer(uint8_t u8Index, uint16_t u16Value) {
	if (u8Index < ku8MaxBufferSize) {
		_u16TransmitBuffer[u8Index] = u16Value;
		return ku8MBSuccess;
	} else {
		return ku8MBIllegalDataAddress;
	}
}


/**
Clear Modbus transmit buffer.

@see ModbusMaster::setTransmitBuffer(uint8_t u8Index, uint16_t u16Value)
@ingroup buffer
*/
void Modbus::clearTransmitBuffer() {
	uint8_t i;
  
	for (i = 0; i < ku8MaxBufferSize; i++) {
	  _u16TransmitBuffer[i] = 0;
	}
}

/**
Modbus function 0x01 Read Coils.

This function code is used to read from 1 to 2000 contiguous status of 
coils in a remote device. The request specifies the starting address, 
i.e. the address of the first coil specified, and the number of coils. 
Coils are addressed starting at zero.

The coils in the response buffer are packed as one coil per bit of the 
data field. Status is indicated as 1=ON and 0=OFF. The LSB of the first 
data word contains the output addressed in the query. The other coils 
follow toward the high order end of this word and from low order to high 
order in subsequent words.

If the returned quantity is not a multiple of sixteen, the remaining 
bits in the final data word will be padded with zeros (toward the high 
order end of the word).

@param u16ReadAddress address of first coil (0x0000..0xFFFF)
@param u16BitQty quantity of coils to read (1..2000, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup discrete
*/
uint8_t Modbus::readCoils(uint8_t u8MBSlave, uint16_t u16ReadAddress, uint16_t u16BitQty) {
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16BitQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBReadCoils);
}


/**
Modbus function 0x02 Read Discrete Inputs.

This function code is used to read from 1 to 2000 contiguous status of 
discrete inputs in a remote device. The request specifies the starting 
address, i.e. the address of the first input specified, and the number 
of inputs. Discrete inputs are addressed starting at zero.

The discrete inputs in the response buffer are packed as one input per 
bit of the data field. Status is indicated as 1=ON; 0=OFF. The LSB of 
the first data word contains the input addressed in the query. The other 
inputs follow toward the high order end of this word, and from low order 
to high order in subsequent words.

If the returned quantity is not a multiple of sixteen, the remaining 
bits in the final data word will be padded with zeros (toward the high 
order end of the word).

@param u16ReadAddress address of first discrete input (0x0000..0xFFFF)
@param u16BitQty quantity of discrete inputs to read (1..2000, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup discrete
*/
uint8_t Modbus::readDiscreteInputs(uint8_t u8MBSlave, uint16_t u16ReadAddress,
  uint16_t u16BitQty) {
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16BitQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBReadDiscreteInputs);
}


/**
Modbus function 0x03 Read Holding Registers.

This function code is used to read the contents of a contiguous block of 
holding registers in a remote device. The request specifies the starting 
register address and the number of registers. Registers are addressed 
starting at zero.

The register data in the response buffer is packed as one word per 
register.

@param u16ReadAddress address of the first holding register (0x0000..0xFFFF)
@param u16ReadQty quantity of holding registers to read (1..125, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t Modbus::readHoldingRegisters(uint8_t u8MBSlave, uint16_t u16ReadAddress,
  uint16_t u16ReadQty) {
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16ReadQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBReadHoldingRegisters);
}


/**
Modbus function 0x04 Read Input Registers.

This function code is used to read from 1 to 125 contiguous input 
registers in a remote device. The request specifies the starting 
register address and the number of registers. Registers are addressed 
starting at zero.

The register data in the response buffer is packed as one word per 
register.

@param u16ReadAddress address of the first input register (0x0000..0xFFFF)
@param u16ReadQty quantity of input registers to read (1..125, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t Modbus::readInputRegisters(uint8_t u8MBSlave, uint16_t u16ReadAddress,
  uint8_t u16ReadQty) {
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16ReadQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBReadInputRegisters);
}


/**
Modbus function 0x05 Write Single Coil.

This function code is used to write a single output to either ON or OFF 
in a remote device. The requested ON/OFF state is specified by a 
constant in the state field. A non-zero value requests the output to be 
ON and a value of 0 requests it to be OFF. The request specifies the 
address of the coil to be forced. Coils are addressed starting at zero.

@param u16WriteAddress address of the coil (0x0000..0xFFFF)
@param u8State 0=OFF, non-zero=ON (0x00..0xFF)
@return 0 on success; exception number on failure
@ingroup discrete
*/
uint8_t Modbus::writeSingleCoil(uint8_t u8MBSlave, uint16_t u16WriteAddress, uint8_t u8State) {
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = (u8State ? 0xFF00 : 0x0000);
	return ModbusMasterTransaction(u8MBSlave, ku8MBWriteSingleCoil);
}


/**
Modbus function 0x06 Write Single Register.

This function code is used to write a single holding register in a 
remote device. The request specifies the address of the register to be 
written. Registers are addressed starting at zero.

@param u16WriteAddress address of the holding register (0x0000..0xFFFF)
@param u16WriteValue value to be written to holding register (0x0000..0xFFFF)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t Modbus::writeSingleRegister(uint8_t u8MBSlave, uint16_t u16WriteAddress,
  uint16_t u16WriteValue) {
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = 0;
	_u16TransmitBuffer[0] = u16WriteValue;
	return ModbusMasterTransaction(u8MBSlave, ku8MBWriteSingleRegister);
}


/**
Modbus function 0x0F Write Multiple Coils.

This function code is used to force each coil in a sequence of coils to 
either ON or OFF in a remote device. The request specifies the coil 
references to be forced. Coils are addressed starting at zero.

The requested ON/OFF states are specified by contents of the transmit 
buffer. A logical '1' in a bit position of the buffer requests the 
corresponding output to be ON. A logical '0' requests it to be OFF.

@param u16WriteAddress address of the first coil (0x0000..0xFFFF)
@param u16BitQty quantity of coils to write (1..2000, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup discrete
*/
uint8_t Modbus::writeMultipleCoils(uint8_t u8MBSlave, uint16_t u16WriteAddress,
  uint16_t u16BitQty) {
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = u16BitQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBWriteMultipleCoils);
}

uint8_t Modbus::writeMultipleCoils(uint8_t u8MBSlave) {
	_u16WriteQty = u16TransmitBufferLength;
	return ModbusMasterTransaction(u8MBSlave, ku8MBWriteMultipleCoils);
}


/**
Modbus function 0x10 Write Multiple Registers.

This function code is used to write a block of contiguous registers (1 
to 123 registers) in a remote device.

The requested written values are specified in the transmit buffer. Data 
is packed as one word per register.

@param u16WriteAddress address of the holding register (0x0000..0xFFFF)
@param u16WriteQty quantity of holding registers to write (1..123, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t Modbus::writeMultipleRegisters(uint8_t u8MBSlave, uint16_t u16WriteAddress,
  uint16_t u16WriteQty) {
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = u16WriteQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBWriteMultipleRegisters);
}

// new version based on Wire.h
uint8_t Modbus::writeMultipleRegisters(uint8_t u8MBSlave) {
	_u16WriteQty = _u8TransmitBufferIndex;
	return ModbusMasterTransaction(u8MBSlave, ku8MBWriteMultipleRegisters);
}


/**
Modbus function 0x16 Mask Write Register.

This function code is used to modify the contents of a specified holding 
register using a combination of an AND mask, an OR mask, and the 
register's current contents. The function can be used to set or clear 
individual bits in the register.

The request specifies the holding register to be written, the data to be 
used as the AND mask, and the data to be used as the OR mask. Registers 
are addressed starting at zero.

The function's algorithm is:

Result = (Current Contents && And_Mask) || (Or_Mask && (~And_Mask))

@param u16WriteAddress address of the holding register (0x0000..0xFFFF)
@param u16AndMask AND mask (0x0000..0xFFFF)
@param u16OrMask OR mask (0x0000..0xFFFF)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t Modbus::maskWriteRegister(uint8_t u8MBSlave, uint16_t u16WriteAddress,
  uint16_t u16AndMask, uint16_t u16OrMask) {
	_u16WriteAddress = u16WriteAddress;
	_u16TransmitBuffer[0] = u16AndMask;
	_u16TransmitBuffer[1] = u16OrMask;
	return ModbusMasterTransaction(u8MBSlave, ku8MBMaskWriteRegister);
}


/**
Modbus function 0x17 Read Write Multiple Registers.

This function code performs a combination of one read operation and one 
write operation in a single MODBUS transaction. The write operation is 
performed before the read. Holding registers are addressed starting at 
zero.

The request specifies the starting address and number of holding 
registers to be read as well as the starting address, and the number of 
holding registers. The data to be written is specified in the transmit 
buffer.

@param u16ReadAddress address of the first holding register (0x0000..0xFFFF)
@param u16ReadQty quantity of holding registers to read (1..125, enforced by remote device)
@param u16WriteAddress address of the first holding register (0x0000..0xFFFF)
@param u16WriteQty quantity of holding registers to write (1..121, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t Modbus::readWriteMultipleRegisters(uint8_t u8MBSlave, uint16_t u16ReadAddress,
  uint16_t u16ReadQty, uint16_t u16WriteAddress, uint16_t u16WriteQty) {
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16ReadQty;
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = u16WriteQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBReadWriteMultipleRegisters);
}
uint8_t Modbus::readWriteMultipleRegisters(uint8_t u8MBSlave, uint16_t u16ReadAddress,
  uint16_t u16ReadQty) {
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16ReadQty;
	_u16WriteQty = _u8TransmitBufferIndex;
	return ModbusMasterTransaction(u8MBSlave, ku8MBReadWriteMultipleRegisters);
}


/* _____PRIVATE FUNCTIONS____________________________________________________ */
/**
Modbus transaction engine.
Sequence:
  - assemble Modbus Request Application Data Unit (ADU),
	based on particular function called
  - transmit request over selected serial port
  - wait for/retrieve response
  - evaluate/disassemble response
  - return status (success/exception)

@param u8MBFunction Modbus function (0x01..0xFF)
@return 0 on success; exception number on failure
*/
uint8_t Modbus::ModbusMasterTransaction(uint8_t u8MBSlave, uint8_t u8MBFunction) {
	uint8_t u8ModbusADUSize = 0;
	uint8_t i, u8Qty;
	uint16_t u16CRC;
	//uint32_t u32StartTime;
	//uint8_t u8BytesLeft = 9;
	uint8_t u8MBStatus = ku8MBSuccess;
	
	// assemble Modbus Request Application Data Unit
	u8ModbusADU[u8ModbusADUSize++] = u8MBSlave;
	u8ModbusADU[u8ModbusADUSize++] = u8MBFunction;
  
	switch(u8MBFunction) {
		case ku8MBReadCoils:
		case ku8MBReadDiscreteInputs:
		case ku8MBReadInputRegisters:
		case ku8MBReadHoldingRegisters:
		case ku8MBReadWriteMultipleRegisters:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16ReadAddress);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16ReadAddress);
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16ReadQty);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16ReadQty);
			break;
	}
  
	switch(u8MBFunction) {
		case ku8MBWriteSingleCoil:
		case ku8MBMaskWriteRegister:
		case ku8MBWriteMultipleCoils:
		case ku8MBWriteSingleRegister:
		case ku8MBWriteMultipleRegisters:
		case ku8MBReadWriteMultipleRegisters:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteAddress);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteAddress);
			break;
	}
  
	switch(u8MBFunction) {
		case ku8MBWriteSingleCoil:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteQty);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty);
			break;
	  
		case ku8MBWriteSingleRegister:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[0]);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[0]);
			break;
	  
		case ku8MBWriteMultipleCoils:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteQty);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty);
			u8Qty = (_u16WriteQty % 8) ? ((_u16WriteQty >> 3) + 1) : (_u16WriteQty >> 3);
			u8ModbusADU[u8ModbusADUSize++] = u8Qty;
			for (i = 0; i < u8Qty; i++) {
				switch(i % 2) {
					case 0: // i is even
						u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[i >> 1]);
						break;
					
					case 1: // i is odd
						u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[i >> 1]);
						break;
				}
			}
			break;
	  
		case ku8MBWriteMultipleRegisters:
		case ku8MBReadWriteMultipleRegisters:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteQty);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty << 1);
			
			for (i = 0; i < lowByte(_u16WriteQty); i++) {
				u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[i]);
				u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[i]);
			}
			break;
		
		case ku8MBMaskWriteRegister:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[0]);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[0]);
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[1]);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[1]);
			break;
	}
  
	// append CRC
	u16CRC = 0xFFFF;
	for (i = 0; i < u8ModbusADUSize; i++) {
		u16CRC = _crc16_update(u16CRC, u8ModbusADU[i]);
	}
	u8ModbusADU[u8ModbusADUSize++] = lowByte(u16CRC);
	u8ModbusADU[u8ModbusADUSize++] = highByte(u16CRC);
	u8ModbusADU[u8ModbusADUSize] = 0;
	
	/*
	if (MBSerial.available()) { //Empty the receive buffer before beginning.
		MBSerial.read();
	}
	*/

	int extranuousBytesRead;

	// clear reading bufffer
	MBSerial.setTimeout(4);
	do {
		extranuousBytesRead = MBSerial.readBytes(dumpBuffer,10);
	} while (extranuousBytesRead==10);

	// Switch RS485 driver to transmitting mode.		
	if (MBUseEnablePin == 1) {
		digitalWrite(MBTXEnablePin, HIGH);  
	}
	
	// SINGLE_THREADED_BLOCK() {
		{
		// Transmit request
		MBSerial.write(u8ModbusADU,u8ModbusADUSize);

		u8ModbusADUSize = 0;

		// Wait for transmission to get completed
		MBSerial.flush(); 

		// Switch RS485 driver back to receiving mode.
		if (MBUseEnablePin == 1) {  
			digitalWrite(MBTXEnablePin, LOW);  
		}
	}

	// read 1 byte, needs to slave address otherwise assume noise
	MBSerial.setTimeout(40);
	int maxReads = 5;
	u8ModbusADU[0] = 0;
	do {
    	u8ModbusADUSize = MBSerial.readBytes((char *)u8ModbusADU,1);
		maxReads--;
	} while((u8ModbusADU[0] != u8MBSlave)&&(maxReads>0));

	// Read 5 bytes function code, 1+1.5 ms each = 12.5ms
    MBSerial.setTimeout(20);
    u8ModbusADUSize += MBSerial.readBytes((char *)(&u8ModbusADU[1]),4);

	if (u8ModbusADUSize!=5){                 
		u8MBStatus = ku8MBResponseTimedOut;
		printf("mb: read adu failed, size %d != 5\r\n",u8ModbusADUSize);
	} else {
		if (u8ModbusADU[0] != u8MBSlave) {
			u8MBStatus = ku8MBInvalidSlaveID;
			printf("mb: invalid slave id\r\n");
		}

		if ((u8ModbusADU[1] & 0x7F) != u8MBFunction) {
			u8MBStatus = ku8MBInvalidFunction;
			printf("mb: invalid function\r\n");
		}

		if (bitRead(u8ModbusADU[1], 7)) {
			u8MBStatus = u8ModbusADU[2];
			printf("mb: exception %d\r\n",(int)u8MBStatus);
		}
	}

	// Read data if all good so far
	if(u8MBStatus == ku8MBSuccess) {

		int u8BytesLeft = 0;

		// Determine bytes to be read
		switch(u8ModbusADU[1]) {
			case ku8MBReadCoils:
			case ku8MBReadDiscreteInputs:
			case ku8MBReadInputRegisters:
			case ku8MBReadHoldingRegisters:
			case ku8MBReadWriteMultipleRegisters:
				u8BytesLeft = u8ModbusADU[2];
				break;
			
			case ku8MBWriteSingleCoil:
			case ku8MBWriteMultipleCoils:
			case ku8MBWriteSingleRegister:
			case ku8MBWriteMultipleRegisters:
				u8BytesLeft = 3;
				break;
			
			case ku8MBMaskWriteRegister:
				u8BytesLeft = 5;
				break;
		}

		// Set timeout and read remaining data
        MBSerial.setTimeout(u8BytesLeft*2);
        int bytesRead = MBSerial.readBytes((char *)(u8ModbusADU+5),u8BytesLeft);

		u8ModbusADUSize += bytesRead;

		if(bytesRead!=u8BytesLeft) {

			u8MBStatus = ku8MBResponseTimedOut;
			printf("mb: data read %d != %d\r\n",bytesRead,u8BytesLeft);

		} else {

			// calculate CRC
			u16CRC = 0xFFFF;
			for (i = 0; i < (u8ModbusADUSize - 2); i++) {
				u16CRC = _crc16_update(u16CRC, u8ModbusADU[i]);
			}
			
			// verify CRC
			if (lowByte(u16CRC) != u8ModbusADU[u8ModbusADUSize - 2] ||
			highByte(u16CRC) != u8ModbusADU[u8ModbusADUSize - 1]) {
				u8MBStatus = ku8MBInvalidCRC;
				printf("mb: invalid crc\r\n");
			}
		}
	}

	// disassemble ADU into words
	if (!u8MBStatus) {
		// evaluate returned Modbus function code
		switch(u8ModbusADU[1]) {
			case ku8MBReadCoils:
			case ku8MBReadDiscreteInputs:
				// load bytes into word; response bytes are ordered L, H, L, H, ...
				for (i = 0; i < (u8ModbusADU[2] >> 1); i++) {
					if (i < ku8MaxBufferSize) {
					_u16ResponseBuffer[i] = word(u8ModbusADU[2 * i + 4], u8ModbusADU[2 * i + 3]);
					}
					_u8ResponseBufferLength = i;
				}
				
				// in the event of an odd number of bytes, load last byte into zero-padded word
				if (u8ModbusADU[2] % 2) {
					if (i < ku8MaxBufferSize) {
					_u16ResponseBuffer[i] = word(0, u8ModbusADU[2 * i + 3]);
					}
					_u8ResponseBufferLength = i + 1;
				}
				break;
			
			case ku8MBReadInputRegisters:
			case ku8MBReadHoldingRegisters:
			case ku8MBReadWriteMultipleRegisters:
				// load bytes into word; response bytes are ordered H, L, H, L, ...
				for (i = 0; i < (u8ModbusADU[2] >> 1); i++) {
					if (i < ku8MaxBufferSize) {
						_u16ResponseBuffer[i] = word(u8ModbusADU[2 * i + 3], u8ModbusADU[2 * i + 4]);
					}
					_u8ResponseBufferLength = i;
				}
				break;
		}
	}
  
	_u8TransmitBufferIndex = 0;
	u16TransmitBufferLength = 0;
	_u8ResponseBufferIndex = 0;
	
	return u8MBStatus;
}
