#define TOUCH_MODULES_CST_MUTUAL

// this needs esp32 bsp 2.x (3.x does not compile)

#include "lvgl.h"
#include "misc/lv_color.h"
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "TouchLib.h"

volatile bool Touch_Int_Flag = false;



TouchLib touch(Wire, TOUCH_SDA, TOUCH_SCL, CST3240_ADDRESS);

Arduino_DataBus *bus = new Arduino_XL9535SWSPI(IIC_SDA /* SDA */, IIC_SCL /* SCL */, -1 /* XL PWD */,
                                               XL95X5_CS /* XL CS */, XL95X5_SCLK /* XL SCK */, XL95X5_MOSI /* XL MOSI */);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    -1 /* DE */, LCD_VSYNC /* VSYNC */, LCD_HSYNC /* HSYNC */, LCD_PCLK /* PCLK */,
    LCD_B0 /* B0 */, LCD_B1 /* B1 */, LCD_B2 /* B2 */, LCD_B3 /* B3 */, LCD_B4 /* B4 */,
    LCD_G0 /* G0 */, LCD_G1 /* G1 */, LCD_G2 /* G2 */, LCD_G3 /* G3 */, LCD_G4 /* G4 */, LCD_G5 /* G5 */,
    LCD_R0 /* R0 */, LCD_R1 /* R1 */, LCD_R2 /* R2 */, LCD_R3 /* R3 */, LCD_R4 /* R4 */,
    1 /* hsync_polarity */, 20 /* hsync_front_porch */, 2 /* hsync_pulse_width */, 0 /* hsync_back_porch */,
    1 /* vsync_polarity */, 30 /* vsync_front_porch */, 8 /* vsync_pulse_width */, 1 /* vsync_back_porch */,
    10 /* pclk_active_neg */, 6000000L /* prefer_speed */, false /* useBigEndian */,
    0 /* de_idle_high*/, 0 /* pclk_idle_high */);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    LCD_WIDTH /* width */, LCD_HEIGHT /* height */, rgbpanel, 0 /* rotation */, true /* auto_flush */,
    bus, -1 /* RST */, st7701_type9_init_operations, sizeof(st7701_type9_init_operations));

static lv_obj_t * label;
static int value;
unsigned long next_time;
int v =10;
static int button;
static lv_obj_t * slider;
static lv_obj_t * btn1;
static lv_obj_t * btn3;
lv_obj_t * label_c_1;

static void btn_event_cb(lv_event_t * e) {

    lv_event_code_t code = lv_event_get_code(e);
    //lv_obj_t * btn = lv_event_get_target_obj(e);
		lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        

        /*Get the first child of the button which is the label and change its text*/
        //lv_obj_t * label = lv_obj_get_child(btn, 0);

        if(button == 1) {
            button = 0;
            lv_label_set_text_fmt(label, "%d", 0);
            lv_slider_set_value(slider, 0, LV_ANIM_ON);
        } else {
            button = 1;
            lv_label_set_text_fmt(label, "%d", value);

            lv_slider_set_value(slider, value, LV_ANIM_ON);
        }
    }
}

static void event_handler_btn1(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
       Serial.printf("1\r\n");
       bool is_checked = lv_obj_has_state(btn1, LV_STATE_CHECKED);
       if(is_checked) {
lv_obj_clear_state(btn3, LV_STATE_CHECKED); 
       } else {
        lv_obj_add_state(btn3, LV_STATE_CHECKED); 
    }
    }
}

static void event_handler_btn3(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
       Serial.printf("3\r\n");
       bool is_checked = lv_obj_has_state(btn3, LV_STATE_CHECKED);
       if(is_checked) {
lv_obj_clear_state(btn1, LV_STATE_CHECKED); 
       } else {
        lv_obj_add_state(btn1, LV_STATE_CHECKED); 
    }
     /*  int v = lv_btn_get_value(btn3);
       if(v==0) {
       lv_btn_set_value(btn1, 1);   
       } else {
        lv_btn_set_value(btn1, 0);   
       }*/
    }
}


