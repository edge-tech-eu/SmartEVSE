/*
  This is example code to show how to read out your DSMR 5 smart meter.
  The code is for a pico controller which can invert the UART RX, so no inverter
  is needed: an RJ12 ("telephone") cable and one resistor are enough.

  Wiring (P1 = RJ12 6P6C):
    GP1 (UART0 RX)  - P1 pin 5, data. 4K7 pull-up resistor to 3.3V (3V3 pin)
    GND             - P1 pin 3 (data ground) and pin 6 (power ground)
    P1 pin 1 (+5V)  - P1 pin 2, data request (so the meter sends data). Pin 1
                      supplies at most 250 mA; above that, use an external 5V supply
  The data output of the meter is open collector, so the pull-up sets the logic
  level. Never pull the data line up to 5V: the Pico pins are 3.3V.

  This example runs on a WizNet W55RP20 (which had ethernet). But you should be
  able to use it on any pico

  The output of the DSMR 5 reader is the current draw of your house per phase.
  This can now be used to load balance your EVSE. See the other examples to see
  how to control the EdgeTech charge controller

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

  int meter_status = dsmr5reader_check();

  if(meter_status != DSMR_OK) {

    Serial.printf("No compatible meter found! (code: %d)\r\n", meter_status);

    while(1) {}
  }

  Serial.printf("DSMR 5 meter found (%s phase)\r\n", dsmr_single_phase ? "single" : "three");
}

void loop() {
  
  if(dsmr5reader_process_line()) {

    Serial.printf("Current: %f, %f, %f\r\n",dsmr_current[0],dsmr_current[1],dsmr_current[2]);
  }
}