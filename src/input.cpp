#include "input.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>

InputManager::InputManager() {
    moves = {
        {GLFW_KEY_UP, Direction::N},    {GLFW_KEY_W, Direction::N},
        {GLFW_KEY_DOWN, Direction::S},  {GLFW_KEY_S, Direction::S},
        {GLFW_KEY_LEFT, Direction::W},  {GLFW_KEY_A, Direction::W},
        {GLFW_KEY_RIGHT, Direction::E}, {GLFW_KEY_D, Direction::E},
    };
}

void InputManager::onKey(int key, int scancode, int action, int mods) {
    auto it = moves.find(key);
    if (it != moves.end()) {
        int beforeCount = 0;
        for (const auto& kv : moves) {
            auto it2 = keysPressed.find(kv.first);
            if (it2 != keysPressed.end() && it2->second) ++beforeCount;
        }

        if (action == GLFW_PRESS) {
            keysPressed[key] = true;
            keyPressedTime[key] = glfwGetTime();
        } else if (action == GLFW_RELEASE) {
            keysPressed[key] = false;
            keyPressedTime.erase(key);
        }

        int afterCount = 0;
        for (const auto& kv : moves) {
            auto it2 = keysPressed.find(kv.first);
            if (it2 != keysPressed.end() && it2->second) ++afterCount;
        }

        if (beforeCount == 0 && afterCount > 0) {
            pendingMove = true;
            pendingStartTime = glfwGetTime();
            movedForCurrentPress = false;
        }

        if (afterCount == 0) {
            pendingMove = false;
            movedForCurrentPress = false;
        }
    }
}

InputAction InputManager::handleNonMovementKey(int key, int action) const {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) return InputAction::Quit;
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) return InputAction::CycleTile;
    if (key == GLFW_KEY_Y && action == GLFW_PRESS) return InputAction::ToggleMode;
    if (key == GLFW_KEY_P && action == GLFW_PRESS) return InputAction::StartPegaPega;
    return InputAction::None;
}

static Direction combineDiag(Direction v, Direction h) {
    if ((v == Direction::N) && (h == Direction::E)) return Direction::NE;
    if ((v == Direction::N) && (h == Direction::W)) return Direction::NW;
    if ((v == Direction::S) && (h == Direction::E)) return Direction::SE;
    if ((v == Direction::S) && (h == Direction::W)) return Direction::SW;
    return v;
}

bool InputManager::pickMovement(Direction& out, double deadzone) {
    std::vector<std::pair<int, Direction>> pressedMoves;
    for (const auto& kv : moves) {
        int k = kv.first;
        auto it = keysPressed.find(k);
        if (it != keysPressed.end() && it->second) {
            pressedMoves.emplace_back(k, kv.second);
        }
    }
    if (pressedMoves.empty()) return false;

    int vertKey = 0, horKey = 0;
    Direction vertDir = Direction::N, horDir = Direction::E;
    double vertTime = -1.0, horTime = -1.0;
    double mostRecentTime = -1.0;
    Direction mostRecentDir = Direction::N;

    for (auto& p : pressedMoves) {
        int k = p.first;
        Direction d = p.second;
        double t = 0.0;
        auto it = keyPressedTime.find(k);
        if (it != keyPressedTime.end()) t = it->second;
        if (t > mostRecentTime) {
            mostRecentTime = t;
            mostRecentDir = d;
        }

        if (d == Direction::N || d == Direction::S) {
            if (t > vertTime) { vertTime = t; vertKey = k; vertDir = d; }
        } else if (d == Direction::E || d == Direction::W) {
            if (t > horTime) { horTime = t; horKey = k; horDir = d; }
        }
    }

    if (movedForCurrentPress) return false;

    double now = glfwGetTime();
    bool shouldCommit = false;

    if (pendingMove) {
        if ((now - pendingStartTime) >= deadzone) {
            shouldCommit = true;
        }
    } else {
        shouldCommit = true;
    }

    if (!shouldCommit) return false;

    Direction chosen = mostRecentDir;
    if (vertKey != 0 && horKey != 0) {
        if (std::abs(vertTime - horTime) <= deadzone) {
            chosen = combineDiag(vertDir, horDir);
        } else {
            chosen = mostRecentDir;
        }
    }

    out = chosen;
    movedForCurrentPress = true;
    pendingMove = false;
    return true;
}
