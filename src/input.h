#pragma once

#include <unordered_map>
#include "direction.h"

enum class InputAction {
    None,
    Quit,
    CycleTile,
    ToggleDebug,
    ToggleMode,
    ToggleSubmode,
    StartPegaPega,
    EditorNext,
    EditorPrev
};

class InputManager {
public:
    InputManager();

    void onKey(int key, int scancode, int action, int mods);

    InputAction handleNonMovementKey(int key, int action) const;

    bool pickMovement(Direction& out, double deadzone);

private:
    bool pendingMove = false;
    double pendingStartTime = 0.0;
    bool movedForCurrentPress = false;

private:
    std::unordered_map<int, bool> keysPressed;
    std::unordered_map<int, double> keyPressedTime;
    std::unordered_map<int, Direction> moves;
};
