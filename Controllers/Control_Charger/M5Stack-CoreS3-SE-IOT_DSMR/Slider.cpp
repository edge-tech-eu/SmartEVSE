#include "Slider.h"

void slider_t::setup(const rect_t& r, int min_value, int max_value, int value,
                     uint16_t frame_color, uint16_t back_color, uint16_t thumb_color) {
    setupPosition(r);
    setupValue(min_value, max_value, value);
    setupColor(frame_color, back_color, thumb_color);
}

void slider_t::setupPosition(const rect_t& r) {
    _rect = r;
    _vertical = r.h > r.w;
}

void slider_t::setupValue(int min_value, int max_value, int value) {
    _min_value = min_value;
    _max_value = max_value;
    _current_value = value;
    _control_value = value;
}

void slider_t::setupColor(uint16_t frame_color, uint16_t back_color, uint16_t thumb_color) {
    _frame_color = frame_color;
    _back_color  = back_color;
    _thumb_color = thumb_color;
}

void slider_t::draw(LovyanGFX* gfx) {
    gfx->startWrite();
    draw_frame(gfx);
    int pos = calc_pos(_current_value);
    draw_thumb(pos, _thumb_color, gfx);
    gfx->endWrite();
}

bool slider_t::update(m5::touch_detail_t& td, LovyanGFX* gfx) {
    _was_changed = false;

    if (!_rect.contain(td.base)) return false;

    if (td.wasReleased()) {
        _current_value = _control_value;
    }

    if (td.isPressed()) {
        int rw = _vertical ? _rect.h : _rect.w;
        int tmp = (_current_value - _min_value) * rw / (_max_value - _min_value);
        tmp += _vertical ? td.distanceY() : td.distanceX();

        int v = _min_value + (_max_value - _min_value) * tmp / rw;
        _was_changed = value_update(v, gfx);
        return true;
    }

    return false;
}

int slider_t::getValue(void) const {
    return _control_value;
}

bool slider_t::wasChanged(void) const {
    return _was_changed;
}

bool slider_t::value_update(int new_value, LovyanGFX* gfx) {
    int min_v = _min_value;
    int max_v = _max_value;
    if (min_v > max_v) {
        min_v = _max_value;
        max_v = _min_value;
    }

    new_value = (new_value < min_v) ? min_v : (new_value > max_v) ? max_v : new_value;

    if (_control_value == new_value) {
        return false;
    }

    int prev_pos = calc_pos(_control_value);
    int new_pos  = calc_pos(new_value);

    _control_value = new_value;

    if (prev_pos != new_pos) {
        gfx->startWrite();
        draw_thumb(prev_pos, _back_color, gfx);
        draw_thumb(new_pos,  _thumb_color, gfx);
        gfx->endWrite();
    }

    return true;
}

int slider_t::calc_pos(int value) {
    int diff = _max_value - _min_value;
    if (diff == 0) return 0;

    value -= _min_value;
    int w = _vertical ? (_rect.h - _rect.w) : (_rect.w - _rect.h);
    return (w * value) / diff;
}

void slider_t::draw_frame(LovyanGFX* gfx) {
    gfx->fillRoundRect(_rect.x + 1, _rect.y + 1, _rect.w - 2, _rect.h - 2, 2, _back_color);
    gfx->drawRoundRect(_rect.x, _rect.y, _rect.w, _rect.h, 3, _frame_color);
}

void slider_t::draw_thumb(int pos, uint16_t color, LovyanGFX* gfx) {
    int rx = _rect.x + 1;
    int ry = _rect.y + 1;
    int rw;

    if (_vertical) {
        ry += pos;
        rw = _rect.w;
    } else {
        rx += pos;
        rw = _rect.h;
    }

    rw -= 2;
    gfx->fillRoundRect(rx, ry, rw, rw, 3, color);
}