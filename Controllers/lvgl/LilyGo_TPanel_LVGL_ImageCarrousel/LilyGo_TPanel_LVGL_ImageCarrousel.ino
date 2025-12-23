#define TOUCH_MODULES_CST_MUTUAL

// this needs esp32 bsp 2.x (3.x does not compile)

#include "lvgl.h"
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

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;


typedef struct
{
  
	lv_obj_t *screen;
	bool screen_del;
	lv_obj_t *screen_label_1;
	lv_obj_t *screen_carousel_1;
	lv_obj_t *screen_carousel_1_element_1;
	lv_obj_t *screen_carousel_1_element_2;
	lv_obj_t *screen_carousel_1_element_3;
	lv_obj_t *screen_carousel_1_element_4;
	lv_obj_t *screen_img_1;
	lv_obj_t *screen_img_2;
	lv_obj_t *screen_img_3;
	lv_obj_t *screen_img_4;
} lv_ui;

lv_ui guider_ui;

void ui_init_style(lv_style_t * style);
void init_scr_del_flag(lv_ui *ui);
void setup_ui(lv_ui *ui);

void setup_scr_screen(lv_ui *ui);
LV_IMG_DECLARE(_1_alpha_240x400);
LV_IMG_DECLARE(_2_alpha_240x400);
LV_IMG_DECLARE(_3_alpha_240x400);
LV_IMG_DECLARE(_4_alpha_240x400);

LV_FONT_DECLARE(lv_font_arial_30)
LV_FONT_DECLARE(lv_font_montserratMedium_16)


void ui_init_style(lv_style_t * style) {

  if (style->prop_cnt > 1)
    lv_style_reset(style);
  else
    lv_style_init(style);
}

void init_scr_del_flag(lv_ui *ui) {
  
	ui->screen_del = true;
}

void setup_ui(lv_ui *ui) {

  init_scr_del_flag(ui);
  setup_scr_screen(ui);
  lv_scr_load(ui->screen);
}



