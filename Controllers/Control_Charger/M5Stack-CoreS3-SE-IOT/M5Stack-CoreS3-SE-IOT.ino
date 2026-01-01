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

// the default address of the EdgeTech EVSE has modbus address 1
// be aware that when the address is still 1, the board needs a state change t respond to modbus

#define ADDRESS 1

ChargerState charger_state;

unsigned long next_time;


void ui_set_advertizing_current_callback(int advertizing_current) {

    Serial.printf("advertizing %dA\r\n", advertizing_current);
}

void ui_set_phases_callback(int phases) {

    Serial.printf("set phases to %d\r\n", phases);
}

void setup(void) {

  Serial.begin(115200);
  delay(2500);
  Serial.printf("\r\n\r\nM5Stack CoreS3 SE\r\n\r\n");

  smart_evse_init();

  int board_max_current = 16;

  if (smart_evse_is_32_amp(ADDRESS)) {
    board_max_current = 32;
  }

  ui_init(board_max_current);

  Serial.printf("init done\r\n");

  Serial.printf("Request serial and fw version:\r\n");

  smart_evse_get_serial(ADDRESS);

  smart_evse_get_fw_version(ADDRESS);

  smart_evse_get_max_currents(ADDRESS);

  next_time = millis() + 1000;
}

void loop(void) {

  unsigned long now = millis();

  if (now > next_time) {

    next_time = now + 5000;

    smart_evse_get_charger_state(ADDRESS, &charger_state);

    if (charger_state.isNew) {

      charger_state.isNew = false;

      Serial.printf("state: %d, error: %d, temperature: %d, cable: %d a\r\n",
                    charger_state.state, charger_state.error, charger_state.temperature, charger_state.cable_current);
      Serial.printf("current: %f, %f, %f a\r\n", charger_state.c[0], charger_state.c[1], charger_state.c[2]);
      Serial.printf("voltage: %f, %f, %f v\r\n", charger_state.v[0], charger_state.v[1], charger_state.v[2]);
      Serial.printf("session: %f kwh, total: %f kwh\r\n\r\n", charger_state.kwh_session, charger_state.kwh_total);

      ui_set_ev_values(charger_state);
    }
  }

  ui_process();
}
