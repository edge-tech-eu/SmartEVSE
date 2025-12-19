#include <Arduino.h>
#include "Board.h"
#include "ModBus.h"


#ifdef ARDUINO_ARCH_RP2040

	// use UART1 / Serial2
	#define SERIAL_RS485	Serial2
	
	void Modbus::begin(uint16_t u16BaudRate) {

		_u8TransmitBufferIndex = 0;
		u16TransmitBufferLength = 0;

		SERIAL_RS485.setTX(TXD1);
		SERIAL_RS485.setRX(RXD1);
		SERIAL_RS485.begin(u16BaudRate);
	}

#elif ARDUINO_ESP32S3_DEV

	#ifdef T_Panel_V1_2_RS485

		// LillyGo T-Panel RS485
		#include <HardwareSerial.h>
		#include "pin_config.h"
		#include "XL95x5_Driver.h"
		
		// modbus write enable is mapped on an i2c extender
		XL95x5 Class_XL95x5(XL95x5_IIC_ADDRESS, XL95x5_SDA, XL95x5_SCL);

		HardwareSerial SerialPort(2);

		#define SERIAL_RS485	SerialPort

		void Modbus::begin(uint16_t u16BaudRate) {

				_u8TransmitBufferIndex = 0;
				u16TransmitBufferLength = 0;

				SerialPort.begin(u16BaudRate, SERIAL_8N1, RS485_RX, RS485_TX);
				
				Class_XL95x5.begin();
				Class_XL95x5.read_all_reg(); // Read all registers

				Class_XL95x5.portMode(XL95x5_PORT_0, OUTPUT); // Configure the XL95x5 full port mode
				Class_XL95x5.portMode(XL95x5_PORT_1, OUTPUT);

				Class_XL95x5.digitalWrite(XL95X5_RS485_CON, LOW);
			
			}

	#else

		// This works for the Waveshare Industrial ESP32S3 Control board
		#define TXD1              17
		#define RXD1              18
		#define TXD1EN            21

		HardwareSerial MBSerial(1);

		#define SERIAL_RS485	MBSerial	

		void Modbus::begin(uint16_t u16BaudRate) {

			_u8TransmitBufferIndex = 0;
			u16TransmitBufferLength = 0;
			
			MBSerial.begin(u16BaudRate, SERIAL_8N1, RXD1, TXD1);

			if (!MBSerial.setPins(-1, -1, -1, TXD1EN)) {
				DEBUG_PRINTF("Failed to set TXDEN pins\r\n");
			}

			if (!MBSerial.setMode(UART_MODE_RS485_HALF_DUPLEX)) {
				DEBUG_PRINTF("Failed to set RS485 mode\r\n");
			}
		}
	#endif

#else

	#error ModBus.cpp needs to be adapted to architecture and rs485 interface, please check/add your combination

#endif


uint8_t u8ModbusADU[256];
char dumpBuffer[10];


uint16_t word(uint8_t high, uint8_t low) {
	uint16_t ret_val = low;
	ret_val += (high << 8);
	return ret_val;
}


uint16_t _crc16_update(uint16_t crc, uint8_t a) {
	int i;

	crc ^= a;
	for (i = 0; i < 8; ++i) {
		if (crc & 1)
			crc = (crc >> 1) ^ 0xA001;
		else
			crc = (crc >> 1);
	}
	return crc;
}


Modbus::Modbus(void) {
}


void Modbus::begin(void) {
	begin(9600);
}


void Modbus::beginTransmission(uint16_t u16Address) {
	_u16WriteAddress = u16Address;
	_u8TransmitBufferIndex = 0;
	u16TransmitBufferLength = 0;
}