static void slider_event_cb(lv_event_t * e)
{
    
    //lv_obj_t * slider = lv_event_get_target(e);

    /*Refresh the text*/
    //lv_label_set_text_fmt(label, "%" LV_PRId32, lv_slider_get_value(slider));
    //lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);    /*Align top of the slider*/
    if(button == 1) {
lv_label_set_text_fmt(label, "%d", lv_slider_get_value(slider));
    }
   // lv_label_set_text_fmt(label, "Button: %d", lv_slider_get_value(slider));
}



static lv_style_t sl;

void lv_example_get_started_2(void) {

    lv_obj_t * scr = lv_scr_act();
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

// button
    lv_obj_t * btn = lv_btn_create(scr);     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/


    btn1 = lv_btn_create(scr);  
    lv_obj_add_event_cb(btn1, event_handler_btn1, LV_EVENT_ALL, NULL);
    lv_obj_set_pos(btn1, 100, 100);   
    lv_obj_set_size(btn1, 30, 30);       
     // lv_btn_set_value(btn1, 0);  
     lv_obj_add_state(btn1, LV_STATE_CHECKED);            
    lv_obj_add_flag(btn1, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_t * label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "1");
    lv_obj_center(label1);

    btn3 = lv_btn_create(scr);  
    lv_obj_add_event_cb(btn3, event_handler_btn3, LV_EVENT_ALL, NULL);
    lv_obj_set_pos(btn3, 140, 100);   
    lv_obj_set_size(btn3, 30, 30);  
    //lv_obj_set_value(btn3, 1);      
    
    lv_obj_clear_state(btn3, LV_STATE_CHECKED);       
    lv_obj_add_flag(btn3, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_t * label3 = lv_label_create(btn3);
    lv_label_set_text(label3, "3");
    lv_obj_center(label3);

/*Create a slider in the center of the display*/
    slider = lv_slider_create(scr);
    lv_slider_set_range(slider, 0, 32); 

    lv_slider_set_value(slider, value, LV_ANIM_ON);
    lv_obj_set_width(slider, 200);                          /*Set the width*/
    lv_obj_center(slider);                                  /*Align to the center of the parent (screen)*/
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);     /*Assign an event function*/

// label
    //lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    label = lv_label_create(scr);          /*Add a label to the button*/
   // lv_label_set_text(label, "0");                     /*Set the labels text*/

lv_label_set_text_fmt(label, "%d", value);
   lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);  

#define STAT_X     240
#define STAT_Y      10

#define STAT_L1_DX  32
#define STAT_L2_DX  50
#define STAT_L3_DX  50
#define STAT_L4_DX  80

#define STAT_R1_DY  30
#define STAT_R2_DY  20
#define STAT_R3_DY  20
#define STAT_R4_DY  30
#define STAT_R5_DY  20

#define STAT_R_V_DX 18
#define STAT_R_S_DX  2
#define STAT_R_A_DX 30

#define STAT_R_VV_DX 18
#define STAT_R_SS_DX  2
#define STAT_R_AA_DX 30


#define STAT_HEADER_ALIGN   LV_TEXT_ALIGN_LEFT
#define STAT_LABEL_ALIGN    LV_TEXT_ALIGN_LEFT



lv_obj_t * l;

// header row
// no label
// ev
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX, STAT_Y);  
lv_obj_set_width(l,STAT_L2_DX); 
lv_obj_set_style_text_align(l, STAT_HEADER_ALIGN, 0);
lv_label_set_text(l, "EV");
// home
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX, STAT_Y);  
lv_obj_set_width(l,STAT_L3_DX); 
lv_obj_set_style_text_align(l, STAT_HEADER_ALIGN, 0);
lv_label_set_text(l, "Home");
//voltage
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX+STAT_L3_DX, STAT_Y);  
lv_obj_set_width(l,STAT_L4_DX); 
lv_obj_set_style_text_align(l, STAT_HEADER_ALIGN, 0);
lv_label_set_text(l, "Voltage");

