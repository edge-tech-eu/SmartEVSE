/*

  M5Stack M5CoreS3 SE + COMMU

  Board: M5Stack: M5CoreS3

  Connections:
    Charger:              COMMU:   
    ModBud_Ground/white   G (pin 4)
    ModBud_A/yellow       A (pin 2)
    ModBud_B/green        B (pin 1)
  
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
#include "UI.h"

// Modbus address of the EdgeTech charge controller. It leaves the factory at address 1.
// At address 1 the charger does not answer after boot, only after a state change
// (press the button on the board, or plug in a car).
// For your own controller, give the charger address 2 or higher via register 0x0200 (and save it),
// then set ADDRESS to that value. See docs/en/modbus-registers.md

#define ADDRESS 1

ChargerState charger_state;

#define INTERVAL  1000
unsigned long next_time;

#define ALLOW_MISSING   3
unsigned char allow_missing_count;

#define PHASES  3

// current limit chosen with the slider (A, 0 = stop). It is written to register 0x0201
// on every change and again every INTERVAL: the charger needs a write at least once a second.
int requested_current = 0;
int active_phases = PHASES;

// keep writing 0 A for a while, so the charger does not fall back to its static limit
void hold_zero_current(unsigned long ms) {

  unsigned long until = millis() + ms;

  do {
    smart_evse_set_max_current(ADDRESS, 0, 0);
    delay(500);
  } while ((long)(millis() - until) < 0);
}

void ui_set_advertizing_current_callback(int advertizing_current) {

  Serial.printf("advertizing %dA\r\n", advertizing_current);

  requested_current = advertizing_current;

  // 0x0201 low byte: RemoteMaxCurrent in A
  smart_evse_set_max_current(ADDRESS, requested_current, 0);
}

void ui_set_phases_callback(int phases) {

  if (phases == active_phases) {
    return;
  }

  Serial.printf("set phases to %d\r\n", phases);

  // stop charging before switching phases
  if (requested_current > 0) {
    Serial.printf("stopping charge (advertizing 0 A)\r\n");
    hold_zero_current(3000);
  }

  // 0x0204 bit 0, read-modify-write keeps the other settings bits
  if (smart_evse_set_phases(ADDRESS, phases == 3)) {
    active_phases = phases;
  } else {
    Serial.printf("could not change phases\r\n");
  }

  if (requested_current > 0) {
    hold_zero_current(500);
    Serial.printf("restoring advertizing current to %d A\r\n", requested_current);
    smart_evse_set_max_current(ADDRESS, requested_current, 0);
  }
}

void setup(void) {

  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 2, 1);
  delay(2500);
  Serial.printf("\r\n\r\nM5Stack CoreS3 SE\r\n\r\n");
  Serial1.printf("\r\n\r\nM5Stack CoreS3 SE\r\n\r\n");

  smart_evse_init();

  allow_missing_count = ALLOW_MISSING;

  int board_max_current = 16;

  if (smart_evse_is_32_amp(ADDRESS)) {
    board_max_current = 32;
  }

  ui_init(board_max_current, PHASES);

  Serial.printf("init done\r\n");

  Serial.printf("Request serial and fw version:\r\n");

  smart_evse_get_serial(ADDRESS);

  smart_evse_get_fw_version(ADDRESS);

  smart_evse_get_max_currents(ADDRESS);

  next_time = millis() + INTERVAL;
}

void loop(void) {

  unsigned long now = millis();

  if (now > next_time) {

    next_time = now + INTERVAL;

    Serial1.printf(".");

    // refresh the current limit, at least once per second
    smart_evse_set_max_current(ADDRESS, requested_current, 0);

    smart_evse_get_charger_state(ADDRESS, &charger_state);

    if (charger_state.isNew) {

      allow_missing_count = ALLOW_MISSING;

      charger_state.isNew = false;

      Serial.printf("state: %d, error: %d, temperature: %d, cable: %d a\r\n",
                    charger_state.state, charger_state.error, charger_state.temperature, charger_state.cable_current);
      Serial.printf("current: %f, %f, %f a\r\n", charger_state.c[0], charger_state.c[1], charger_state.c[2]);
      Serial.printf("voltage: %f, %f, %f v\r\n", charger_state.v[0], charger_state.v[1], charger_state.v[2]);
      Serial.printf("session: %f kwh, total: %f kwh\r\n\r\n", charger_state.kwh_session, charger_state.kwh_total);

      ui_set_ev_values(charger_state);

    } else {

      if(allow_missing_count) {
        allow_missing_count--;
        if(!allow_missing_count) {
          ui_unset_ev_values();
        }
      }
    }
  }

  ui_process();
}
