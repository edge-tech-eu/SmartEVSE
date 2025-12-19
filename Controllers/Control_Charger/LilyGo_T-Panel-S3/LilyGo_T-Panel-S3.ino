/*

  LillyGo T-Panel S3 RS485

  Board: esp32: ESP32S3 Dev Module
  - USB CDC On Boot	  Enabled
  - Flash Size	      16MB (128Mb)
  - Partition Scheme	16M Flash (3MB APP/9.9MB FATFS)
  - PSRAM	            QSPI PSRAM

  Connections:
    Charger:              T-Panel:   
    ModBud_Ground/white   SGND
    ModBud_A/yellow       L
    ModBud_B/green        H

  Documentation:
  - https://lilygo.cc/products/t-panel-s3?srsltid=AfmBOoqK3WmcNyMwsreH9n6bwDtGik-VFqjp-NglJnTqaYiQIHH78q6t
  - https://github.com/Xinyuan-LilyGO/T-Panel
  - https://github.com/Xinyuan-LilyGO/T-Panel/blob/arduino-esp32-libs_V2.0.14/project/T-Panel_V1.2.pdf


  Contact us if you help controlling our evse: https://edgetech.eu/

    your friends @ EdgeTech

*/
#include <Arduino.h>
#include "Board.h"
#include "ModBus.h"
#include "SmartEVSE.h"


// the default address of the EdgeTech EVSE has modbus address 1
// be aware that when the address is still 1, the board needs a state change t respond to modbus

#define ADDRESS 1

Modbus mb = Modbus();

unsigned long next_time;

// no not forget to press the boot button

ChargerState charger_state;

void setup() {

  Serial.begin(115200);
  mb.begin(9600);

  delay(10000);

  Serial.printf("\r\n\r\nTesting Controlling Charger\r\nusing LilyGo T-Panel RS485...\r\n\r\n");

  Serial.printf("Request serial and fw version:\r\n");

  smart_evse_get_serial(ADDRESS);

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