void Modbus::sendBit(bool data) {
	uint8_t txBitIndex = u16TransmitBufferLength % 16;
	if ((u16TransmitBufferLength >> 4) < ku8MaxBufferSize) {
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
	send(word(0x00, data));  //MSB = 0, LSB = data
}


uint8_t Modbus::available(void) {
	return _u8ResponseBufferLength - _u8ResponseBufferIndex;
}


uint16_t Modbus::receive(void) {
	if (_u8ResponseBufferIndex < _u8ResponseBufferLength) {
		return _u16ResponseBuffer[_u8ResponseBufferIndex++];
	} else {
		return 0xFFFF;
	}
}


uint16_t Modbus::getResponseBuffer(uint8_t u8Index) {
	if (u8Index < ku8MaxBufferSize) {
		return _u16ResponseBuffer[u8Index];
	} else {
		return 0xFFFF;
	}
}


void Modbus::clearResponseBuffer() {
	uint8_t i;

	for (i = 0; i < ku8MaxBufferSize; i++) {
		_u16ResponseBuffer[i] = 0;
	}
}


uint8_t Modbus::setTransmitBuffer(uint8_t u8Index, uint16_t u16Value) {
	if (u8Index < ku8MaxBufferSize) {
		_u16TransmitBuffer[u8Index] = u16Value;
		return ku8MBSuccess;
	} else {
		return ku8MBIllegalDataAddress;
	}
}


void Modbus::clearTransmitBuffer() {
	uint8_t i;

	for (i = 0; i < ku8MaxBufferSize; i++) {
		_u16TransmitBuffer[i] = 0;
	}
}


uint8_t Modbus::readCoils(uint8_t u8MBSlave, uint16_t u16ReadAddress, uint16_t u16BitQty) {
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16BitQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBReadCoils);
}


uint8_t Modbus::readDiscreteInputs(uint8_t u8MBSlave, uint16_t u16ReadAddress,
                                   uint16_t u16BitQty) {
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16BitQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBReadDiscreteInputs);
}


uint8_t Modbus::readHoldingRegisters(uint8_t u8MBSlave, uint16_t u16ReadAddress,
                                     uint16_t u16ReadQty) {
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16ReadQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBReadHoldingRegisters);
}


uint8_t Modbus::readInputRegisters(uint8_t u8MBSlave, uint16_t u16ReadAddress,
                                   uint8_t u16ReadQty) {
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16ReadQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBReadInputRegisters);
}


uint8_t Modbus::writeSingleCoil(uint8_t u8MBSlave, uint16_t u16WriteAddress, uint8_t u8State) {
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = (u8State ? 0xFF00 : 0x0000);
	return ModbusMasterTransaction(u8MBSlave, ku8MBWriteSingleCoil);
}


uint8_t Modbus::writeSingleRegister(uint8_t u8MBSlave, uint16_t u16WriteAddress,
                                    uint16_t u16WriteValue) {
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = 0;
	_u16TransmitBuffer[0] = u16WriteValue;
	return ModbusMasterTransaction(u8MBSlave, ku8MBWriteSingleRegister);
}


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


uint8_t Modbus::writeMultipleRegisters(uint8_t u8MBSlave, uint16_t u16WriteAddress,
                                       uint16_t u16WriteQty) {
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = u16WriteQty;
	return ModbusMasterTransaction(u8MBSlave, ku8MBWriteMultipleRegisters);
}


uint8_t Modbus::writeMultipleRegisters(uint8_t u8MBSlave) {
	_u16WriteQty = _u8TransmitBufferIndex;
	return ModbusMasterTransaction(u8MBSlave, ku8MBWriteMultipleRegisters);
}


uint8_t Modbus::maskWriteRegister(uint8_t u8MBSlave, uint16_t u16WriteAddress,
                                  uint16_t u16AndMask, uint16_t u16OrMask) {
	_u16WriteAddress = u16WriteAddress;
	_u16TransmitBuffer[0] = u16AndMask;
	_u16TransmitBuffer[1] = u16OrMask;
	return ModbusMasterTransaction(u8MBSlave, ku8MBMaskWriteRegister);
}


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


