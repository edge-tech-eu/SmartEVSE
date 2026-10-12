/*
  This is example code to show how to read out your DSMR 5 smart meter.
  The code is for a pico controller which can invert the UART RX and directly
  read from the meter without any hardware (just a "telephone"/RJ12 cable)

  This example runs on a WizNet W55RP20 (which had ethernet). But you should be
  able to use it on any pico

  The output of the DSMR 5 reader is the current draw of your house per phase.
  This can now be used to load balance your EVSE. See the other examples to see
  how to control our EVSE

  Board used:
  - W55RP2.0-EVB-Pico

  Documentation:
  - https://docs.wiznet.io/Product/Chip/MCU/W55RP20/w55rp20-evb-pico
  - https://github.com/earlephilhower/arduino-pico
  - https://arduino-pico.readthedocs.io/en/latest/
  - https://docs.wiznet.io/Product/Chip/MCU/W55RP20/w55rp20-evb-pico#hardware-specification

  Contact us if you help controlling our evse: https://edgetech.eu/

    Edge Technologies  

*/
#include <Arduino.h>
#include <W55RP20lwIP.h>
#include "DSMR5Reader.h"

void setup() {
  
  Serial.begin(115200);
  delay(10000);

  Serial.printf("\r\n\r\nTesting WIZnet W55RP20 EVB\r\nTesting SmartMeter reading...\r\n\r\n");

  dsmr5reader_init();

  if(dsmr5reader_check() != DSMR_OK) {

    Serial.printf("No compatible meter found!\r\n");

    while(1) {}
  }
}

void loop() {
  
  if(dsmr5reader_process_line()) {

    Serial.printf("Current: %f, %f, %f\r\n",dsmr_current[0],dsmr_current[1],dsmr_current[2]);
  }
}