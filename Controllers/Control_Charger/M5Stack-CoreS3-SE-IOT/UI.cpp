#include <M5ModuleDisplay.h>
#include <M5Unified.h>
#include <stdint.h>
#include "UI.h"
#include "Slider.h"

void ui_set_advertizing_current_callback(int advertizing_current);
void ui_set_phases_callback(int phases);

static slider_t slider;

#define POS_X   16
#define POS_Y   8

#define POS_DX 12
#define POS_DY 16

#define POS_Y_L1 1
#define POS_Y_L2 2
#define POS_Y_L3 3

#define POS_X_EV_A 4
#define POS_X_EV_V 10
#define POS_X_HOME_A 16

#define ADVERTIZED_X    16
#define ADVERTIZED_Y    160
#define ADVERTIZED_V_X    (21*POS_DX)
#define ADVERTIZED_V_Y    0

int advertized;

void ui_init(int board_max_current) {

  advertized = board_max_current;

  M5.begin();

  int board = M5.getBoard();
  //if(board == M5::board_M5StackCoreS3SE) {
  //  Serial.printf("Board: board_M5StackCoreS3SE\r\n");
  //} else {
  Serial.printf("Board type: %d\r\n", board);
  //}

  M5.Display.setTextSize(2);

  M5.Display.setCursor(POS_X, POS_Y);
  M5.Display.print("    EV          Home");
  M5.Display.setCursor(POS_X, POS_Y+POS_DY);
  M5.Display.print("L1:     A    V      A");
  M5.Display.setCursor(POS_X, POS_Y+2*POS_DY);
  M5.Display.print("L2:     A    V      A");
  M5.Display.setCursor(POS_X, POS_Y+3*POS_DY);
  M5.Display.print("L3:     A    V      A");

  M5.Display.setCursor(ADVERTIZED_X, ADVERTIZED_Y);
  M5.Display.print("Advertizing current:   A");

  ui_set_advertized(advertized);

  Serial.printf("slider: max value %d\r\n", advertized);

  slider.setup({ 10, 190, 300, 40 }, 0, advertized, advertized, TFT_WHITE, TFT_BLACK, TFT_LIGHTGRAY);
  slider.draw();

  ui_set_advertizing_current_callback(advertized);
}

void ui_set_advertized(int value) {

  M5.Display.setCursor(ADVERTIZED_X+ADVERTIZED_V_X, ADVERTIZED_Y+ADVERTIZED_V_Y);
  M5.Display.printf("%2d", value);

}

void ui_set_ev_values(ChargerState charger_state) {

  M5.Display.setCursor(POS_X+POS_X_EV_A * POS_DX, POS_Y+POS_Y_L1 * POS_DY);
  M5.Display.printf("%4.1f", charger_state.c[0]);
  M5.Display.setCursor(POS_X+POS_X_EV_V * POS_DX, POS_Y+POS_Y_L1 * POS_DY);
  M5.Display.printf("%3.0f", charger_state.v[0]);

  M5.Display.setCursor(POS_X+POS_X_EV_A * POS_DX, POS_Y+POS_Y_L2 * POS_DY);
  M5.Display.printf("%4.1f", charger_state.c[1]);
  M5.Display.setCursor(POS_X+POS_X_EV_V * POS_DX, POS_Y+POS_Y_L2 * POS_DY);
  M5.Display.printf("%3.0f", charger_state.v[1]);

  M5.Display.setCursor(POS_X+POS_X_EV_A * POS_DX, POS_Y+POS_Y_L3 * POS_DY);
  M5.Display.printf("%4.1f", charger_state.c[2]);
  M5.Display.setCursor(POS_X+POS_X_EV_V * POS_DX, POS_Y+POS_Y_L3 * POS_DY);
  M5.Display.printf("%3.0f", charger_state.v[2]);
}

void ui_set_home_values(double home_current[3]) {

  M5.Display.setCursor(POS_X+POS_X_HOME_A * POS_DX, POS_Y+POS_Y_L1 * POS_DY);
  M5.Display.printf("%4.1f", home_current[0]);

  M5.Display.setCursor(POS_X+POS_X_HOME_A * POS_DX, POS_Y+POS_Y_L2 * POS_DY);
  M5.Display.printf("%4.1f", home_current[1]);

  M5.Display.setCursor(POS_X+POS_X_HOME_A * POS_DX, POS_Y+POS_Y_L3 * POS_DY);
  M5.Display.printf("%4.1f", home_current[2]);
}


void ui_process() {

  // M5.delay(16);

  M5.update();

  if (M5.Touch.getCount() > 0) {

    auto t = M5.Touch.getDetail();

    slider.update(t);

    if(t.isPressed()) {
      int suggested_advertized = slider.getValue();
      ui_set_advertized(suggested_advertized);
    }

    if (t.wasReleased()) {

      Serial.println("released");

      int new_advertized = slider.getValue();
      if (advertized != new_advertized) {

        if ((new_advertized > 0) && (new_advertized < 6)) {
          Serial.printf("slider set to %d, reset to 0\r\n", new_advertized);
          new_advertized = 0;
          slider.value_update(new_advertized);
        }

        if (advertized != new_advertized) {
          advertized = new_advertized;
          ui_set_advertized(advertized);
          ui_set_advertizing_current_callback(advertized);
        }
      }
    }
  }
}