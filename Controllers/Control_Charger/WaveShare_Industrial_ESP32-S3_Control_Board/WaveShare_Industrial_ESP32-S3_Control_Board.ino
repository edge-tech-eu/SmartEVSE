/*

  Board: esp32: ESP32 S3 Relay 1CH
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
#include <Arduino.h>
#include "DebugPrintf.h"
#include "ModBus.h"
#include "SmartEVSE.h"


// the default address of the EdgeTech EVSE has modbus address 1
// be aware that when the address is still 1, the board needs a state change t respond to modbus

#define ADDRESS 1

Modbus mb = Modbus();

unsigned long next_time;

void setup() {

  DEBUG_PRINTF("\r\n\r\nBoard: ESP32 S3 Relay 1CH\r\nTesting ModBus\r\n");

  // modbus serial, mode and pins hard-coded
  mb.begin(9600);

  delay(2500);

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

    smart_evse_get_state(ADDRESS);

    smart_evse_get_temperature(ADDRESS);
  }
}