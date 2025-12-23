#include "lvgl.h"
#include "misc/lv_color.h"
#include "SmartEVSE.h"
#include "UI.h"


// use: https://lvgl.io/tools/imageconverter
// select: LVGL v8, CF_RGB565AB, C array
// Red: 5 bit, Green: 6 bit, Blue: 5 bit*

LV_IMG_DECLARE(charger_200x100);
LV_IMG_DECLARE(ev_connected_200x100);
LV_IMG_DECLARE(ev_charging_200x100);

int allow_charging;
int advertize_current;
int state;

lv_obj_t *slider;
lv_obj_t *btn13;
lv_obj_t *label13;
lv_obj_t *labelbtn;
lv_obj_t *ACV;
lv_obj_t *btn;
lv_obj_t *c1, *c2, *c3, *ch1, *ch2, *ch3, *v1, *v2, *v3, *p, *ph, *es, *et;
lv_style_t sl;
// static lv_anim_t animation_template;
static lv_style_t error_label_style;
lv_obj_t *connectionvalue;
lv_obj_t *img;

void ui_set_advertizing_current_callback(int advertizing_current);
void ui_set_phases_callback(int phases);

void ui_set_ev_values(ChargerState charger_state) {

  if(charger_state.state != state) {

    state = charger_state.state;

    switch(state) {
      case 0:
      case 1:
        lv_img_set_src(img, &charger_200x100);
        break;
      case 2:
        lv_img_set_src(img, &ev_connected_200x100);
        break;
      case 3:
      case 4:
          lv_img_set_src(img, &ev_charging_200x100);
        break;
    }
  }

  lv_label_set_text_fmt(c1, "%d", (int)(charger_state.c[0] + 0.5));
  lv_label_set_text_fmt(c2, "%d", (int)(charger_state.c[1] + 0.5));
  lv_label_set_text_fmt(c3, "%d", (int)(charger_state.c[2] + 0.5));

  lv_label_set_text_fmt(v1, "%d", (int)(charger_state.v[0] + 0.5));
  lv_label_set_text_fmt(v2, "%d", (int)(charger_state.v[1] + 0.5));
  lv_label_set_text_fmt(v3, "%d", (int)(charger_state.v[2] + 0.5));

  double power = charger_state.v[0] * charger_state.c[0] + charger_state.v[1] * charger_state.c[1] + charger_state.v[2] * charger_state.c[2];

  double v = power / 1000.;
  int bdp = (int)v;
  int adp = (int)(v * 10 - bdp * (10));

  lv_label_set_text_fmt(p, "%d.%d", bdp, adp);

  v = charger_state.kwh_session;
  bdp = (int)v;
  adp = (int)(v * 10 - bdp * 10);
  lv_label_set_text_fmt(es, "%d.%d", bdp, adp);

  v = charger_state.kwh_total;
  bdp = (int)v;
  adp = (int)(v * 10 - bdp * 10);
  lv_label_set_text_fmt(et, "%d.%d", bdp, adp);

  lv_label_set_text_fmt(connectionvalue, "%d", 100 - (int)(0.5 + (double)charger_state.error_count / (double)charger_state.count));
}

void ui_set_home_values(double c[3], double v[3]) {

  lv_label_set_text_fmt(ch1, "%d", (int)(c[0] + 0.5));
  lv_label_set_text_fmt(ch2, "%d", (int)(c[1] + 0.5));
  lv_label_set_text_fmt(ch3, "%d", (int)(c[2] + 0.5));


  double power = v[0] * c[0] + v[1] * c[1] + v[2] * c[2];

  double vv = power / 1000.;
  int bdp = (int)vv;
  int adp = (int)(vv * 10 - bdp * (10));

  lv_label_set_text_fmt(ph, "%d.%d", bdp, adp);
}

