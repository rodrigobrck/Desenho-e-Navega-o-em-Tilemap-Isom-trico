// input.h
// Contrato do InputManager: define o enum InputAction (sair, ciclar tile, trocar
// de modo, iniciar Pega-Pega) e as funcoes que interpretam o teclado, inclusive
// a combinacao de duas teclas numa direcao diagonal.
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
