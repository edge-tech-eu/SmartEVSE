#pragma once

#include <M5Unified.h>
#include <stdint.h>

struct rect_t {
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

class slider_t {
public:
    void setup(const rect_t& r, int min_value, int max_value, int value,
               uint16_t frame_color = 0xFFFF, uint16_t back_color = 0, uint16_t thumb_color = 0xFFE0);

    void setupPosition(const rect_t& r);
    void setupValue(int min_value, int max_value, int value);
    void setupColor(uint16_t frame_color, uint16_t back_color, uint16_t thumb_color);

    void draw(LovyanGFX* gfx = &M5.Display);

    bool update(m5::touch_detail_t& td, LovyanGFX* gfx = &M5.Display);

    int getValue(void) const;
    bool wasChanged(void) const;

protected:
    bool value_update(int new_value, LovyanGFX* gfx = &M5.Display);
    int calc_pos(int value);
    void draw_frame(LovyanGFX* gfx = &M5.Display);
    void draw_thumb(int pos, uint16_t color, LovyanGFX* gfx = &M5.Display);

private:
    rect_t _rect;
    int _min_value;
    int _max_value;
    int _current_value;
    int _control_value;

    uint16_t _frame_color = 0xFFFF;
    uint16_t _back_color   = 0;
    uint16_t _thumb_color  = 0xFFE0;

    bool _vertical     = false;
    bool _was_changed  = false;
};