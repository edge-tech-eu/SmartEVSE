#ifndef UI_h
#define UI_h

#include "SmartEVSE.h"

#define BUTTON_CHARGING_X                20
#define BUTTON_CHARGING_Y               340
#define BUTTON_CHARGING_WIDTH           440
#define BUTTON_CHARGING_HEIGHT          100

#define SLIDER_ADVERTIZE_X               20
#define SLIDER_ADVERTIZE_Y              300
#define SLIDER_ADVERTIZE_WIDTH          440
#define SLIDER_ADVERTIZE_TICK_WIDTH      30
#define SLIDER_ADVERTIZE_TICK_Y         280
#define SLIDER_ADVERTIZE_TICK_0_X        20
#define SLIDER_ADVERTIZE_TICK_MAX_X     460

#define LABEL_ADVERTIZING_X              20
#define LABEL_ADVERTIZING_Y             240
#define LABEL_ADVERTIZING_VALUE_X       110
#define LABEL_ADVERTIZING_VALUE_Y       240

#define BUTTON_13_X                     207
#define BUTTON_13_Y                     238
#define BUTTON_WIDTH                     20
#define BUTTON_HEIGHT                    20

#define LABEL_ERROR_X                    20
#define LABEL_ERROR_Y                   450
#define LABEL_ERROR_WIDTH               440

#define LABEL_HEADER_LINE_Y              10
#define LABEL_L1_LINE_Y                  40
#define LABEL_L2_LINE_Y                  60
#define LABEL_L3_LINE_Y                  80
#define LABEL_P_LINE_Y                  110
#define LABEL_E_SESSION_LINE_Y          150
#define LABEL_E_TOTAL_LINE_Y            170
#define LABEL_CONNECTION_LINE           200

#define LABEL_EV_X                      277
#define LABEL_HOME_X                    392

#define LABEL_L_X                       240

#define LABEL_CV_X                      262
#define LABEL_CV_WIDTH                   30
#define LABEL_CU_X                      298

#define LABEL_VV_X                      322
#define LABEL_VV_WIDTH                   30
#define LABEL_VU_X                      358

#define LABEL_HCV_X                     380
#define LABEL_HCV_WIDTH                  30
#define LABEL_HCU_X                     418

#define LABEL_PV_X                      262
#define LABEL_PV_WIDTH                   30
#define LABEL_PU_X                      298

#define LABEL_HPV_X                     372
#define LABEL_HPV_WIDTH                  40
#define LABEL_HPU_X                     418

#define LABEL_ENV_X                      352
#define LABEL_ENV_WIDTH                   40
#define LABEL_ENU_X                      398

#define LABEL_CONNECTIONV_X              352
#define LABEL_CONNECTIONV_WIDTH           40
#define LABEL_CONNECTIONU_X              398

void ui_init(int board_max_current);
void ui_set_ev_values(ChargerState charger_state);
void ui_set_home_values(double c[3], double v[3]);

#endif