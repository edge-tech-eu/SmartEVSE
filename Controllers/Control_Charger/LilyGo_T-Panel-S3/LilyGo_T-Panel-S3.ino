 // USB CDC On Boot	- Enabled
// Flash Size	- 16MB (128Mb)
// Partition Scheme	- 16M Flash (3MB APP/9.9MB FATFS)
// PSRAM	QSPI PSRAM

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include "pin_config.h"
#include "XL95x5_Driver.h"
#include "DebugPrintf.h"
#include "ModBus.h"
#include "SmartEVSE.h"

XL95x5 Class_XL95x5(XL95x5_IIC_ADDRESS, XL95x5_SDA, XL95x5_SCL);

// the default address of the EdgeTech EVSE has modbus address 1
// be aware that when the address is still 1, the board needs a state change t respond to modbus

#define ADDRESS 1

Modbus mb = Modbus();

unsigned long next_time;

// no not forget to press the boot button

/*
static uint32_t temp = 0;

// XL95x5 initializes the configuration


HardwareSerial SerialPort(2); // use UART2
char DATA1 = 0;
*/

// XL95x5 Class_XL95x5(XL95x5_IIC_ADDRESS, XL95x5_SDA, XL95x5_SCL);
HardwareSerial SerialPort(2); // use UART2
//extern HardwareSerial SerialPort;
void setup() {
    
    Serial.begin(9600);

    delay(10000);

    DEBUG_PRINTF("\r\n\r\n\r\nCiallo100\r\n");

    SerialPort.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
    //Serial2.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);

    Class_XL95x5.begin();
    Class_XL95x5.read_all_reg(); // Read all registers

    Class_XL95x5.portMode(XL95x5_PORT_0, OUTPUT); // Configure the XL95x5 full port mode
    Class_XL95x5.portMode(XL95x5_PORT_1, OUTPUT);

    Class_XL95x5.digitalWrite(XL95X5_RS485_CON, LOW);

    DEBUG_PRINTF("Request serial and fw version:\r\n");

  smart_evse_get_serial(ADDRESS);
  
  smart_evse_get_fw_version(ADDRESS);

  smart_evse_get_max_currents(ADDRESS);

  next_time = millis() + 1000;
}

void loop() {

    unsigned long now = millis();

  if(now > next_time) {
    
    next_time += 5000;
/*
    Class_XL95x5.digitalWrite(XL95X5_RS485_CON, HIGH);
    
    SerialPort.printf("hallo");
    SerialPort.flush();
    Class_XL95x5.digitalWrite(XL95X5_RS485_CON, LOW);
    */
    
    smart_evse_get_state(ADDRESS);

    smart_evse_get_temperature(ADDRESS);
    
  }

    /*
    Class_XL95x5.digitalWrite(XL95X5_RS485_CON, HIGH);
    temp++;
    SerialPort.printf("%d", temp);

    delay(1000);

    while (SerialPort.available() > 0)
    {
        Class_XL95x5.digitalWrite(XL95X5_RS485_CON, LOW);
        DATA1 = SerialPort.read();
        Serial.print(DATA1);
    }
    delay(1000);
    */
}