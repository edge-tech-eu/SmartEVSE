#include "Button.h"

void button_t::setup(const button_shape_t& r, const char *unchecked_text, const char *checked_text, bool checked,
                     uint16_t frame_color, uint16_t back_color, uint16_t thumb_color) {
    setupPosition(r);
    setupValue(unchecked_text, checked_text, checked);
    setupColor(frame_color, back_color, thumb_color);
}

void button_t::setupPosition(const button_shape_t& r) {
    _rect = r;
}

void button_t::setupValue(const char *unchecked_text, const char *checked_text, bool checked) {
    _unchecked_text = unchecked_text;
    _checked_text = checked_text;
    _current_value = checked;
}

void button_t::setupColor(uint16_t frame_color, uint16_t back_color, uint16_t thumb_color) {
    _frame_color = frame_color;
    _back_color  = back_color;
    _thumb_color = thumb_color;
}

void button_t::draw(LovyanGFX* gfx) {
    //gfx->startWrite();
    draw_frame(gfx);
    draw_label(gfx);
    //gfx->endWrite();
}


bool button_t::update(m5::touch_detail_t& td, LovyanGFX* gfx) {
    _was_changed = false;

    if (!_rect.contain(td.base)) {
        return false;
    }

    if (td.wasReleased()) {
        _current_value = !_current_value;
        _was_changed = true;
        gfx->startWrite();
        draw_label();
        gfx->endWrite();
        return true;
    }

    return false;
}

bool button_t::getValue(void) const {
    return _current_value;
}

bool button_t::wasChanged(void) const {
    return _was_changed;
}

bool button_t::value_update(bool new_value, LovyanGFX* gfx) {
    
    if (_current_value == new_value) {
        return false;
    }

    _current_value = new_value;

    return true;
}

void button_t::draw_frame(LovyanGFX* gfx) {
    gfx->fillRoundRect(_rect.x + 1, _rect.y + 1, _rect.w - 2, _rect.h - 2, 2, _back_color);
    gfx->drawRoundRect(_rect.x, _rect.y, _rect.w, _rect.h, 3, _frame_color);
}

void button_t::draw_label(LovyanGFX* gfx) {
    gfx->drawString((_current_value?_checked_text:_unchecked_text), _rect.x + 7,  _rect.y + 5);
}
