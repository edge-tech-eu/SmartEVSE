#include <M5Unified.h>
#include "UI.h"
#include "Slider.h"
#include "Button.h"

void ui_set_advertizing_current_callback(int advertizing_current);
void ui_set_phases_callback(int phases);

static slider_t slider;
static button_t button;

int advertized;
bool single_phases;

int last_value;


void ui_init() {

  auto cfg = M5.config();
  cfg.internal_imu = false;
  cfg.internal_mic = false;
  cfg.clear_display = true;
  M5.begin(cfg);
  M5.Display.setBrightness(128);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.display();
}

void ui_delay_untill_tapped(unsigned long delay) {

  unsigned long till = millis() + delay;

  while (millis() < till) {

    M5.update();

    auto touch = M5.Touch.getDetail();
    if (touch.wasClicked()) {
      return;
    }
  }
}

void ui_start_update() {

  M5.Display.startWrite();
}

void ui_end_update() {

  //M5.Display.display();
  M5.Display.endWrite();
}

void ui_clear() {

  M5.Display.fillScreen(DEFAULT_BG_COLOR);
  M5.Display.setTextSize(1);
  M5.Display.setFont(&DEFAULT_FONT);
  M5.Display.setTextColor(DEFAULT_TEXT_COLOR, DEFAULT_BG_COLOR);

  M5.Display.display();
}

void ui_splash() {

  M5.Display.setTextColor(SPLASH_COLOR_1, DEFAULT_BG_COLOR);
  M5.Display.setFont(&SPLASH_FONT);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(30, 15);
  M5.Display.setCursor(320 / 2 - SPLASH_FONT_DX * 19 / 2, 15);
  M5.Display.printf("from your friends @");
  M5.Display.setTextColor(SPLASH_COLOR_2, DEFAULT_BG_COLOR);
  M5.Display.setCursor(320 / 2 - SPLASH_FONT_DX * 15 / 2, 240 - 15 - SPLASH_FONT_DY);
  M5.Display.printf("www.edgetech.eu");

  M5.Display.fillRoundRect(SPLASH_X, SPLASH_Y, SPLASH_WIDTH, SPLASH_HEIGHT, SPLASH_ROUNDNESS, SPLASH_COLOR_2);

  M5.Display.fillTriangle(
    SPLASH_X + SPLASH_WIDTH / 2 - SPLASH_DIAG_DX, SPLASH_Y,
    SPLASH_X + SPLASH_WIDTH / 2 + SPLASH_DIAG_DX, SPLASH_Y,
    SPLASH_X + SPLASH_WIDTH / 2 + SPLASH_DIAG_DX, SPLASH_Y + SPLASH_HEIGHT,
    SPLASH_COLOR_1);
  M5.Display.fillRoundRect(SPLASH_X + SPLASH_WIDTH / 2, SPLASH_Y, SPLASH_WIDTH / 2, SPLASH_HEIGHT, SPLASH_ROUNDNESS, SPLASH_COLOR_1);

  M5.Display.fillTriangle(
    SPLASH_X + SPLASH_WIDTH / 2 - SPLASH_DIAG_DX, SPLASH_Y,
    SPLASH_X + SPLASH_WIDTH / 2 + SPLASH_DIAG_DX, SPLASH_Y + SPLASH_HEIGHT,
    SPLASH_X + SPLASH_WIDTH / 2 - SPLASH_DIAG_DX, SPLASH_Y + SPLASH_HEIGHT,
    SPLASH_COLOR_2);

  M5.Display.setTextColor(SPLASH_COLOR_1, SPLASH_COLOR_2);
  M5.Display.setFont(&SPLASH_FONT);
  M5.Display.setTextSize(SPLASH_FONT_SIZE);
  M5.Display.setCursor(SPLASH_X + SPLASH_TEXT_1_DX, SPLASH_Y + SPLASH_TEXT_1_DY);
  M5.Display.printf("edge");
  M5.Display.setTextColor(SPLASH_COLOR_2, SPLASH_COLOR_1);
  M5.Display.setCursor(SPLASH_X + SPLASH_TEXT_2_DX, SPLASH_Y + SPLASH_TEXT_2_DY);
  M5.Display.printf("tech");

  M5.Display.display();
}