// l1 row
// l1
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X,STAT_Y+STAT_R1_DY);  
lv_obj_set_width(l,STAT_L1_DX); 
lv_obj_set_style_text_align(l, STAT_LABEL_ALIGN, 0);
lv_label_set_text(l, "L1");
// value
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX, STAT_Y+STAT_R1_DY);  
lv_obj_set_width(l,STAT_R_V_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
lv_label_set_text(l, "0");
// a
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_R_V_DX+STAT_R_S_DX, STAT_Y+STAT_R1_DY);  
lv_obj_set_width(l,STAT_R_A_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_LEFT, 0);
lv_label_set_text(l, "A");
// value
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX, STAT_Y+STAT_R1_DY);  
lv_obj_set_width(l,STAT_R_V_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
lv_label_set_text(l, "0");
// a
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX+STAT_R_V_DX+STAT_R_S_DX, STAT_Y+STAT_R1_DY);  
lv_obj_set_width(l,STAT_R_A_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_LEFT, 0);
lv_label_set_text(l, "A");
// value
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX+STAT_L3_DX, STAT_Y+STAT_R1_DY);  
lv_obj_set_width(l,STAT_R_V_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
lv_label_set_text(l, "0");
// a
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX+STAT_L3_DX+STAT_R_V_DX+STAT_R_S_DX, STAT_Y+STAT_R1_DY);  
lv_obj_set_width(l,STAT_R_A_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_LEFT, 0);
lv_label_set_text(l, "V");

// l2 row
// l2
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X,STAT_Y+STAT_R1_DY+STAT_R2_DY);  
lv_obj_set_width(l,STAT_L1_DX); 
lv_obj_set_style_text_align(l, STAT_LABEL_ALIGN, 0);
lv_label_set_text(l, "L2");
// value
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY);  
lv_obj_set_width(l,STAT_R_V_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
lv_label_set_text(l, "0");
// a
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_R_V_DX+STAT_R_S_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY);  
lv_obj_set_width(l,STAT_R_A_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_LEFT, 0);
lv_label_set_text(l, "A");
// value
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY);  
lv_obj_set_width(l,STAT_R_V_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
lv_label_set_text(l, "0");
// a
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX+STAT_R_V_DX+STAT_R_S_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY);  
lv_obj_set_width(l,STAT_R_A_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_LEFT, 0);
lv_label_set_text(l, "A");
// value
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX+STAT_L3_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY);  
lv_obj_set_width(l,STAT_R_V_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
lv_label_set_text(l, "0");
// a
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX+STAT_L3_DX+STAT_R_V_DX+STAT_R_S_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY);  
lv_obj_set_width(l,STAT_R_A_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_LEFT, 0);
lv_label_set_text(l, "V");

// l3 row
// l3
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X,STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY);  
lv_obj_set_width(l,STAT_L1_DX); 
lv_obj_set_style_text_align(l, STAT_LABEL_ALIGN, 0);
lv_label_set_text(l, "L3");
// value
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY);  
lv_obj_set_width(l,STAT_R_V_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
lv_label_set_text(l, "0");
// a
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_R_V_DX+STAT_R_S_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY);  
lv_obj_set_width(l,STAT_R_A_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_LEFT, 0);
lv_label_set_text(l, "A");
// value
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY);  
lv_obj_set_width(l,STAT_R_V_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
lv_label_set_text(l, "0");
// a
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX+STAT_R_V_DX+STAT_R_S_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY);  
lv_obj_set_width(l,STAT_R_A_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_LEFT, 0);
lv_label_set_text(l, "A");
// value
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX+STAT_L3_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY);  
lv_obj_set_width(l,STAT_R_V_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
lv_label_set_text(l, "0");
// v
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX+STAT_L3_DX+STAT_R_V_DX+STAT_R_S_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY);  
lv_obj_set_width(l,STAT_R_A_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_LEFT, 0);
lv_label_set_text(l, "V");