/*
    logic:

    - unchecked button "Start Charging"
      - charging = 0
      - advertized = 0
      - slider is greyed out
      
    - checked buttont "Stop Charging"
      - if advertize_current = 0 then advertize_current = 6
      - charging = 1
      - advertized = advertize_current
      - slider is active color
*/
static void button_charging_event_cb(lv_event_t *e) {

  lv_event_code_t code = lv_event_get_code(e);
  //lv_obj_t * btn = lv_event_get_target_obj(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    bool is_checked = lv_obj_has_state(btn, LV_STATE_CHECKED);
    if (is_checked) {
      allow_charging = 1;
      if (advertize_current == 0) {
        advertize_current = 6;
        lv_slider_set_value(slider, advertize_current, LV_ANIM_ON);
      }
      lv_label_set_text_fmt(ACV, "%dA", advertize_current);
      lv_label_set_text(labelbtn, "Stop charging");
    } else {
      allow_charging = 0;
      lv_label_set_text_fmt(ACV, "%dA", 0);
      lv_label_set_text(labelbtn, "Allow charging");
    }
  }
}

/*
    logic:
    - set advertize_current on release
    - value below 6 will reset value to 0 and "Stop charging"
*/
static void slider_advertized_event_cb(lv_event_t *e) {

  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_RELEASED) {

    advertize_current = lv_slider_get_value(slider);

    if (advertize_current < 6) {
      advertize_current = 0;
      lv_slider_set_value(slider, advertize_current, LV_ANIM_ON);
      allow_charging = 0;
      lv_obj_clear_state(btn, LV_STATE_CHECKED);
      lv_label_set_text(labelbtn, "Start charging");
    }

    if (allow_charging) {
      lv_label_set_text_fmt(ACV, "%dA", advertize_current);
    }
  }
}

/*
    logic:

    - unchecked: 3
    - checked: 1
    - transition: stop charging, change, start charging
*/
static void button_13phase_event_cb(lv_event_t *e) {

  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    bool is_checked = lv_obj_has_state(btn13, LV_STATE_CHECKED);
    if (is_checked) {
      lv_label_set_text(label13, "1");
      ui_set_phases_callback(1);
    } else {
      lv_label_set_text(label13, "3");
      ui_set_phases_callback(3);
    }
  }
}




