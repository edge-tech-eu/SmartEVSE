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

ChargerState charger_state;

unsigned long next_time;

void setup() {

  Serial.begin(115200);

  mb.begin(9600);

  delay(10000);

  Serial.printf("\r\n\r\nTesting Controlling Charger\r\nusing WaveShare RP2350 Touch LCD 4...\r\n\r\n");

  Serial.printf("Request serial and fw version:\r\n");

  smart_evse_get_serial(ADDRESS);

  // smart_evse_change_address(ADDRESS, NEW_ADDRESS, 0x1234);
  
  smart_evse_get_fw_version(ADDRESS);

  smart_evse_get_max_currents(ADDRESS);

  next_time = millis() + 1000;
}

void loop() {

unsigned long now = millis();

  if (now > next_time) {

    next_time += 10000;

    smart_evse_get_charger_state(ADDRESS, &charger_state);

    Serial.printf("state: %d, error: %d, temperature: %d, cable: %d a\r\n", 
      charger_state.state, charger_state.error, charger_state.temperature, charger_state.cable_current);
    Serial.printf("current: %f, %f, %f a\r\n",charger_state.c[0],charger_state.c[1],charger_state.c[2]);
    Serial.printf("voltage: %f, %f, %f v\r\n",charger_state.v[0],charger_state.v[1],charger_state.v[2]);
    Serial.printf("session: %f kwh, total: %f kwh\r\n\r\n");
  }
}