void setup_scr_screen(lv_ui *ui) {

	//Write codes screen
	ui->screen = lv_obj_create(NULL);
	lv_obj_set_size(ui->screen, 480, 480);
	lv_obj_set_scrollbar_mode(ui->screen, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_label_1
	ui->screen_label_1 = lv_label_create(ui->screen);
	lv_label_set_text(ui->screen_label_1, "LVGL");
	lv_label_set_long_mode(ui->screen_label_1, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->screen_label_1, 167, 13);
	lv_obj_set_size(ui->screen_label_1, 146, 33);
	lv_obj_set_scrollbar_mode(ui->screen_label_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_label_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_label_1, &lv_font_arial_30, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_label_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_carousel_1
	ui->screen_carousel_1 = lv_carousel_create(ui->screen);
	lv_carousel_set_element_width(ui->screen_carousel_1, 300);
	lv_obj_set_size(ui->screen_carousel_1, 480, 430);
	ui->screen_carousel_1_element_1 = lv_carousel_add_element(ui->screen_carousel_1, 0);
	ui->screen_carousel_1_element_2 = lv_carousel_add_element(ui->screen_carousel_1, 1);
	ui->screen_carousel_1_element_3 = lv_carousel_add_element(ui->screen_carousel_1, 2);
	ui->screen_carousel_1_element_4 = lv_carousel_add_element(ui->screen_carousel_1, 3);
	lv_obj_set_pos(ui->screen_carousel_1, 0, 50);
	lv_obj_set_size(ui->screen_carousel_1, 480, 430);
	lv_obj_set_scrollbar_mode(ui->screen_carousel_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_carousel_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_carousel_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_carousel_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_carousel_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_carousel_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_carousel_1, Part: LV_PART_SCROLLBAR, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_carousel_1, 0, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_carousel_1, 0, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_DEFAULT for &style_screen_carousel_1_extra_element_items_default
	static lv_style_t style_screen_carousel_1_extra_element_items_default;
	ui_init_style(&style_screen_carousel_1_extra_element_items_default);
	
	lv_style_set_bg_opa(&style_screen_carousel_1_extra_element_items_default, 0);
	lv_style_set_outline_width(&style_screen_carousel_1_extra_element_items_default, 0);
	lv_style_set_border_width(&style_screen_carousel_1_extra_element_items_default, 0);
	lv_style_set_radius(&style_screen_carousel_1_extra_element_items_default, 5);
	lv_style_set_shadow_width(&style_screen_carousel_1_extra_element_items_default, 0);
	lv_obj_add_style(ui->screen_carousel_1_element_4, &style_screen_carousel_1_extra_element_items_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_carousel_1_element_3, &style_screen_carousel_1_extra_element_items_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_carousel_1_element_2, &style_screen_carousel_1_extra_element_items_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_carousel_1_element_1, &style_screen_carousel_1_extra_element_items_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_FOCUSED for &style_screen_carousel_1_extra_element_items_focused
	static lv_style_t style_screen_carousel_1_extra_element_items_focused;
	ui_init_style(&style_screen_carousel_1_extra_element_items_focused);
	
	lv_style_set_bg_opa(&style_screen_carousel_1_extra_element_items_focused, 0);
	lv_style_set_outline_width(&style_screen_carousel_1_extra_element_items_focused, 0);
	lv_style_set_border_width(&style_screen_carousel_1_extra_element_items_focused, 0);
	lv_style_set_radius(&style_screen_carousel_1_extra_element_items_focused, 5);
	lv_style_set_shadow_width(&style_screen_carousel_1_extra_element_items_focused, 0);
	lv_obj_add_style(ui->screen_carousel_1_element_4, &style_screen_carousel_1_extra_element_items_focused, LV_PART_MAIN|LV_STATE_FOCUSED);
	lv_obj_add_style(ui->screen_carousel_1_element_3, &style_screen_carousel_1_extra_element_items_focused, LV_PART_MAIN|LV_STATE_FOCUSED);
	lv_obj_add_style(ui->screen_carousel_1_element_2, &style_screen_carousel_1_extra_element_items_focused, LV_PART_MAIN|LV_STATE_FOCUSED);
	lv_obj_add_style(ui->screen_carousel_1_element_1, &style_screen_carousel_1_extra_element_items_focused, LV_PART_MAIN|LV_STATE_FOCUSED);



	//Write codes screen_img_1
	ui->screen_img_1 = lv_img_create(ui->screen_carousel_1_element_1);
	lv_obj_add_flag(ui->screen_img_1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_img_1, &_1_alpha_240x400);
	lv_img_set_pivot(ui->screen_img_1, 50,50);
	lv_img_set_angle(ui->screen_img_1, 0);
	lv_obj_set_pos(ui->screen_img_1, 30, 1);
	lv_obj_set_size(ui->screen_img_1, 240, 400);
	lv_obj_set_scrollbar_mode(ui->screen_img_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->screen_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);



	//Write codes screen_img_2
	ui->screen_img_2 = lv_img_create(ui->screen_carousel_1_element_2);
	lv_obj_add_flag(ui->screen_img_2, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_img_2, &_2_alpha_240x400);
	lv_img_set_pivot(ui->screen_img_2, 50,50);
	lv_img_set_angle(ui->screen_img_2, 0);
	lv_obj_set_pos(ui->screen_img_2, 30, 1);
	lv_obj_set_size(ui->screen_img_2, 240, 400);
	lv_obj_set_scrollbar_mode(ui->screen_img_2, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->screen_img_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);



	//Write codes screen_img_3
	ui->screen_img_3 = lv_img_create(ui->screen_carousel_1_element_3);
	lv_obj_add_flag(ui->screen_img_3, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_img_3, &_3_alpha_240x400);
	lv_img_set_pivot(ui->screen_img_3, 50,50);
	lv_img_set_angle(ui->screen_img_3, 0);
	lv_obj_set_pos(ui->screen_img_3, 30, 1);
	lv_obj_set_size(ui->screen_img_3, 240, 400);
	lv_obj_set_scrollbar_mode(ui->screen_img_3, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_img_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->screen_img_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);



	//Write codes screen_img_4
	ui->screen_img_4 = lv_img_create(ui->screen_carousel_1_element_4);
	lv_obj_add_flag(ui->screen_img_4, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_img_4, &_4_alpha_240x400);
	lv_img_set_pivot(ui->screen_img_4, 50,50);
	lv_img_set_angle(ui->screen_img_4, 0);
	lv_obj_set_pos(ui->screen_img_4, 30, 1);
	lv_obj_set_size(ui->screen_img_4, 240, 400);
	lv_obj_set_scrollbar_mode(ui->screen_img_4, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_img_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->screen_img_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Update current screen layout.
	lv_obj_update_layout(ui->screen);

	
}
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

    setup_ui(&guider_ui);
}

void loop() {

    lv_timer_handler();
    delay(5);
}