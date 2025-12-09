/*

  Board: Raspberry Pi Pico/RP2040/RP2350: Raspberry Pi Pico 2W
  - with WaveShare 2-Channel RS485 Module for Pico 

  Documentation:
  - https://www.waveshare.com/pico-2ch-rs485.htm
  - https://github.com/edge-tech-eu/SmartEVSE/blob/main/ModBusRegisters.pdf
  - https://arduino-pico.readthedocs.io/en/latest/

  Contact us if you help controlling our evse: https://edgetech.eu/

    your friends @ EdgeTech

*/
#include <Arduino.h>
#include "ModBus.h"
#include "SmartEVSE.h"

// the default address of the EdgeTech EVSE has modbus address 1
// be aware that when the address is still 1, the board needs a state change t respond to modbus

#define ADDRESS 1

Modbus mb = Modbus();

unsigned long next_time;

void setup() {

  Serial.begin(115200);
  
  mb.begin(9600);

  delay(10000);

  Serial.printf("\r\n\r\nTesting Controlling Charger\r\nusing WaveShare 2 CH Rs495...\r\n\r\n");

  Serial.printf("Request serial and fw version:\r\n");

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
