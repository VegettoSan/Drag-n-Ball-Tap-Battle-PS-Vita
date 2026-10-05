#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

// Platform-neutral events. The original Dragon Ball Tap Battle engine only
// consumes pointer IDs 0..4 in TCBManajer.Run, mirroring Android's compact
// pointer slots. Vita hardware touch IDs are not part of that contract, so the
// Vita backend maps them to stable logical IDs before publishing events.
enum class PointerPhase { Begin, Move, End };
struct PointerEvent { int id = -1; float x = 0, y = 0; PointerPhase phase = PointerPhase::Move; };
struct InputFrame {
    bool up = false, down = false, confirm = false, back = false, pause = false;
    std::array<PointerEvent, 16> pointers{};
    size_t pointer_count = 0;
};

class VitaInput {
public:
    VitaInput();
    InputFrame poll();
private:
    uint32_t previous_buttons_ = 0;
    bool previous_up_ = false, previous_down_ = false, touch_ready_ = false;
    float min_x_ = 0, min_y_ = 0, span_x_ = 1, span_y_ = 1;
    std::array<PointerEvent, 5> previous_pointers_{};
    std::array<int, 5> previous_raw_ids_{};
    size_t previous_count_ = 0;
    int touch_log_budget_ = 24;
    int touch_peek_error_logs_ = 3;
    bool touch_poll_logged_ = false;
};
