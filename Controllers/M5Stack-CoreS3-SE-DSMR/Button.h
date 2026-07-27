#pragma once

#include <M5Unified.h>
#include <stdint.h>

struct button_shape_t {
    int x;
    int y;
    int w;
    int h;

    bool contain(int x, int y) {
        return this->x <= x && x < (this->x + this->w)
            && this->y <= y && y < (this->y + this->h);
    }

    bool contain(m5::Touch_Class::point_t& p) {
        return contain(p.x, p.y);
    }
};

class button_t {
public:
    void setup(const button_shape_t& r, const char *unchecked_text, const char *checked_text, bool checked,
               uint16_t frame_color = 0xFFFF, uint16_t back_color = 0, uint16_t thumb_color = 0xFFE0);

    void setupPosition(const button_shape_t& r);
    void setupValue(const char *unchecked_text, const char *checked_text, bool checked);
    void setupColor(uint16_t frame_color, uint16_t back_color, uint16_t thumb_color);

    void draw(LovyanGFX* gfx = &M5.Display);

    bool update(m5::touch_detail_t& td, LovyanGFX* gfx = &M5.Display);

    bool getValue(void) const;
    bool wasChanged(void) const;
    
    bool value_update(bool new_value, LovyanGFX* gfx = &M5.Display);

protected:
    
    //int calc_pos(int value);
    void draw_frame(LovyanGFX* gfx = &M5.Display);
    void draw_label(LovyanGFX* gfx = &M5.Display);
    //void draw_thumb(int pos, uint16_t color, LovyanGFX* gfx = &M5.Display);

private:
    button_shape_t _rect;
    const char *_unchecked_text;
    const char *_checked_text;
    bool _current_value;

    uint16_t _frame_color = 0xFFFF;
    uint16_t _back_color   = 0;
    uint16_t _thumb_color  = 0xFFE0;

    //bool _vertical     = false;
    bool _was_changed  = false;
};