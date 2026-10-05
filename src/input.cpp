#include "input.hpp"
#include "log.hpp"
#include <algorithm>
#include <string>
#include <psp2/ctrl.h>
#include <psp2/touch.h>

namespace {
constexpr size_t kGameTouchSlots = 5;
}

VitaInput::VitaInput() {
    previous_raw_ids_.fill(-1);
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
                   info.maxAaX > info.minAaX && info.maxAaY > info.minAaY;
    if (touch_ready_) {
        min_x_ = info.minAaX; min_y_ = info.minAaY;
        span_x_ = info.maxAaX - info.minAaX; span_y_ = info.maxAaY - info.minAaY;
        runtimeLog("Front touch active area: " + std::to_string(info.minAaX) + "," +
                   std::to_string(info.minAaY) + " -> " + std::to_string(info.maxAaX) + "," +
                   std::to_string(info.maxAaY) + "; display area: " +
                   std::to_string(info.minDispX) + "," + std::to_string(info.minDispY) + " -> " +
                   std::to_string(info.maxDispX) + "," + std::to_string(info.maxDispY));
        // Prime an already-held finger at screen transitions so it does not
        // become a fresh Android ACTION_DOWN in the next scene.
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
    const int peek_result = sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
    if (peek_result <= 0) {
        if (touch_peek_error_logs_-- > 0)
            runtimeLog("Front touch peek failed/result=" + std::to_string(peek_result));
        return frame;
    }
    if (!touch_poll_logged_) {
        runtimeLog("Front touch polling active");
        touch_poll_logged_ = true;
    }

    const size_t count = std::min<size_t>(touch.reportNum, kGameTouchSlots);
    std::array<PointerEvent, 5> current{};
    std::array<int, 5> current_raw_ids{};
    current_raw_ids.fill(-1);

    // The original game does not iterate arbitrary Android pointer IDs. Its
    // TCBManajer.Run explicitly queries KeyData IDs 0 through 4 every frame.
    // Keep Vita's hardware IDs only for continuity tracking and expose compact,
    // stable logical IDs to the game.
    bool reserved[kGameTouchSlots] = {false, false, false, false, false};
    for (size_t p = 0; p < previous_count_; ++p) {
        const int logical = previous_pointers_[p].id;
        if (logical >= 0 && logical < static_cast<int>(kGameTouchSlots)) reserved[logical] = true;
    }

    for (size_t i = 0; i < count; ++i) {
        const int raw_id = static_cast<int>(touch.report[i].id);
        int logical_id = -1;
        bool existed = false;
        for (size_t p = 0; p < previous_count_; ++p) {
            if (previous_raw_ids_[p] == raw_id) {
                logical_id = previous_pointers_[p].id;
                existed = true;
                break;
            }
        }
        if (logical_id < 0) {
            for (size_t slot = 0; slot < kGameTouchSlots; ++slot) {
                if (!reserved[slot]) {
                    logical_id = static_cast<int>(slot);
                    reserved[slot] = true;
                    break;
                }
            }
        }
        if (logical_id < 0) continue;

        PointerEvent& event = current[i];
        event.id = logical_id;
        event.x = std::max(0.0f, std::min(959.0f, (touch.report[i].x - min_x_) * 959.0f / span_x_));
        event.y = std::max(0.0f, std::min(543.0f, (touch.report[i].y - min_y_) * 543.0f / span_y_));
        event.phase = existed ? PointerPhase::Move : PointerPhase::Begin;
        current_raw_ids[i] = raw_id;
        frame.pointers[frame.pointer_count++] = event;

        if (touch_log_budget_ > 0 && event.phase == PointerPhase::Begin) {
            runtimeLog("Touch begin raw=" + std::to_string(raw_id) +
                       " logical=" + std::to_string(logical_id) +
                       " rawXY=" + std::to_string(touch.report[i].x) + "," +
                       std::to_string(touch.report[i].y) +
                       " screenXY=" + std::to_string(static_cast<int>(event.x)) + "," +
                       std::to_string(static_cast<int>(event.y)));
            --touch_log_budget_;
        }
    }

    for (size_t p = 0; p < previous_count_; ++p) {
        bool found = false;
        for (size_t i = 0; i < count; ++i)
            if (current_raw_ids[i] == previous_raw_ids_[p]) { found = true; break; }
        if (!found) {
            PointerEvent event = previous_pointers_[p];
            event.phase = PointerPhase::End;
            frame.pointers[frame.pointer_count++] = event;
            if (touch_log_budget_ > 0) {
                runtimeLog("Touch end raw=" + std::to_string(previous_raw_ids_[p]) +
                           " logical=" + std::to_string(event.id));
                --touch_log_budget_;
            }
        }
    }

    previous_pointers_ = current;
    previous_raw_ids_ = current_raw_ids;
    previous_count_ = count;
    return frame;
}
