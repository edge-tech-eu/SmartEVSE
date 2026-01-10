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

// room to store the charger state (charge state, currents, voltages)
ChargerState charger_state;

// DSMR smart meter should get new values every second
#define INTERVAL 1000
unsigned long next_time;

// allow for 3x not being able to read values before nulling
#define ALLOW_MISSING 3
unsigned char allow_missing_count;

// the charger has a jumper to hard set the max current to 16a (no jumper) or 32a (jumper)
int board_max_current;

// load balancing
int max_current;
int last_max_current;

// set initial value
#define INITIAL_PHASES 3
int active_phases;

void ui_set_advertizing_current_callback(int advertizing_current) {

  Serial.printf("advertizing %dA\r\n", advertizing_current);

  smart_evse_set_max_current(ADDRESS, advertizing_current, 0);
}

void ui_set_phases_callback(int phases) {

  Serial.printf("set phases to %d\r\n", phases);

  smart_evse_set_phases(ADDRESS, phases==3);
}

void board_init_and_check() {

  bool error;
  int serial[5];
  int version;
  char line[80];

  do {

    error = false;

    ui_clear();

    ui_start_up();

    ui_start_up_add_line("\r\n- Connecting to charger: ");

    smart_evse_init();

    if (!smart_evse_get_serial(ADDRESS, serial)) {

      ui_start_up_add_line_error("not found!\r\n  check wires and address\r\n  change state if at address 1\r\n");
      board_max_current = 0;
      error = true;

    } else {

      ui_start_up_add_line("found!\r\n");
      sprintf(line, "- Serial number: %04x%04x%04x%04x%04x\r\n", serial[0], serial[1], serial[2], serial[3], serial[4]);
      smart_evse_get_fw_version(ADDRESS, &version);
      ui_start_up_add_line(line);
      sprintf(line, "- Firmware version: %d.%d.%d\r\n", version >> 12, (version >> 8) & 0x0f, version & 0xff);
      ui_start_up_add_line(line);
      board_max_current = 16;

      if (smart_evse_is_32_amp(ADDRESS)) {
        board_max_current = 32;
      }

      sprintf(line, "- Board max current: %d\r\n", board_max_current);
      ui_start_up_add_line(line);
    }

    last_max_current = board_max_current;
    max_current = board_max_current;

    dsmr5reader_init();

    int meter_status = dsmr5reader_check();
    if (meter_status != DSMR_OK) {

      sprintf(line, "- No compatible meter found! (code: %d)\r\n", meter_status);
      ui_start_up_add_line_error(line);
      error = true;
    } else {

      sprintf(line, "- DSMR: %s\r\n", dsmr_id);
      ui_start_up_add_line(line);
      if (dsmr_single_phase) {
        sprintf(line, "- Singel phase system\r\n");
      } else {
        sprintf(line, "- Three phase system\r\n");
      }
      ui_start_up_add_line(line);
    }

    ui_start_up_add_line("- Charger comm test: ");
    int errors = 0;
    for (int i = 0; i < 10; i++) {
      bool found_error = false;
      for (int j = 0; j < 10; j++) {
        if (!(smart_evse_get_serial(ADDRESS, serial) && smart_evse_get_fw_version(ADDRESS, &version))) {
          errors++;
          found_error = true;
        }
      }
      if (found_error) {
        ui_start_up_add_line_error("x");
      } else {
        ui_start_up_add_line(".");
      }
    }
    int quality = 100 - errors;
    sprintf(line, " %d%%\r\n", quality);
    if (quality < 100) {
      ui_start_up_add_line_error(line);
      error = true;
    } else {
      ui_start_up_add_line(line);
    }

    if (error) {

      ui_start_up_add_line_error("\r\nErrors: restarting...");

      ui_delay_untill_tapped(5000);
    }

  } while (error);

  ui_start_up_add_line("\r\nAll fine!");
}

void setup(void) {

  ui_init();
  ui_splash();

  Serial.begin(115200);

  Serial.printf("\r\n\r\nM5Stack CoreS3 SE\r\n\r\n");

  ui_delay_untill_tapped(5000);

  board_init_and_check();

  ui_delay_untill_tapped(30000);

  ui_clear();

  // make sure the charge is in sync
  active_phases = INITIAL_PHASES;
  ui_set_advertizing_current_callback(max_current);
  ui_set_phases_callback(active_phases);
  allow_missing_count = ALLOW_MISSING;

  ui_setup_main(board_max_current, active_phases);

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

    ui_start_update();

    smart_evse_get_charger_state(ADDRESS, &charger_state);

    // calculate advertizing current
    max_current = board_max_current;
    for (int i = 0; i < 3; i++) {
      int spare = MAX_HOME_CURRENT - dsmr_current[i] + charger_state.c[i];
      if (spare < max_current) {
        max_current = spare;
      }
    }
    if (max_current < 6) {
      max_current = 0;
    }

    if (last_max_current != max_current) {

      last_max_current = max_current;

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

    ui_end_update();
  }

  ui_process();
}
