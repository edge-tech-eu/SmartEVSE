/*

  M5Stack M5CoreS3 SE + COMMU

  Board: M5Stack: M5CoreS3

  Connections:
    Charger:              COMMU:   
    ModBud_Ground/white   G (pin 4)
    ModBud_A/yellow       A (pin 2)
    ModBud_B/green        B (pin 1)

  Ext port M5CoreS3:            DSMR:
    Pin 1: SDA/pulse/GPIO1/TX - N.C.
    Pin 2: SCL/edge/GPIO2/RX  - Data, 4K7 resistor to 5V
    Pin 3: 5V                 - N.C.
    Pin 4: GND                - Power ground + Signal ground
                              - Data request tie to 5V

  RJ12 6P6C plug:

             //////
            //////
       +-------------+
      /             /|
     /             / |
    /6 5 4 3 2 1  /  |
   +-+-+-+-+-+-+-+   +
   | | | | | | | |  /
   |             | /
   |   +---+     |/
   +---+   +-----+
       +---+
  
    Pinout:
      1 - +5V
      2 - Data request (tie to +5V for data)
      3 - Data ground
      4 - N.C.
      5 - Data (pull high)
      6 - Power Ground


  Documentation:
  - M5Stack M5CoreS3 SE: https://docs.m5stack.com/en/core/M5CoreS3%20SE
  - COMMU: https://docs.m5stack.com/en/module/commu
  - API: https://docs.m5stack.com/en/arduino/m5unified/m5unified_appendix
  - schematics: https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/498/Sch_M5_CoreS3_SE_v1.0.pdf
  - schematics COMMU: https://static-cdn.m5stack.com/resource/docs/products/module/commu/commu_sch_01.webp

  Notes:
  - screen size: 320 x 240
  

  Contact us if you help controlling our evse: https://edgetech.eu/

    your friends @ EdgeTech
  
*/
#include <Arduino.h>
#include "SmartEVSE.h"
#include "DSMR5Reader.h"
#include "UI.h"

#define MAX_HOME_CURRENT 25

// the default address of the EdgeTech EVSE has modbus address 1
// be aware that when the address is still 1, the board needs a state change t respond to modbus

#define ADDRESS 1

ChargerState charger_state;

#define INTERVAL 1000
unsigned long next_time;

#define ALLOW_MISSING 3
unsigned char allow_missing_count;

int board_max_current;
int max_current;
int last_max_current;

#define PHASES 3

void ui_set_advertizing_current_callback(int advertizing_current) {

  Serial.printf("advertizing %dA\r\n", advertizing_current);
  smart_evse_set_max_current(ADDRESS, max_current, 0);
}

void ui_set_phases_callback(int phases) {

  Serial.printf("set phases to %d\r\n", phases);
}

void setup(void) {

  Serial.begin(115200);

  delay(2500);
  Serial.printf("\r\n\r\nM5Stack CoreS3 SE\r\n\r\n");
  Serial1.printf("\r\n\r\nM5Stack CoreS3 SE\r\n\r\n");

  smart_evse_init();

  allow_missing_count = ALLOW_MISSING;

  board_max_current = 16;

  if (smart_evse_is_32_amp(ADDRESS)) {
    board_max_current = 32;
  }

  last_max_current = board_max_current;
  max_current = board_max_current;

  ui_init();

  Serial.printf("Request serial and fw version:\r\n");

  int serial[5];
  int version;
  smart_evse_get_serial(ADDRESS, serial);
  smart_evse_get_fw_version(ADDRESS, &version);
  // ui_start_up(serial,version);

  Serial.printf("init done\r\n");

  dsmr5reader_init();

  int meter_status = dsmr5reader_check();
  if (meter_status != DSMR_OK) {

    Serial.printf("No compatible meter found! (code: %d)\r\n", meter_status);

    while (1) {}
  }

  Serial.printf("DSMR: %s\r\n", dsmr_id);

  delay(4000);

  ui_setup_main(board_max_current, PHASES);

  next_time = millis() + INTERVAL;
}


void loop(void) {

  unsigned long now = millis();

  if (dsmr5reader_process_line()) {

    // Serial.printf("Current: %f, %f, %f\r\n", dsmr_current[0], dsmr_current[1], dsmr_current[2]);

    ui_set_home_values(dsmr_current);
  }

  if (now > next_time) {

    next_time = now + INTERVAL;

    smart_evse_get_charger_state(ADDRESS, &charger_state);

    // calculate advertizing current
    max_current = board_max_current;
    for (int i = 0; i < 3; i++) {
      int spare = MAX_HOME_CURRENT - dsmr_current[i] + charger_state.c[i];
      if (spare < max_current) {
        max_current = spare;
      }
    }
    if(max_current < 6) {
      max_current = 0;
    }

    if (last_max_current != max_current) {

      last_max_current = max_current;

      Serial.printf("Setting advertizing current to %d\r\n", max_current);

      ui_set_advertized(max_current);

      ui_set_advertizing_current_callback(max_current);
    }

    if (charger_state.isNew) {

      allow_missing_count = ALLOW_MISSING;

      charger_state.isNew = false;

      // Serial.printf("state: %d, error: %d, temperature: %d, cable: %d a\r\n",
      //                charger_state.state, charger_state.error, charger_state.temperature, charger_state.cable_current);
      // Serial.printf("current: %f, %f, %f a\r\n", charger_state.c[0], charger_state.c[1], charger_state.c[2]);
      // Serial.printf("voltage: %f, %f, %f v\r\n", charger_state.v[0], charger_state.v[1], charger_state.v[2]);
      // Serial.printf("session: %f kwh, total: %f kwh\r\n\r\n", charger_state.kwh_session, charger_state.kwh_total);

      ui_set_ev_values(charger_state);

    } else {

      if (allow_missing_count) {
        allow_missing_count--;
        if (!allow_missing_count) {
          ui_unset_ev_values();
        }
      }
    }
  }

  ui_process();
}
