#ifndef UI_h
#define UI_h

#include "SmartEVSE.h"

void ui_init();
void ui_start_update();
void ui_end_update();
void ui_start_up(int *serial, int version);
void ui_setup_main(int board_max_current, int initial_phases);
void ui_set_advertized(int value);
void ui_set_state(int state);
void ui_set_ev_values(ChargerState charger_state);
void ui_unset_ev_values();
void ui_set_home_values(double home_current[3]);
void ui_unset_home_values();
void ui_process();
void ui_clear();

// #define POS_X 16
#define POS_X (16+18)
#define POS_Y 8

#define POS_DX 12
#define POS_DY 16

#define POS_Y_L1 (POS_DY*1.5)
#define POS_Y_L2 (POS_DY*2.5)
#define POS_Y_L3 (POS_DY*3.5)

#define POS_Y_KWH_SESSION (POS_DY*5)
#define POS_Y_KWH_TOTAL (POS_DY*6)
#define POS_X_KWH (7*POS_DX)

#define POS_X_EV_A 4
#define POS_X_EV_V 10
#define POS_X_HOME_A 15

#define STATE_X POS_X
#define STATE_Y 136
#define STATE_V_X (7 * POS_DX)

#define ADVERTIZED_X POS_X
#define ADVERTIZED_Y 160
#define ADVERTIZED_V_X (13 * POS_DX)
#define ADVERTIZED_V_Y 0

#define SLIDER_X 10
#define SLIDER_Y 190
#define SLIDER_WIDTH 300
#define SLIDER_HEIGHT 40

#define BUTTON_X (POS_X+230)
#define BUTTON_Y 156
#define BUTTON_WIDTH 24
#define BUTTON_HEIGHT 24

#endif