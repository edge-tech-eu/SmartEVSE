/*

  M5Stack M5CoreS3 SE + COMMU

  Board: M5Stack: M5CoreS3

  BUS usage:
  COMMU: tx: U2_TXD = GPIO18/BUS_PC_RX/bus pin 16
  COMMU: rx: U2_RXD = GOIO17/BUS_PC_TX/bus pin 15
  LAN:  int: jumper left:  G35 = bus pin 2/BUS_ADC1/GPIO10   (alt     G34 = bus pin 26/I2S_DAT1/GPIO14)
  LAN:  rst: jumper right: G13 = bus pin 22/BUS_G7/GPIO7     (default G0  = bus pin 24/ESP_BOOT/GPIO0)
  LAN:   cs: jumper left:  G15 = bus pin 20/BUS_PA_SCL/GPIO1 (alt     G5  = bus pin 23/I2S_DAT0/GPIO13)


  BUS  CORES3 SE              COMMU       LAN Module
  --------------------------+-----------+--------------
   1    GND                    GND         GND
   2    GPIO10 | BUS_ADC1      -           int
   3    GND                    GND         GND
   4    GPIO7  | BUS_PB_IN     -           -
   5    GND                    GND         GND
   6    AXP_PG                 -           -
   7    GPIO37 | SPI_MOSI      CAN_MOSI    MOSI
   8    GPIO5  | BUS_G5        -           -
   9    GPIO35 | SPI_MISO      CAN_MISO    MISO
  10    GPIO9  | BUS_PB_OUT    -           -
  11    GPIO36 | SPI_SCK       CAN_SCK     SCLK
  12    3V3                    3V3         3V3
  13    BUS_U0RXD              U1_RXD      -
  14    BUS_U0TXD              U1_TXD      -
  15    GPIO18 | BUS_PC_RX     U2_RXD      -
  16    GPIO17 | BUS_PC_TX     U2_TXD      -
  17    GPIO12 | I2C_SYS_SDA   I2C_SDA     -
  18    GPIO11 | I2C_SYS_SCL   I2C_SCL     -
  19    GPIO2  | BUS_PA_SDA    -           -
  20    GPIO1  | BUS_PA_SCL    -           -
  21    GPIO6  | BUS_G6        CAN_CS      -
  22    GPIO7  | BUS_G7        -           -
  23    GPIO13 | I2S_DAT0      CAN_INT     cs
  24    GPIO0  | ESP_BOOT      -           rst
  25    HPWR                   -           HPWR
  26    GPIO14 | I2S_DAT1      -           -
  27    HPWR                   -           HPWR
  28    BUS_OUT                5V          5V
  29    HPWR                   -           HPWR
  30    VBAT                   -           -


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
  - LAN Module 13.2: https://docs.m5stack.com/en/module/LAN%20Module%2013.2
  
  Notes:
  - screen size: 320 x 240
  

  Contact us if you help controlling our evse: https://edgetech.eu/

    your friends @ EdgeTech
  
*/
#include <Arduino.h>
#include "Board.h"
#include "SmartEVSE.h"
#include "DSMR5Reader.h"
#include "UI.h"

#include <M5Unified.h>
#include <SPI.h>
#include <M5Module_LAN.h>
#include <ArduinoHttpClient.h>

M5Module_LAN LAN;


void lan_init() {
  
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x89};
  //int  cs_pin  = 1; // jumper left
  int  cs_pin  = 13; // jumper right
  int rst_pin = 0; // jumper left 
  //int rst_pin = 7; // jumper right
  int  int_pin = 10;// jumper left 
  //int  int_pin = 14;// jumper right 

    SPI.begin(SCK, MISO, MOSI, -1);

    LAN.setResetPin(rst_pin);
    LAN.reset();
    LAN.init(cs_pin);

    int count = 10;
    while ((count--) && (LAN.begin(mac) != 1)) {
        Serial.println("Error getting IP address via DHCP, trying again...");
        delay(500);
    }

    auto link = LAN.linkStatus();
    if(link == LinkON) {
      IPAddress IP = LAN.localIP();
      Serial.println(IP.toString());
    }
}

// the maximum current draw of the complete house for each phase

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

// load balancing
int max_current;
int last_max_current;

// set initial value
#define INITIAL_PHASES 3
int active_phases;

int last_set_advertizing_current;
int last_phases;

void ui_set_advertizing_current_callback(int advertizing_current) {

  Serial.printf("advertizing %dA\r\n", advertizing_current);

  smart_evse_set_max_current(ADDRESS, advertizing_current, 0);

  last_set_advertizing_current = advertizing_current;
}

void ui_set_phases_callback(int phases) {

  if (last_phases != phases) {

    if (last_set_advertizing_current > 0) {

      Serial.printf("closing relays (advertizing 0 A)\r\n");

      smart_evse_set_max_current(ADDRESS, 0, 0);
      delay(3000);
    }

    Serial.printf("set phases to %d\r\n", phases);

    smart_evse_set_phases(ADDRESS, phases == 3);
    last_phases = phases;
    delay(500);

    if (last_set_advertizing_current > 0) {

      Serial.printf("resetting advertizing current to %d A\r\n", last_set_advertizing_current);

      smart_evse_set_max_current(ADDRESS, last_set_advertizing_current, 0);
    }
  }
}


void setup(void) {

  Serial.begin(115200);

  Serial.printf("\r\n\r\nM5Stack CoreS3 SE running EV Charger controller\r\n");
  Serial.printf(" - read DSMR5 meter for totaal house consumption\r\n");
  Serial.printf(" - calculate and write max current to EV charger\r\n\r\n");

  board_init();

  // lan_init();

  ui_init();
  ui_splash();

  ui_delay_untill_tapped(5000);

  board_system_check(ADDRESS);

  Serial.printf("All seem to work fine!\r\n\r\n");

  ui_delay_untill_tapped(30000);

  ui_clear();

  // make sure the charge is in sync
  max_current = board_max_current;
  last_max_current = max_current;
  ui_set_advertizing_current_callback(max_current);
  active_phases = INITIAL_PHASES;
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
