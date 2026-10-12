/*

  LilyGo T-Panel S3 RS485

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
#include <Wire.h>
#include "Arduino_GFX_Library.h"
#include "Board.h"
#include "SmartEVSE.h"


// Modbus address of the EdgeTech charge controller. It leaves the factory at address 1.
// At address 1 the charger does not answer after boot, only after a state change
// (press the button on the board, or plug in a car).
// For your own controller, give the charger address 2 or higher via register 0x0200 (and save it),
// then set ADDRESS to that value. See docs/en/modbus-registers.md

#define ADDRESS 1

// The RS485 driver-enable line is on the I/O expander. ModBus.cpp drives it through
// this bus object (the Modbus object itself, mb, lives in SmartEVSE.cpp).
Arduino_XL9535SWSPI *bus = new Arduino_XL9535SWSPI(IIC_SDA /* SDA */, IIC_SCL /* SCL */, -1 /* XL PWD */,
                                                   XL95X5_CS /* XL CS */, XL95X5_SCLK /* XL SCK */, XL95X5_MOSI /* XL MOSI */);

unsigned long next_time;

ChargerState charger_state;

void setup() {

  Serial.begin(115200);

  delay(10000);

  // start the I/O expander (on I2C) before the Modbus port uses it
  Wire.begin(IIC_SDA, IIC_SCL);
  bus->begin();

  Serial.printf("\r\n\r\nTesting Controlling Charger\r\nusing LilyGo T-Panel RS485...\r\n\r\n");

  smart_evse_init();
  charger_state.count = 0;
  charger_state.error_count = 0;

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

    if (charger_state.isNew) {

      charger_state.isNew = false;

      Serial.printf("state: %d, error: %d, temperature: %d, cable: %d a\r\n",
        charger_state.state, charger_state.error, charger_state.temperature, charger_state.cable_current);
      Serial.printf("current: %f, %f, %f a\r\n",charger_state.c[0],charger_state.c[1],charger_state.c[2]);
      Serial.printf("voltage: %f, %f, %f v\r\n",charger_state.v[0],charger_state.v[1],charger_state.v[2]);
      Serial.printf("session: %f kwh, total: %f kwh\r\n\r\n", charger_state.kwh_session, charger_state.kwh_total);
    }
  }
}