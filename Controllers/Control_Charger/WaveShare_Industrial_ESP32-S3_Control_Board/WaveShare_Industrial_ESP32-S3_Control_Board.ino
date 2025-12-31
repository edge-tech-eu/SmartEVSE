/*

  WaveShare Industrial ESP32S3 Controller Board

  Board: esp32: ESP32S3 Dev Module
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
#include "Board.h"
#include "SmartEVSE.h"

// the default address of the EdgeTech EVSE has modbus address 1
// be aware that when the address is still 1, the board needs a state change t respond to modbus

#define ADDRESS 1

ChargerState charger_state;

unsigned long next_time;


void setup() {

  DEBUG_PRINTF("\r\n\r\nBoard: ESP32 S3 Relay 1CH\r\nTesting ModBus\r\n");

  delay(2500);

  DEBUG_PRINTF("Request serial and fw version:\r\n");

  smart_evse_init();

  smart_evse_get_serial(ADDRESS);
  
  smart_evse_get_fw_version(ADDRESS);

  smart_evse_get_max_currents(ADDRESS);

  next_time = millis() + 1000;
}


void loop() {
  
  unsigned long now = millis();

  next_time = now + 5000;
    
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