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
#include <M5ModuleDisplay.h>
#include <M5Unified.h>
#include <stdint.h>
#include "Slider.h"
#include "SmartEVSE.h"

// the default address of the EdgeTech EVSE has modbus address 1
// be aware that when the address is still 1, the board needs a state change t respond to modbus

#define ADDRESS 1

ChargerState charger_state;

unsigned long next_time;

static slider_t slider;


void setup(void) {

  Serial.begin(115200);
  delay(2500);
  Serial.printf("\r\n\r\nM5Stack CoreS3 SE\r\n\r\n");

  M5.begin();

  int board = M5.getBoard();
  //if(board == M5::board_M5StackCoreS3SE) {
  //  Serial.printf("Board: board_M5StackCoreS3SE\r\n");
  //} else {
    Serial.printf("Board: %d\r\n",board);
  //}

  slider.setup({ 10, 190, 300, 40 }, 0, 32, 16, TFT_WHITE, TFT_BLACK, TFT_LIGHTGRAY);


  // Get the number of available displays
  //int display_count = M5.getDisplayCount();

     int textsize = 2;
    Serial.printf("screen size: %d x %d\r\n",M5.Display.width(),M5.Display.height());
    M5.Display.setTextSize(textsize);

    slider.draw();

    smart_evse_init();

/*
    M5.Display.print("    EV          Home\n");
    M5.Display.print("L1: 10.1A 220V   0.9A\n");
    M5.Display.print("L2: 16.2A 230V  10.8A\n");
    M5.Display.print("L3:  9.3A 240V  30.7A\n");
*/

    M5.Display.print("    EV          Home\n");
    M5.Display.print("L1:     A    V      A\n");
    M5.Display.print("L2:     A    V      A\n");
    M5.Display.print("L3:     A    V      A\n");

    #define POS_Y_L1  1
    #define POS_Y_L2  2
    #define POS_Y_L3  3

    #define POS_X_EV_A     4
    #define POS_X_EV_V    10
    #define POS_X_HOME_A  16

    M5.Display.setCursor(POS_Y_L1, POS_X_EV_A);
    M5.Display.print("10.1");
    M5.Display.setCursor(POS_Y_L1, POS_X_EV_V);
    M5.Display.print("220");
    M5.Display.setCursor(POS_Y_L1, POS_X_HOME_A);
    M5.Display.print(" 0.9");

    M5.Display.setCursor(POS_Y_L2, POS_X_EV_A);
    M5.Display.print("10.1");
    M5.Display.setCursor(POS_Y_L2, POS_X_EV_V);
    M5.Display.print("220");
    M5.Display.setCursor(POS_Y_L2, POS_X_HOME_A);
    M5.Display.print(" 0.9");
        
    M5.Display.setCursor(POS_Y_L3, POS_X_EV_A);
    M5.Display.print("10.1");
    M5.Display.setCursor(POS_Y_L3, POS_X_EV_V);
    M5.Display.print("220");
    M5.Display.setCursor(POS_Y_L3, POS_X_HOME_A);
    M5.Display.print(" 0.9");

  int board_max_current = 16;

  if (smart_evse_is_32_amp(ADDRESS)) {
    board_max_current = 32;
  }

  // ui_init(board_max_current);

  Serial.printf("init done\r\n");

  Serial.printf("Request serial and fw version:\r\n");

  smart_evse_get_serial(ADDRESS);

  smart_evse_get_fw_version(ADDRESS);

  smart_evse_get_max_currents(ADDRESS);

  next_time = millis() + 1000;
}

void loop(void) {

  M5.delay(16);

  unsigned long now = millis();

  if(now > next_time) {

    next_time = now + 5000;
    
    smart_evse_get_charger_state(ADDRESS, &charger_state);

    if (charger_state.isNew) {

      charger_state.isNew = false;

      Serial.printf("state: %d, error: %d, temperature: %d, cable: %d a\r\n",
                    charger_state.state, charger_state.error, charger_state.temperature, charger_state.cable_current);
      Serial.printf("current: %f, %f, %f a\r\n", charger_state.c[0], charger_state.c[1], charger_state.c[2]);
      Serial.printf("voltage: %f, %f, %f v\r\n", charger_state.v[0], charger_state.v[1], charger_state.v[2]);
      Serial.printf("session: %f kwh, total: %f kwh\r\n\r\n",charger_state.kwh_session, charger_state.kwh_total);

      // ui_set_ev_values(charger_state);
    }
  }

  M5.update();

  if (M5.Touch.getCount() > 0) {

    auto t = M5.Touch.getDetail();

    slider.update(t);

    if(slider.wasChanged()) {
      Serial.printf("value now is %d\r\n", slider.getValue());
    }
  }
}
