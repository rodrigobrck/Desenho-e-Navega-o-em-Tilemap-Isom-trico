#pragma once

#include <unordered_map>
#include "types.h"

enum class InputAction {
    None,
    Quit,
    CycleTile,
    ToggleMode,
    StartPegaPega
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
