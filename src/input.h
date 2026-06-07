#pragma once

#include <unordered_map>
#include "tilemap.h"

enum class InputAction {
    None,
    Quit,
    CycleTile,
    ToggleDebug,
    ToggleMode
};

class InputManager {
public:
    InputManager();

    // Called from key callback to update state
    void onKey(int key, int scancode, int action, int mods);

    // Handle non-movement keys (ESC, TAB, F1) and return an action
    InputAction handleNonMovementKey(int key, int action) const;

    // Determine movement direction based on current key state and deadzone
    // Returns true if a movement direction was chosen
    bool pickMovement(Direction& out, double deadzone);

private:
    // Pending move state: when first movement key is pressed we wait up to the deadzone
    // to allow a second key for diagonal input. After committing a movement we ignore
    // further moves until all movement keys are released.
    bool pendingMove = false;
    double pendingStartTime = 0.0;
    bool movedForCurrentPress = false;

private:
    std::unordered_map<int, bool> keysPressed;
    std::unordered_map<int, double> keyPressedTime;
    std::unordered_map<int, Direction> moves;
};
