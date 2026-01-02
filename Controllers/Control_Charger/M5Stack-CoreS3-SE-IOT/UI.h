#ifndef UI_h
#define UI_h

#include "SmartEVSE.h"

void ui_init(int board_max_current);
void ui_set_advertized(int value);
void ui_set_ev_values(ChargerState charger_state);
void ui_unset_ev_values();
void ui_set_home_values(double home_current[3]);
void ui_process();

#define POS_X 16
#define POS_Y 8

#define POS_DX 12
#define POS_DY 16

#define POS_Y_L1 1
#define POS_Y_L2 2
#define POS_Y_L3 3

#define POS_X_EV_A 4
#define POS_X_EV_V 10
#define POS_X_HOME_A 16

#define ADVERTIZED_X 16
#define ADVERTIZED_Y 160
#define ADVERTIZED_V_X (13 * POS_DX)
#define ADVERTIZED_V_Y 0

#define SLIDER_X 10
#define SLIDER_Y 190
#define SLIDER_WIDTH 300
#define SLIDER_HEIGHT 40

#define BUTTON_X 246
#define BUTTON_Y 156
#define BUTTON_WIDTH 24
#define BUTTON_HEIGHT 24

#endif