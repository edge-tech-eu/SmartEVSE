#ifndef UI_h
#define UI_h

#include "SmartEVSE.h"

void ui_init(int board_max_current);
void ui_set_advertized(int value);
void ui_set_ev_values(ChargerState charger_state);
void ui_set_home_values(double home_current[3]);
void ui_process();

#endif