void ui_init(int board_max_current) {

  // variables

  allow_charging = 0;
  advertize_current = board_max_current;
  state = 0;

  // screen
  lv_obj_t *scr = lv_scr_act();
  lv_obj_remove_style_all(scr);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

  // image
  img = lv_img_create(scr);
  lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
  lv_img_set_src(img, &charger_200x100);
  // lv_img_set_pivot(img, 50,50);
  // lv_img_set_angle(img, 0);
  lv_obj_set_pos(img, 20, 118);
  lv_obj_set_size(img, 200, 100);
  lv_obj_set_scrollbar_mode(img, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_img_opa(img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);


  // button
  // initial: checked, "Start Charging"
  btn = lv_btn_create(scr);
  lv_obj_set_pos(btn, BUTTON_CHARGING_X, BUTTON_CHARGING_Y);
  lv_obj_set_size(btn, BUTTON_CHARGING_WIDTH, BUTTON_CHARGING_HEIGHT);
  lv_obj_clear_state(btn, LV_STATE_CHECKED);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_event_cb(btn, button_charging_event_cb, LV_EVENT_ALL, NULL);
  labelbtn = lv_label_create(btn);
  lv_label_set_text(labelbtn, "Start Charging");
  lv_obj_center(labelbtn);

  // slider, advertize current
  // initial: max
  slider = lv_slider_create(scr);
  lv_slider_set_range(slider, 0, board_max_current);
  lv_slider_set_value(slider, board_max_current, LV_ANIM_ON);
  lv_obj_set_pos(slider, SLIDER_ADVERTIZE_X, SLIDER_ADVERTIZE_Y);
  lv_obj_set_width(slider, SLIDER_ADVERTIZE_WIDTH);
  lv_obj_add_event_cb(slider, slider_advertized_event_cb, LV_EVENT_ALL, NULL);

  // slider ticks
  lv_obj_t *t0 = lv_label_create(scr);
  lv_label_set_text(t0, "0A");
  lv_obj_set_width(t0, SLIDER_ADVERTIZE_TICK_WIDTH);
  lv_obj_set_pos(t0, SLIDER_ADVERTIZE_TICK_0_X - SLIDER_ADVERTIZE_TICK_WIDTH / 2, SLIDER_ADVERTIZE_TICK_Y);
  lv_obj_set_style_text_align(t0, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_t *t6 = lv_label_create(scr);
  lv_label_set_text(t6, "6A");
  lv_obj_set_width(t6, SLIDER_ADVERTIZE_TICK_WIDTH);
  lv_obj_set_pos(t6, SLIDER_ADVERTIZE_TICK_0_X + SLIDER_ADVERTIZE_WIDTH * 6 / board_max_current - SLIDER_ADVERTIZE_TICK_WIDTH / 2, SLIDER_ADVERTIZE_TICK_Y);
  lv_obj_set_style_text_align(t6, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_t *t32 = lv_label_create(scr);
  lv_label_set_text_fmt(t32, "%dA", board_max_current);
  lv_obj_set_width(t32, SLIDER_ADVERTIZE_TICK_WIDTH);
  lv_obj_set_pos(t32, SLIDER_ADVERTIZE_TICK_MAX_X - SLIDER_ADVERTIZE_TICK_WIDTH / 2, SLIDER_ADVERTIZE_TICK_Y);
  lv_obj_set_style_text_align(t32, LV_TEXT_ALIGN_CENTER, 0);


  // advertizing
  lv_obj_t *AC = lv_label_create(scr);
  lv_label_set_text_fmt(AC, "Advertizing:        (of %dA) x  /", board_max_current);
  lv_obj_set_pos(AC, LABEL_ADVERTIZING_X, LABEL_ADVERTIZING_Y);
  ACV = lv_label_create(scr);
  lv_label_set_text(ACV, "0A");
  lv_obj_set_pos(ACV, LABEL_ADVERTIZING_VALUE_X, LABEL_ADVERTIZING_VALUE_Y);


  // buttons 1/3 phase
  // initial: unchecked, 3 phase
  btn13 = lv_btn_create(scr);
  lv_obj_add_event_cb(btn13, button_13phase_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_set_pos(btn13, BUTTON_13_X, BUTTON_13_Y);
  lv_obj_set_size(btn13, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_clear_state(btn13, LV_STATE_CHECKED);
  lv_obj_add_flag(btn13, LV_OBJ_FLAG_CHECKABLE);
  label13 = lv_label_create(btn13);
  lv_label_set_text(label13, "3");
  lv_obj_center(label13);

  lv_obj_t *l;



  // header row
  // EV
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_EV_X, LABEL_HEADER_LINE_Y);
  lv_label_set_text(l, "EV");
  // Home
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_HOME_X, LABEL_HEADER_LINE_Y);
  lv_label_set_text(l, "Home");


  // l1 row
  // L1 label
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_L_X, LABEL_L1_LINE_Y);
  lv_label_set_text(l, "L1");

  // current value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_CV_X, LABEL_L1_LINE_Y);
  lv_obj_set_width(l, LABEL_CV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  c1 = l;
  // unit
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_CU_X, LABEL_L1_LINE_Y);
  lv_label_set_text(l, "A");

  // voltage value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_VV_X, LABEL_L1_LINE_Y);
  lv_obj_set_width(l, LABEL_VV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  v1 = l;
  // voltage unit
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_VU_X, LABEL_L1_LINE_Y);
  lv_label_set_text(l, "V");

  // home current value
  // current value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_HCV_X, LABEL_L1_LINE_Y);
  lv_obj_set_width(l, LABEL_HCV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  ch1 = l;
  // homecurrent unit
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_HCU_X, LABEL_L1_LINE_Y);
  lv_label_set_text(l, "A");


  // l2 row
  // L2 label
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_L_X, LABEL_L2_LINE_Y);
  lv_label_set_text(l, "L2");

  // current value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_CV_X, LABEL_L2_LINE_Y);
  lv_obj_set_width(l, LABEL_CV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  c2 = l;
  // unit
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_CU_X, LABEL_L2_LINE_Y);
  lv_label_set_text(l, "A");

  // voltage value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_VV_X, LABEL_L2_LINE_Y);
  lv_obj_set_width(l, LABEL_VV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  v2 = l;
  // voltage unit
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_VU_X, LABEL_L2_LINE_Y);
  lv_label_set_text(l, "V");

  // home current value
  // current value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_HCV_X, LABEL_L2_LINE_Y);
  lv_obj_set_width(l, LABEL_HCV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  ch2 = l;

  // homecurrent unit
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_HCU_X, LABEL_L2_LINE_Y);
  lv_label_set_text(l, "A");


  // l3 row
  // L3 label
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_L_X, LABEL_L3_LINE_Y);
  lv_label_set_text(l, "L3");

  // current value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_CV_X, LABEL_L3_LINE_Y);
  lv_obj_set_width(l, LABEL_CV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  c3 = l;
  // unit
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_CU_X, LABEL_L3_LINE_Y);
  lv_label_set_text(l, "A");

  // voltage value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_VV_X, LABEL_L3_LINE_Y);
  lv_obj_set_width(l, LABEL_VV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  v3 = l;
  // voltage unit
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_VU_X, LABEL_L3_LINE_Y);
  lv_label_set_text(l, "V");

  // home current value
  // current value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_HCV_X, LABEL_L3_LINE_Y);
  lv_obj_set_width(l, LABEL_HCV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  ch3 = l;

  // homecurrent unit
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_HCU_X, LABEL_L3_LINE_Y);
  lv_label_set_text(l, "A");


  // p row
  // p
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_L_X, LABEL_P_LINE_Y);
  lv_label_set_text(l, "P");
  // value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_PV_X, LABEL_P_LINE_Y);
  lv_obj_set_width(l, LABEL_PV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  p = l;
  // kw
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_PU_X, LABEL_P_LINE_Y);
  lv_label_set_text(l, "kW");
  // value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_HPV_X, LABEL_P_LINE_Y);
  lv_obj_set_width(l, LABEL_HPV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  ph = l;
  // kw
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_HPU_X, LABEL_P_LINE_Y);
  lv_label_set_text(l, "kW");


  // energy session
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_L_X, LABEL_E_SESSION_LINE_Y);
  lv_label_set_text(l, "Energy (session)");
  // value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_ENV_X, LABEL_E_SESSION_LINE_Y);
  lv_obj_set_width(l, LABEL_ENV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  es = l;
  // kw
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_ENU_X, LABEL_E_SESSION_LINE_Y);
  lv_label_set_text(l, "kWh");

  // energy total
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_L_X, LABEL_E_TOTAL_LINE_Y);
  lv_label_set_text(l, "Energy (total)");
  // value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_ENV_X, LABEL_E_TOTAL_LINE_Y);
  lv_obj_set_width(l, LABEL_ENV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  et = l;
  // kw
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_ENU_X, LABEL_E_TOTAL_LINE_Y);
  lv_label_set_text(l, "kWh");

  // connection
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_L_X, LABEL_CONNECTION_LINE);
  lv_label_set_text(l, "Connection");
  // value
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_CONNECTIONV_X, LABEL_CONNECTION_LINE);
  lv_obj_set_width(l, LABEL_ENV_WIDTH);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(l, "...");
  connectionvalue = l;
  // %
  l = lv_label_create(scr);
  lv_obj_set_pos(l, LABEL_CONNECTIONU_X, LABEL_CONNECTION_LINE);
  lv_label_set_text(l, "%");


  //error
  /*
  lv_anim_init(&animation_template);
  lv_anim_set_delay(&animation_template, 1000);         //Wait 1 second to start the first scroll
  lv_anim_set_repeat_delay(&animation_template, 10000); //Repeat the scroll 3 seconds after the label scrolls back to the initial position
  lv_anim_set_repeat_count(&animation_template, LV_ANIM_REPEAT_INFINITE);
  lv_style_init(&error_label_style);
  lv_style_set_anim(&error_label_style, &animation_template);
  */
  lv_obj_t *error_label = lv_label_create(scr);
  lv_label_set_long_mode(error_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(error_label, LABEL_ERROR_WIDTH);
  lv_label_set_text(error_label, "no errors");
  lv_obj_set_pos(error_label, LABEL_ERROR_X, LABEL_ERROR_Y);
  lv_obj_add_style(error_label, &error_label_style, LV_STATE_DEFAULT);

  //update
  lv_obj_update_layout(scr);
}