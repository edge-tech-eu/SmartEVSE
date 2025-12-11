/*

  WaveShare RP2350 Touch LCD 4

  Board: Raspberry Pi Pico/RP2040/RP2350: WaveShare RP2350 LCD 0.96
  
    Connections:
    Charger:              WaveShare:   
    ModBud_Ground/white   N.C.
    ModBud_A/yellow       A
    ModBud_B/green        B

  Documentation:
  - https://www.waveshare.com/rp2350-lcd-4.htm
  - https://www.waveshare.com/wiki/RP2350-Touch-LCD-4
  - https://files.waveshare.com/wiki/RP2350-Touch-LCD-4/RP2350-Touch-LCD-4.pdf

  Contact us if you help controlling our evse: https://edgetech.eu/

    your friends @ EdgeTech

*/
#include <Arduino.h>
#include "Board.h"
#include "ModBus.h"
#include "SmartEVSE.h"

#define ADDRESS 1

Modbus mb = Modbus();

unsigned long next_time;

void setup() {

  Serial.begin(115200);

  mb.begin(9600);

  delay(10000);

  Serial.printf("\r\n\r\nTesting Controlling Charger\r\nusing WaveShare RP2350 Touch LCD 4...\r\n\r\n");

  Serial.printf("Request serial and fw version:\r\n");

  smart_evse_get_serial(ADDRESS);
  
  smart_evse_get_fw_version(ADDRESS);

  smart_evse_get_max_currents(ADDRESS);

  next_time = millis() + 1000;
}

void loop() {

  unsigned long now = millis();

  if (now > next_time) {

    next_time += 5000;

    smart_evse_get_state(ADDRESS);

    smart_evse_get_temperature(ADDRESS);
  }
}