void ui_start_up() {

  ui_start_update();

  M5.Display.setFont(&AsciiFont8x16);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(0, POS_Y);
  M5.Display.printf("Initializing system:\r\n");
  /*
  M5.Display.printf("ev charger: connected!");
  M5.Display.setCursor(0, POS_Y + 16 * 1.5);
  M5.Display.printf("serial: %04x%04x%04x%04x%04x", serial[0], serial[1], serial[2], serial[3], serial[4]);
  M5.Display.setCursor(0, POS_Y + 16 * 3);
  M5.Display.printf("firmware version: %4x\r\n", version);
*/
  ui_end_update();
}

void ui_start_up_add_line(char *line) {

  M5.Display.print(line);
  M5.Display.display();
}

void ui_start_up_add_line_error(char *line) {

  M5.Display.setTextColor(ERROR_TEXT_COLOR, DEFAULT_BG_COLOR);
  M5.Display.print(line);
  M5.Display.setTextColor(DEFAULT_TEXT_COLOR, DEFAULT_BG_COLOR);
  M5.Display.display();
}

void ui_setup_main(int board_max_current, int initial_phases) {

  ui_start_update();

  advertized = board_max_current;
  single_phases = (initial_phases == 1);

  M5.Display.setTextSize(2);
  M5.Display.setCursor(POS_X, POS_Y);
  M5.Display.print("    EV          Home");
  M5.Display.setCursor(POS_X, POS_Y + POS_Y_L1);
  M5.Display.print("L1:     A    V      A");
  M5.Display.setCursor(POS_X, POS_Y + POS_Y_L2);
  M5.Display.print("L2:     A    V      A");
  M5.Display.setCursor(POS_X, POS_Y + POS_Y_L3);
  M5.Display.print("L3:     A    V      A");

  M5.Display.setCursor(POS_X, POS_Y + POS_Y_KWH_SESSION);
  M5.Display.print("Session:     kWh");
  M5.Display.setCursor(POS_X, POS_Y + POS_Y_KWH_TOTAL);
  M5.Display.print("Total:       kWh");

  M5.Display.setCursor(STATE_X, STATE_Y);
  M5.Display.print("State: ");

  ui_set_state(0);

  M5.Display.setCursor(ADVERTIZED_X, ADVERTIZED_Y);
  M5.Display.print("Advertizing:   A x ");

  ui_set_advertized(advertized);

  Serial.printf("slider: max value %d\r\n", advertized);

  slider.setup({ SLIDER_X, SLIDER_Y, SLIDER_WIDTH, SLIDER_HEIGHT }, 0, advertized, advertized, TFT_WHITE, TFT_BLACK, TFT_LIGHTGRAY);
  slider.draw();

  button.setup({ BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT }, "3", "1", single_phases, TFT_WHITE, TFT_BLACK, TFT_LIGHTGRAY);
  button.draw();

  ui_end_update();
}

void ui_set_state(int state) {

  M5.Display.setCursor(STATE_X + STATE_V_X, STATE_Y);

  switch (state) {
    case 0: M5.Display.printf("unknown     "); break;
    case 1: M5.Display.printf("no ev       "); break;
    case 2: M5.Display.printf("ev connected"); break;
    case 3: M5.Display.printf("ev charging "); break;
    case 4: M5.Display.printf("ev charging "); break;
  }
}

void ui_set_advertized(int value) {

  if (last_value != value) {

    last_value != value;

    M5.Display.setCursor(ADVERTIZED_X + ADVERTIZED_V_X, ADVERTIZED_Y + ADVERTIZED_V_Y);
    M5.Display.printf("%2d", value);

    slider.value_update(value);
  }
}

