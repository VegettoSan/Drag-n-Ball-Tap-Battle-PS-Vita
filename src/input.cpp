#include "input.hpp"
#include "log.hpp"
#include <algorithm>
#include <psp2/ctrl.h>
#include <psp2/touch.h>

VitaInput::VitaInput() {
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    SceCtrlData pad{};
    if (sceCtrlPeekBufferPositive(0, &pad, 1) > 0) {
        previous_buttons_ = pad.buttons;
        previous_up_ = (pad.buttons & SCE_CTRL_UP) || pad.ly < 80;
        previous_down_ = (pad.buttons & SCE_CTRL_DOWN) || pad.ly > 176;
    }
    SceTouchPanelInfo info{};
    touch_ready_ = sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START) >= 0 &&
                   sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &info) >= 0 &&
                   info.maxDispX > info.minDispX && info.maxDispY > info.minDispY;
    if (touch_ready_) {
        min_x_ = info.minDispX; min_y_ = info.minDispY;
        span_x_ = info.maxDispX - info.minDispX; span_y_ = info.maxDispY - info.minDispY;
        // Prime held touches at screen transitions; only new touches select.
        poll();
    } else runtimeLog("Front touch unavailable; physical menu controls remain active");
}

InputFrame VitaInput::poll() {
    InputFrame frame;
    SceCtrlData pad{};
    if (sceCtrlPeekBufferPositive(0, &pad, 1) > 0) {
        const uint32_t pressed = pad.buttons & ~previous_buttons_;
        const bool up = (pad.buttons & SCE_CTRL_UP) || pad.ly < 80;
        const bool down = (pad.buttons & SCE_CTRL_DOWN) || pad.ly > 176;
        frame.up = up && !previous_up_; frame.down = down && !previous_down_;
        frame.confirm = (pressed & SCE_CTRL_CROSS) != 0;
        frame.back = (pressed & (SCE_CTRL_CIRCLE | SCE_CTRL_TRIANGLE)) != 0;
        frame.pause = (pressed & SCE_CTRL_START) != 0;
        previous_buttons_ = pad.buttons; previous_up_ = up; previous_down_ = down;
    }
    if (!touch_ready_) return frame;
    SceTouchData touch{};
    if (sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1) <= 0) return frame;
    const size_t count = std::min<size_t>(touch.reportNum, previous_pointers_.size());
    std::array<PointerEvent, 8> current{};
    for (size_t i = 0; i < count; ++i) {
        PointerEvent& event = current[i];
        event.id = touch.report[i].id;
        event.x = std::max(0.0f, std::min(959.0f, (touch.report[i].x - min_x_) * 959.0f / span_x_));
        event.y = std::max(0.0f, std::min(543.0f, (touch.report[i].y - min_y_) * 543.0f / span_y_));
        event.phase = PointerPhase::Begin;
        for (size_t p = 0; p < previous_count_; ++p)
            if (previous_pointers_[p].id == event.id) event.phase = PointerPhase::Move;
        frame.pointers[frame.pointer_count++] = event;
    }
    for (size_t p = 0; p < previous_count_; ++p) {
        bool found = false;
        for (size_t i = 0; i < count; ++i) if (current[i].id == previous_pointers_[p].id) found = true;
        if (!found) {
            PointerEvent event = previous_pointers_[p];
            event.phase = PointerPhase::End;
            frame.pointers[frame.pointer_count++] = event;
        }
    }
    previous_pointers_ = current; previous_count_ = count;
    return frame;
}