// p row
// p
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X,STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY+STAT_R4_DY);  
lv_obj_set_width(l,STAT_L1_DX); 
lv_obj_set_style_text_align(l, STAT_LABEL_ALIGN, 0);
lv_label_set_text(l, "P");
// value
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY+STAT_R4_DY);  
lv_obj_set_width(l,STAT_R_VV_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
lv_label_set_text(l, "0");
// kw
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_R_VV_DX+STAT_R_SS_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY+STAT_R4_DY);  
lv_obj_set_width(l,STAT_R_AA_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_LEFT, 0);
lv_label_set_text(l, "kW");
// value
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY+STAT_R4_DY);  
lv_obj_set_width(l,STAT_R_VV_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
lv_label_set_text(l, "0");
// kw
l = lv_label_create(scr);
lv_obj_set_pos(l, STAT_X+STAT_L1_DX+STAT_L2_DX+STAT_R_VV_DX+STAT_R_SS_DX, STAT_Y+STAT_R1_DY+STAT_R2_DY+STAT_R3_DY+STAT_R4_DY);  
lv_obj_set_width(l,STAT_R_AA_DX); 
lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_LEFT, 0);
lv_label_set_text(l, "kW");

// debug bg color
lv_color_t bg =LV_COLOR_MAKE(0xF4, 0x43, 0x36);
lv_style_init(&sl);
lv_style_set_bg_color(&sl, bg); 
lv_obj_add_style(l,&sl,LV_PART_MAIN);

// value
// empty
// v
// empty



label_c_1 = lv_label_create(scr);
lv_obj_set_pos(label_c_1, 240, 300);
lv_label_set_text_fmt(label_c_1, "L1: %d",10);
lv_obj_set_style_text_align(label_c_1, LV_TEXT_ALIGN_RIGHT, 0);
//lv_label_set_long_mode(label_c_1, LV_LABEL_LONG_WRAP);
lv_obj_set_width(label_c_1,58);
//static lv_style_t style_label_c_1;
//lv_style_init(&style_label_c_1);
//lv_obj_add_style(label_c_1,&style_label_c_1,LV_TEXT_ALIGN_RIGHT);




//update
    lv_obj_update_layout(scr);
}

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;


/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {

    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);

    lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {

    if (Touch_Int_Flag == true) {

        touch.read();
        TP_Point t = touch.getPoint(0);

        if ((touch.getPointNum() == 1) && (t.pressure > 0) && (t.state != 0)) {

            data->state = LV_INDEV_STATE_PR;

            /*Set the coordinates*/
            data->point.x = t.x;
            data->point.y = t.y;

            // Serial.printf("Touch X: %d Y: %d", t.x, t.y);
            // Serial.printf("Static: %d", t.state);
            // Serial.printf("Pressure: %d", t.pressure);
        }

        Touch_Int_Flag = false;

    } else {

        data->state = LV_INDEV_STATE_REL;
    }
}

void lvgl_initialization(void) {

    lv_init();

    uint32_t screen_width = gfx->width();
    uint32_t screen_height = gfx->height();

    lv_color_t *buf_1 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * LCD_WIDTH * 40, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    lv_color_t *buf_2 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * LCD_WIDTH * 40, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

    while ((!buf_1) || (!buf_2)) {

        Serial.println("LVGL disp_draw_buf allocate failed!");
        delay(1000);
    }

    lv_disp_draw_buf_init(&draw_buf, buf_1, buf_2, LCD_WIDTH * 40);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = screen_width;
    disp_drv.ver_res = screen_height;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 1;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
}

void setup() {

    Serial.begin(115200);
    
    Serial.printf("\r\nLiliGo  T-Panel LVGL\r\n\r\n");

    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);

    attachInterrupt(
        TOUCH_INT,
        []
        {
            Touch_Int_Flag = true;
        },
        FALLING); 

    Wire.begin(IIC_SDA, IIC_SCL);

    gfx->begin();
    gfx->fillScreen(BLACK);

    gfx->XL_digitalWrite(XL95X5_TOUCH_RST, LOW);
    delay(200);
    gfx->XL_digitalWrite(XL95X5_TOUCH_RST, HIGH);
    delay(200);

    touch.init();
    
    lvgl_initialization();

button = 1;
value = 15;

    //setup_ui(&guider_ui);
		lv_example_get_started_2();

		Serial.printf("init done\r\n");

        next_time=millis()+1000;
}



void loop() {

    unsigned long now=millis();

    lv_timer_handler();
    delay(5);

    if(now>next_time) {

        next_time = now + 1000;

        lv_label_set_text_fmt(label_c_1, "%d",v++);
    }
}