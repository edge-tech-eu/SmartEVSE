#ifndef UI_h
#define UI_h

#include "SmartEVSE.h"

void ui_init();
void ui_delay_untill_tapped(unsigned long delay);
void ui_splash();
void ui_start_update();
void ui_end_update();
void ui_start_up();
void ui_start_up_add_line(char *line);
void ui_start_up_add_line_error(char *line);
void ui_setup_main(int board_max_current, int initial_phases);
void ui_set_advertized(int value);
void ui_set_state(int state);
void ui_set_ev_values(ChargerState charger_state);
void ui_unset_ev_values();
void ui_set_home_values(double home_current[3]);
void ui_unset_home_values();
void ui_process();
void ui_clear();

/*

  available fonts:

extern const lgfx::GLCDfont Font0;
    extern const lgfx::BMPfont  Font2;
    extern const lgfx::RLEfont  Font4;
    extern const lgfx::RLEfont  Font6;
    extern const lgfx::RLEfont  Font7;
    extern const lgfx::RLEfont  Font8;
    extern const lgfx::GLCDfont Font8x8C64;
    extern const lgfx::FixedBMPfont AsciiFont8x16;
    extern const lgfx::FixedBMPfont AsciiFont24x48;

    extern const lgfx::GFXfont TomThumb                 ;
    extern const lgfx::GFXfont FreeMono9pt7b            ;
    extern const lgfx::GFXfont FreeMono12pt7b           ;
    extern const lgfx::GFXfont FreeMono18pt7b           ;
    extern const lgfx::GFXfont FreeMono24pt7b           ;
    extern const lgfx::GFXfont FreeMonoBold9pt7b        ;
    extern const lgfx::GFXfont FreeMonoBold12pt7b       ;
    extern const lgfx::GFXfont FreeMonoBold18pt7b       ;
    extern const lgfx::GFXfont FreeMonoBold24pt7b       ;
    extern const lgfx::GFXfont FreeMonoOblique9pt7b     ;
    extern const lgfx::GFXfont FreeMonoOblique12pt7b    ;
    extern const lgfx::GFXfont FreeMonoOblique18pt7b    ;
    extern const lgfx::GFXfont FreeMonoOblique24pt7b    ;
    extern const lgfx::GFXfont FreeMonoBoldOblique9pt7b ;
    extern const lgfx::GFXfont FreeMonoBoldOblique12pt7b;
    extern const lgfx::GFXfont FreeMonoBoldOblique18pt7b;
    extern const lgfx::GFXfont FreeMonoBoldOblique24pt7b;
    extern const lgfx::GFXfont FreeSans9pt7b            ;
    extern const lgfx::GFXfont FreeSans12pt7b           ;
    extern const lgfx::GFXfont FreeSans18pt7b           ;
    extern const lgfx::GFXfont FreeSans24pt7b           ;
    extern const lgfx::GFXfont FreeSansBold9pt7b        ;
    extern const lgfx::GFXfont FreeSansBold12pt7b       ;
    extern const lgfx::GFXfont FreeSansBold18pt7b       ;
    extern const lgfx::GFXfont FreeSansBold24pt7b       ;
    extern const lgfx::GFXfont FreeSansOblique9pt7b     ;
    extern const lgfx::GFXfont FreeSansOblique12pt7b    ;
    extern const lgfx::GFXfont FreeSansOblique18pt7b    ;
    extern const lgfx::GFXfont FreeSansOblique24pt7b    ;
    extern const lgfx::GFXfont FreeSansBoldOblique9pt7b ;
    extern const lgfx::GFXfont FreeSansBoldOblique12pt7b;
    extern const lgfx::GFXfont FreeSansBoldOblique18pt7b;
    extern const lgfx::GFXfont FreeSansBoldOblique24pt7b;
    extern const lgfx::GFXfont FreeSerif9pt7b           ;
    extern const lgfx::GFXfont FreeSerif12pt7b          ;
    extern const lgfx::GFXfont FreeSerif18pt7b          ;
    extern const lgfx::GFXfont FreeSerif24pt7b          ;
    extern const lgfx::GFXfont FreeSerifItalic9pt7b     ;
    extern const lgfx::GFXfont FreeSerifItalic12pt7b    ;
    extern const lgfx::GFXfont FreeSerifItalic18pt7b    ;
    extern const lgfx::GFXfont FreeSerifItalic24pt7b    ;
    extern const lgfx::GFXfont FreeSerifBold9pt7b       ;
    extern const lgfx::GFXfont FreeSerifBold12pt7b      ;
    extern const lgfx::GFXfont FreeSerifBold18pt7b      ;
    extern const lgfx::GFXfont FreeSerifBold24pt7b      ;
    extern const lgfx::GFXfont FreeSerifBoldItalic9pt7b ;
    extern const lgfx::GFXfont FreeSerifBoldItalic12pt7b;
    extern const lgfx::GFXfont FreeSerifBoldItalic18pt7b;
    extern const lgfx::GFXfont FreeSerifBoldItalic24pt7b;

    extern const lgfx::GFXfont Orbitron_Light_24;
    extern const lgfx::GFXfont Orbitron_Light_32;
    extern const lgfx::GFXfont Roboto_Thin_24   ;
    extern const lgfx::GFXfont Satisfy_24       ;
    extern const lgfx::GFXfont Yellowtail_32    ;
    extern const lgfx::GFXfont DejaVu9 ;
    extern const lgfx::GFXfont DejaVu12;
    extern const lgfx::GFXfont DejaVu18;
    extern const lgfx::GFXfont DejaVu24;
    extern const lgfx::GFXfont DejaVu40;
    extern const lgfx::GFXfont DejaVu56;
    extern const lgfx::GFXfont DejaVu72;

    extern const lgfx::U8g2font lgfxJapanMincho_8  ;
    extern const lgfx::U8g2font lgfxJapanMincho_12 ;
    extern const lgfx::U8g2font lgfxJapanMincho_16 ;
    extern const lgfx::U8g2font lgfxJapanMincho_20 ;
    extern const lgfx::U8g2font lgfxJapanMincho_24 ;
    extern const lgfx::U8g2font lgfxJapanMincho_28 ;
    extern const lgfx::U8g2font lgfxJapanMincho_32 ;
    extern const lgfx::U8g2font lgfxJapanMincho_36 ;
    extern const lgfx::U8g2font lgfxJapanMincho_40 ;
    extern const lgfx::U8g2font lgfxJapanMinchoP_8 ;
    extern const lgfx::U8g2font lgfxJapanMinchoP_12;
    extern const lgfx::U8g2font lgfxJapanMinchoP_16;
    extern const lgfx::U8g2font lgfxJapanMinchoP_20;
    extern const lgfx::U8g2font lgfxJapanMinchoP_24;
    extern const lgfx::U8g2font lgfxJapanMinchoP_28;
    extern const lgfx::U8g2font lgfxJapanMinchoP_32;
    extern const lgfx::U8g2font lgfxJapanMinchoP_36;
    extern const lgfx::U8g2font lgfxJapanMinchoP_40;
    extern const lgfx::U8g2font lgfxJapanGothic_8  ;
    extern const lgfx::U8g2font lgfxJapanGothic_12 ;
    extern const lgfx::U8g2font lgfxJapanGothic_16 ;
    extern const lgfx::U8g2font lgfxJapanGothic_20 ;
    extern const lgfx::U8g2font lgfxJapanGothic_24 ;
    extern const lgfx::U8g2font lgfxJapanGothic_28 ;
    extern const lgfx::U8g2font lgfxJapanGothic_32 ;
    extern const lgfx::U8g2font lgfxJapanGothic_36 ;
    extern const lgfx::U8g2font lgfxJapanGothic_40 ;
    extern const lgfx::U8g2font lgfxJapanGothicP_8 ;
    extern const lgfx::U8g2font lgfxJapanGothicP_12;
    extern const lgfx::U8g2font lgfxJapanGothicP_16;
    extern const lgfx::U8g2font lgfxJapanGothicP_20;
    extern const lgfx::U8g2font lgfxJapanGothicP_24;
    extern const lgfx::U8g2font lgfxJapanGothicP_28;
    extern const lgfx::U8g2font lgfxJapanGothicP_32;
    extern const lgfx::U8g2font lgfxJapanGothicP_36;
    extern const lgfx::U8g2font lgfxJapanGothicP_40;

    extern const lgfx::U8g2font efontCN_10   ;
    extern const lgfx::U8g2font efontCN_10_b ;
    extern const lgfx::U8g2font efontCN_10_bi;
    extern const lgfx::U8g2font efontCN_10_i ;
    extern const lgfx::U8g2font efontCN_12   ;
    extern const lgfx::U8g2font efontCN_12_b ;
    extern const lgfx::U8g2font efontCN_12_bi;
    extern const lgfx::U8g2font efontCN_12_i ;
    extern const lgfx::U8g2font efontCN_14   ;
    extern const lgfx::U8g2font efontCN_14_b ;
    extern const lgfx::U8g2font efontCN_14_bi;
    extern const lgfx::U8g2font efontCN_14_i ;
    extern const lgfx::U8g2font efontCN_16   ;
    extern const lgfx::U8g2font efontCN_16_b ;
    extern const lgfx::U8g2font efontCN_16_bi;
    extern const lgfx::U8g2font efontCN_16_i ;
    extern const lgfx::U8g2font efontCN_24   ;
    extern const lgfx::U8g2font efontCN_24_b ;
    extern const lgfx::U8g2font efontCN_24_bi;
    extern const lgfx::U8g2font efontCN_24_i ;

    extern const lgfx::U8g2font efontJA_10   ;
    extern const lgfx::U8g2font efontJA_10_b ;
    extern const lgfx::U8g2font efontJA_10_bi;
    extern const lgfx::U8g2font efontJA_10_i ;
    extern const lgfx::U8g2font efontJA_12   ;
    extern const lgfx::U8g2font efontJA_12_b ;
    extern const lgfx::U8g2font efontJA_12_bi;
    extern const lgfx::U8g2font efontJA_12_i ;
    extern const lgfx::U8g2font efontJA_14   ;
    extern const lgfx::U8g2font efontJA_14_b ;
    extern const lgfx::U8g2font efontJA_14_bi;
    extern const lgfx::U8g2font efontJA_14_i ;
    extern const lgfx::U8g2font efontJA_16   ;
    extern const lgfx::U8g2font efontJA_16_b ;
    extern const lgfx::U8g2font efontJA_16_bi;
    extern const lgfx::U8g2font efontJA_16_i ;
    extern const lgfx::U8g2font efontJA_24   ;
    extern const lgfx::U8g2font efontJA_24_b ;
    extern const lgfx::U8g2font efontJA_24_bi;
    extern const lgfx::U8g2font efontJA_24_i ;

    extern const lgfx::U8g2font efontKR_10   ;
    extern const lgfx::U8g2font efontKR_10_b ;
    extern const lgfx::U8g2font efontKR_10_bi;
    extern const lgfx::U8g2font efontKR_10_i ;
    extern const lgfx::U8g2font efontKR_12   ;
    extern const lgfx::U8g2font efontKR_12_b ;
    extern const lgfx::U8g2font efontKR_12_bi;
    extern const lgfx::U8g2font efontKR_12_i ;
    extern const lgfx::U8g2font efontKR_14   ;
    extern const lgfx::U8g2font efontKR_14_b ;
    extern const lgfx::U8g2font efontKR_14_bi;
    extern const lgfx::U8g2font efontKR_14_i ;
    extern const lgfx::U8g2font efontKR_16   ;
    extern const lgfx::U8g2font efontKR_16_b ;
    extern const lgfx::U8g2font efontKR_16_bi;
    extern const lgfx::U8g2font efontKR_16_i ;
    extern const lgfx::U8g2font efontKR_24   ;
    extern const lgfx::U8g2font efontKR_24_b ;
    extern const lgfx::U8g2font efontKR_24_bi;
    extern const lgfx::U8g2font efontKR_24_i ;

    extern const lgfx::U8g2font efontTW_10   ;
    extern const lgfx::U8g2font efontTW_10_b ;
    extern const lgfx::U8g2font efontTW_10_bi;
    extern const lgfx::U8g2font efontTW_10_i ;
    extern const lgfx::U8g2font efontTW_12   ;
    extern const lgfx::U8g2font efontTW_12_b ;
    extern const lgfx::U8g2font efontTW_12_bi;
    extern const lgfx::U8g2font efontTW_12_i ;
    extern const lgfx::U8g2font efontTW_14   ;
    extern const lgfx::U8g2font efontTW_14_b ;
    extern const lgfx::U8g2font efontTW_14_bi;
    extern const lgfx::U8g2font efontTW_14_i ;
    extern const lgfx::U8g2font efontTW_16   ;
    extern const lgfx::U8g2font efontTW_16_b ;
    extern const lgfx::U8g2font efontTW_16_bi;
    extern const lgfx::U8g2font efontTW_16_i ;
    extern const lgfx::U8g2font efontTW_24   ;
    extern const lgfx::U8g2font efontTW_24_b ;
    extern const lgfx::U8g2font efontTW_24_bi;
    extern const lgfx::U8g2font efontTW_24_i ;

*/
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


  #define DEFAULT_FONT        fonts::Font0
  #define DEFAULT_TEXT_COLOR  WHITE
  #define ERROR_TEXT_COLOR    RED
  #define DEFAULT_BG_COLOR    BLACK

  #define SPLASH_COLOR_1    WHITE
  #define SPLASH_COLOR_2    RED
  #define SPLASH_FONT       fonts::FreeMonoBold12pt7b
  #define SPLASH_FONT_DX    14
  #define SPLASH_FONT_DY    12
  #define SPLASH_FONT_SIZE  2
  #define SPLASH_WIDTH      280
  #define SPLASH_HEIGHT     120
  #define SPLASH_ROUNDNESS  (SPLASH_HEIGHT/3)
  #define SPLASH_X          ((320 - SPLASH_WIDTH)/2)
  #define SPLASH_Y          ((240 - SPLASH_HEIGHT)/2)
  #define SPLASH_TEXT_1_DX  15
  #define SPLASH_TEXT_1_DY  38
  #define SPLASH_TEXT_2_DX  (SPLASH_TEXT_1_DX+5*(SPLASH_FONT_DX*SPLASH_FONT_SIZE))
  #define SPLASH_TEXT_2_DY  38
  #define SPLASH_DIAG_DX    40  

#endif