#ifndef MODBUS_h
#define MODBUS_h

#include "Arduino.h"


#define lowWord(ww) ((uint16_t) ((ww) & 0xFFFF))
#define highWord(ww) ((uint16_t) ((ww) >> 16))
#define LONG(hi, lo) ((uint32_t) ((hi) << 16 | (lo)))


class Modbus {
public:
	Modbus();
	Modbus(uint8_t);
	
	void begin();
	void begin(uint16_t);
	
	static const uint8_t ku8MBSuccess									= 0x00;
	static const uint8_t ku8MBIllegalFunction					= 0x01;
	static const uint8_t ku8MBIllegalDataAddress		 	= 0x02;
	static const uint8_t ku8MBIllegalDataValue		   	= 0x03;
	static const uint8_t ku8MBSlaveDeviceFailure		 	= 0x04;
	static const uint8_t ku8MBInvalidSlaveID			 		= 0xE0;
	static const uint8_t ku8MBInvalidFunction					= 0xE1;
	static const uint8_t ku8MBResponseTimedOut		   	= 0xE2;
	static const uint8_t ku8MBInvalidCRC				 			= 0xE3;
	
	uint16_t getResponseBuffer(uint8_t);
	void	 clearResponseBuffer();
	uint8_t  setTransmitBuffer(uint8_t, uint16_t);
	void	 clearTransmitBuffer();
	
	void beginTransmission(uint16_t);
	uint8_t requestFrom(uint16_t, uint16_t);
	void sendBit(bool);
	void send(uint8_t);
	void send(uint16_t);
	void send(uint32_t);
	uint8_t available(void);
	uint16_t receive(void);

	void enableTXpin(uint8_t);
	void disableTXpin();
	
	uint8_t  readCoils(uint8_t, uint16_t, uint16_t);
	uint8_t  readDiscreteInputs(uint8_t, uint16_t, uint16_t);
	uint8_t  readHoldingRegisters(uint8_t, uint16_t, uint16_t);
	uint8_t  readInputRegisters(uint8_t, uint16_t, uint8_t);
	uint8_t  writeSingleCoil(uint8_t, uint16_t, uint8_t);
	uint8_t  writeSingleRegister(uint8_t, uint16_t, uint16_t);
	uint8_t  writeMultipleCoils(uint8_t, uint16_t, uint16_t);
	uint8_t  writeMultipleCoils(uint8_t);
	uint8_t  writeMultipleRegisters(uint8_t, uint16_t, uint16_t);
	uint8_t  writeMultipleRegisters(uint8_t);
	uint8_t  maskWriteRegister(uint8_t, uint16_t, uint16_t, uint16_t);
	uint8_t  readWriteMultipleRegisters(uint8_t, uint16_t, uint16_t, uint16_t, uint16_t);
	uint8_t  readWriteMultipleRegisters(uint8_t, uint16_t, uint16_t);
	
private:
	uint8_t  _u8SerialPort;									  ///< serial port (0..3) initialized in constructor
	uint16_t _u16BaudRate;									   ///< baud rate (300..115200) initialized in begin()
	static const uint8_t ku8MaxBufferSize				= 64;   ///< size of response/transmit buffers	
	uint16_t _u16ReadAddress;									///< slave register from which to read
	uint16_t _u16ReadQty;										///< quantity of words to read
	uint16_t _u16ResponseBuffer[ku8MaxBufferSize];			   ///< buffer to store Modbus slave response; read via GetResponseBuffer()
	uint16_t _u16WriteAddress;								   ///< slave register to which to write
	uint16_t _u16WriteQty;									   ///< quantity of words to write
	uint16_t _u16TransmitBuffer[ku8MaxBufferSize];			   ///< buffer containing data to transmit to Modbus slave; set via SetTransmitBuffer()
	//uint16_t* txBuffer; // from Wire.h -- need to clean this up Rx
	uint8_t _u8TransmitBufferIndex;
	uint16_t u16TransmitBufferLength;
	//uint16_t* rxBuffer; // from Wire.h -- need to clean this up Rx
	uint8_t _u8ResponseBufferIndex;
	uint8_t _u8ResponseBufferLength;
	
	// Modbus function codes for bit access
	static const uint8_t ku8MBReadCoils				  = 0x01; ///< Modbus function 0x01 Read Coils
	static const uint8_t ku8MBReadDiscreteInputs		 = 0x02; ///< Modbus function 0x02 Read Discrete Inputs
	static const uint8_t ku8MBWriteSingleCoil			= 0x05; ///< Modbus function 0x05 Write Single Coil
	static const uint8_t ku8MBWriteMultipleCoils		 = 0x0F; ///< Modbus function 0x0F Write Multiple Coils

	// Modbus function codes for 16 bit access
	static const uint8_t ku8MBReadHoldingRegisters	   = 0x03; ///< Modbus function 0x03 Read Holding Registers
	static const uint8_t ku8MBReadInputRegisters		 = 0x04; ///< Modbus function 0x04 Read Input Registers
	static const uint8_t ku8MBWriteSingleRegister		= 0x06; ///< Modbus function 0x06 Write Single Register
	static const uint8_t ku8MBWriteMultipleRegisters	 = 0x10; ///< Modbus function 0x10 Write Multiple Registers
	static const uint8_t ku8MBMaskWriteRegister		  = 0x16; ///< Modbus function 0x16 Mask Write Register
	static const uint8_t ku8MBReadWriteMultipleRegisters = 0x17; ///< Modbus function 0x17 Read Write Multiple Registers
	
	// Modbus timeout [milliseconds]
	static const uint8_t ku8MBResponseTimeout			= 200;  ///< Modbus timeout [milliseconds]
	
	// master function that conducts Modbus transactions
	uint8_t ModbusMasterTransaction(uint8_t u8MBSlave, uint8_t u8MBFunction);
};
#endif
