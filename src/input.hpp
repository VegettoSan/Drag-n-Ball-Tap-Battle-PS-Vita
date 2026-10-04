#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

// Platform-neutral events. The future original KeyData/Controller port consumes
// stable pointer IDs and phases; the bootstrap consumes only menu commands.
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
    std::array<PointerEvent, 8> previous_pointers_{};
    size_t previous_count_ = 0;
};
