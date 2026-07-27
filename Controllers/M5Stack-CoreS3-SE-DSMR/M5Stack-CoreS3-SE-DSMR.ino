/*

  M5Stack M5CoreS3 SE

  Board: M5Stack: M5CoreS3

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
  - API: https://docs.m5stack.com/en/arduino/m5unified/m5unified_appendix
  - schematics: https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/498/Sch_M5_CoreS3_SE_v1.0.pdf
   
  Notes:
  - screen size: 320 x 240
  

  Contact us if you help controlling our evse: https://edgetech.eu/

    your friends @ EdgeTech
  
*/
#include <Arduino.h>
#include "Board.h"
#include "DSMR5Reader.h"
#include "UI.h"

#include <M5Unified.h>

// the maximum current draw of the complete house for each phase

#define MAX_HOME_CURRENT 25


// DSMR smart meter should get new values every second
#define INTERVAL 1000
unsigned long next_time;

// allow for 3x not being able to read values before nulling
#define ALLOW_MISSING 3
unsigned char allow_missing_count;

// phases detected in telegram from dsmr
int active_phases;


void setup(void) {

  Serial.begin(115200);

  Serial.printf("\r\n\r\nM5Stack CoreS3 SE running EV Charger controller\r\n");
  Serial.printf(" - read DSMR5 meter for totaal house consumption\r\n\r\n");

  board_init();

  ui_init();
  ui_splash();

  ui_delay_untill_tapped(5000);

  board_system_check();

  Serial.printf("All seem to work fine!\r\n\r\n");

  ui_delay_untill_tapped(30000);

  ui_clear();

  ui_setup_main(MAX_HOME_CURRENT, active_phases);

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
  }

  ui_process();
}