void ui_set_ev_values(ChargerState charger_state) {

  M5.Display.setCursor(POS_X + POS_X_EV_A * POS_DX, POS_Y + POS_Y_L1);
  M5.Display.printf("%4.1f", charger_state.c[0]);

  M5.Display.setCursor(POS_X + POS_X_EV_V * POS_DX, POS_Y + POS_Y_L1);
  M5.Display.printf("%3.0f", charger_state.v[0]);

  M5.Display.setCursor(POS_X + POS_X_EV_A * POS_DX, POS_Y + POS_Y_L2);
  M5.Display.printf("%4.1f", charger_state.c[1]);

  M5.Display.setCursor(POS_X + POS_X_EV_V * POS_DX, POS_Y + POS_Y_L2);
  M5.Display.printf("%3.0f", charger_state.v[1]);

  M5.Display.setCursor(POS_X + POS_X_EV_A * POS_DX, POS_Y + POS_Y_L3);
  M5.Display.printf("%4.1f", charger_state.c[2]);

  M5.Display.setCursor(POS_X + POS_X_EV_V * POS_DX, POS_Y + POS_Y_L3);
  M5.Display.printf("%3.0f", charger_state.v[2]);

  M5.Display.setCursor(POS_X + POS_X_KWH, POS_Y + POS_Y_KWH_SESSION);
  M5.Display.printf("%5.1f", charger_state.kwh_session);

  M5.Display.setCursor(POS_X + POS_X_KWH, POS_Y + POS_Y_KWH_TOTAL);
  M5.Display.printf("%5.1f", charger_state.kwh_total);

  ui_set_state(charger_state.state);
}

void ui_unset_ev_values() {

  M5.Display.setCursor(POS_X + POS_X_EV_A * POS_DX, POS_Y + POS_Y_L1);
  M5.Display.printf("    ");
  M5.Display.setCursor(POS_X + POS_X_EV_V * POS_DX, POS_Y + POS_Y_L1);
  M5.Display.printf("   ");

  M5.Display.setCursor(POS_X + POS_X_EV_A * POS_DX, POS_Y + POS_Y_L2);
  M5.Display.printf("    ");
  M5.Display.setCursor(POS_X + POS_X_EV_V * POS_DX, POS_Y + POS_Y_L2);
  M5.Display.printf("   ");

  M5.Display.setCursor(POS_X + POS_X_EV_A * POS_DX, POS_Y + POS_Y_L3);
  M5.Display.printf("    ");
  M5.Display.setCursor(POS_X + POS_X_EV_V * POS_DX, POS_Y + POS_Y_L3);
  M5.Display.printf("   ");

  ui_set_state(0);
}

void ui_set_home_values(double home_current[3]) {

  M5.Display.setCursor(POS_X + POS_X_HOME_A * POS_DX - 2, POS_Y + POS_Y_L1);
  M5.Display.printf("%5.1f", home_current[0]);

  M5.Display.setCursor(POS_X + POS_X_HOME_A * POS_DX - 2, POS_Y + POS_Y_L2);
  M5.Display.printf("%5.1f", home_current[1]);

  M5.Display.setCursor(POS_X + POS_X_HOME_A * POS_DX - 2, POS_Y + POS_Y_L3);
  M5.Display.printf("%5.1f", home_current[2]);
}

void ui_unset_home_values() {

  M5.Display.setCursor(POS_X + POS_X_HOME_A * POS_DX - 2, POS_Y + POS_Y_L1);
  M5.Display.printf("    ");

  M5.Display.setCursor(POS_X + POS_X_HOME_A * POS_DX - 2, POS_Y + POS_Y_L2);
  M5.Display.printf("    ");

  M5.Display.setCursor(POS_X + POS_X_HOME_A * POS_DX - 2, POS_Y + POS_Y_L3);
  M5.Display.printf("    ");
}


void ui_process() {

  // M5.delay(16);

  M5.update();

  if (M5.Touch.getCount() > 0) {

    auto t = M5.Touch.getDetail();

    if (slider.update(t)) {
      int suggested_advertized = slider.getValue();
      ui_set_advertized(suggested_advertized);
    }

    if (t.wasReleased()) {

      int new_advertized = slider.getValue();
      if (advertized != new_advertized) {

        if ((new_advertized > 0) && (new_advertized < 6)) {
          Serial.printf("slider set to %d, reset to 0\r\n", new_advertized);
          new_advertized = 0;
          ui_start_update();
          slider.value_update(new_advertized);
          ui_end_update();
        }

        if (advertized != new_advertized) {
          advertized = new_advertized;
          ui_start_update();
          ui_set_advertized(advertized);
          ui_end_update();
          ui_set_advertizing_current_callback(advertized);
        }
      }

      if (button.update(t)) {
        bool new_single_phases = button.getValue();
        if (single_phases != new_single_phases) {
          single_phases = new_single_phases;
          ui_set_phases_callback((single_phases ? 1 : 3));
        }
      }
    }
  }
}