uint8_t Modbus::ModbusMasterTransaction(uint8_t u8MBSlave, uint8_t u8MBFunction) {
	uint8_t u8ModbusADUSize = 0;
	uint8_t i, u8Qty;
	uint16_t u16CRC;
	uint8_t u8MBStatus = ku8MBSuccess;

	// assemble Modbus Request Application Data Unit
	u8ModbusADU[u8ModbusADUSize++] = u8MBSlave;
	u8ModbusADU[u8ModbusADUSize++] = u8MBFunction;

	switch (u8MBFunction) {
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

	switch (u8MBFunction) {
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

	switch (u8MBFunction) {
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
				switch (i % 2) {
					case 0:  // i is even
						u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[i >> 1]);
						break;

					case 1:  // i is odd
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


	int extranuousBytesRead;

	// clear reading bufffer
	SERIAL_RS485.setTimeout(4);
	do {
		extranuousBytesRead = SERIAL_RS485.readBytes(dumpBuffer, 10);
	} while (extranuousBytesRead == 10);

	// Transmit request
	SERIAL_RS485.write(u8ModbusADU, u8ModbusADUSize);

	u8ModbusADUSize = 0;

	// Wait for transmission to get completed
	SERIAL_RS485.flush();

	// read 1 byte, needs to slave address otherwise assume noise
	SERIAL_RS485.setTimeout(40);
	int maxReads = 5;
	u8ModbusADU[0] = 0;
	do {
		u8ModbusADUSize = SERIAL_RS485.readBytes((char *)u8ModbusADU, 1);
		maxReads--;
	} while ((u8ModbusADU[0] != u8MBSlave) && (maxReads > 0));

	// Read 5 bytes function code, 1+1.5 ms each = 12.5ms
	SERIAL_RS485.setTimeout(20);
	u8ModbusADUSize += SERIAL_RS485.readBytes((char *)(&u8ModbusADU[1]), 4);

	if (u8ModbusADUSize != 5) {
		u8MBStatus = ku8MBResponseTimedOut;
		DEBUG_PRINTF("mb: read adu failed, size %d != 5\r\n", u8ModbusADUSize);
	} else {
		if (u8ModbusADU[0] != u8MBSlave) {
			u8MBStatus = ku8MBInvalidSlaveID;
			DEBUG_PRINTF("mb: invalid slave id\r\n");
		}

		if ((u8ModbusADU[1] & 0x7F) != u8MBFunction) {
			u8MBStatus = ku8MBInvalidFunction;
			DEBUG_PRINTF("mb: invalid function\r\n");
		}

		if (bitRead(u8ModbusADU[1], 7)) {
			u8MBStatus = u8ModbusADU[2];
			DEBUG_PRINTF("mb: exception %d\r\n", (int)u8MBStatus);
		}
	}

	// Read data if all good so far
	if (u8MBStatus == ku8MBSuccess) {

		int u8BytesLeft = 0;

		// Determine bytes to be read
		switch (u8ModbusADU[1]) {
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
		SERIAL_RS485.setTimeout(u8BytesLeft * 2);
		int bytesRead = SERIAL_RS485.readBytes((char *)(u8ModbusADU + 5), u8BytesLeft);

		u8ModbusADUSize += bytesRead;

		if (bytesRead != u8BytesLeft) {

			u8MBStatus = ku8MBResponseTimedOut;
			DEBUG_PRINTF("mb: data read %d != %d\r\n", bytesRead, u8BytesLeft);

		} else {

			// calculate CRC
			u16CRC = 0xFFFF;
			for (i = 0; i < (u8ModbusADUSize - 2); i++) {
				u16CRC = _crc16_update(u16CRC, u8ModbusADU[i]);
			}

			// verify CRC
			if (lowByte(u16CRC) != u8ModbusADU[u8ModbusADUSize - 2] || highByte(u16CRC) != u8ModbusADU[u8ModbusADUSize - 1]) {
				u8MBStatus = ku8MBInvalidCRC;
				DEBUG_PRINTF("mb: invalid crc\r\n");
			}
		}
	}

	// disassemble ADU into words
	if (!u8MBStatus) {
		// evaluate returned Modbus function code
		switch (u8ModbusADU[1]) {
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
