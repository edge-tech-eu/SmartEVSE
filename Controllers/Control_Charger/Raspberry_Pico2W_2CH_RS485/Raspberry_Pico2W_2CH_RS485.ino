/*

  Raspberry Pi Pico 2W + WaveShare 2-Channel RS485 Module for Pico

  Board: Raspberry Pi Pico/RP2040/RP2350: Raspberry Pi Pico 2W
  
    Connections:
    Charger:              WaveShare:   
    ModBud_Ground/white   Channel 1: G
    ModBud_A/yellow       Channel 1: A
    ModBud_B/green        Channel 1: B

  Documentation:
  - https://www.waveshare.com/pico-2ch-rs485.htm
  - https://github.com/edge-tech-eu/SmartEVSE/blob/main/ModBusRegisters.pdf
  - https://arduino-pico.readthedocs.io/en/latest/

  Contact us if you help controlling our evse: https://edgetech.eu/

    your friends @ EdgeTech

*/
#include <Arduino.h>
#include "Board.h"
#include "SmartEVSE.h"

// the default address of the EdgeTech EVSE has modbus address 1
// be aware that when the address is still 1, the board needs a state change t respond to modbus

#define ADDRESS 1

ChargerState charger_state;

unsigned long next_time;


void setup() {

  Serial.begin(115200);
  
  delay(10000);

  Serial.printf("\r\n\r\nTesting Controlling Charger\r\nusing WaveShare 2 CH RS485...\r\n\r\n");

  Serial.printf("Request serial and fw version:\r\n");

  smart_evse_init();

  smart_evse_get_serial(ADDRESS);
  
  smart_evse_get_fw_version(ADDRESS);

  smart_evse_get_max_currents(ADDRESS);

  next_time = millis() + 1000;
}

void loop() {
  
  unsigned long now = millis();

  if(now > next_time) {
    
    next_time += 5000;

    smart_evse_get_charger_state(ADDRESS, &charger_state);

    if (charger_state.isNew) {

      charger_state.isNew = false;

      Serial.printf("state: %d, error: %d, temperature: %d, cable: %d a\r\n",
                    charger_state.state, charger_state.error, charger_state.temperature, charger_state.cable_current);
      Serial.printf("current: %f, %f, %f a\r\n", charger_state.c[0], charger_state.c[1], charger_state.c[2]);
      Serial.printf("voltage: %f, %f, %f v\r\n", charger_state.v[0], charger_state.v[1], charger_state.v[2]);
      Serial.printf("session: %f kwh, total: %f kwh\r\n\r\n",charger_state.kwh_session, charger_state.kwh_total);
    }
  }
}
