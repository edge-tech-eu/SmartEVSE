/*

  Board: ESP32 S3 Relay 1CH
  - ESP32S3 Dev Module
  - Flash Size: 8MB (64Mb)
  - Partition Scheme: 8M with spiffs (3M APP/1.5MB SPIFFS)
  
  Documentation:
  - https://www.waveshare.com/esp32-s3-rs485-can.htm
  - https://files.waveshare.com/wiki/ESP32-S3-Relay-1CH/ESP32-S3-Relay-1CH-schematic.pdf
  - https://www.waveshare.com/wiki/ESP32-S3-Relay-1CH-RS485

  Contact us if you help controlling our evse: https://edgetech.eu/

    your friends @ EdgeTech

*/
#include <HardwareSerial.h>
#include "Arduino.h"
#include "ModBus.h"

Modbus mb = Modbus();

unsigned long next_time;

void setup() {

  printf("\r\n\r\nBoard: ESP32 S3 Relay 1CH\r\nTesting ModBus\r\n");

  // modbus serial, mode and pins hard-coded
  mb.begin(9600);

  next_time = millis() + 1000;
}

void loop() {
  
  unsigned long now = millis();

  if(now > next_time) {
    
    next_time += 5000;

    printf("Request serial and fw version\r\n");

    if (mb.readInputRegisters(1, 0, 6) == mb.ku8MBSuccess) {
      
      printf("serial: %4x%4x%4x%4x%4x, fw version: %4x\r\n",
        mb.getResponseBuffer(0),
        mb.getResponseBuffer(1),
        mb.getResponseBuffer(2),
        mb.getResponseBuffer(3),
        mb.getResponseBuffer(4),
        mb.getResponseBuffer(5));

    } else {

      printf("failed to read over mb");
    }
  }

}