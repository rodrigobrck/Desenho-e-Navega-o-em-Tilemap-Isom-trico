// input.h
// Contrato do InputManager: define o enum InputAction (sair, ciclar tile, trocar
// de modo, iniciar Pega-Pega) e as funcoes que interpretam o teclado, inclusive
// a combinacao de duas teclas numa direcao diagonal.
#pragma once

#include <unordered_map>
#include "types.h"

// Acoes que uma tecla pode disparar (alem do movimento em si).
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

    // Registra mudancas de estado das teclas de movimento.
    void onKey(int key, int scancode, int action, int mods);

    // Traduz uma tecla de acao (ESC, TAB, Y, P) na InputAction correspondente.
    InputAction handleNonMovementKey(int key, int action) const;

    // Decide a direcao a mover neste quadro a partir das teclas seguradas.
    bool pickMovement(Direction& out, double deadzone);

private:
    // Controle da janela de tempo (deadzone) para combinar diagonais e garantir
    // um unico passo por toque.
    bool pendingMove = false;
    double pendingStartTime = 0.0;
    bool movedForCurrentPress = false;

private:
    // Estado por tecla: se esta pressionada, quando foi pressionada e a qual
    // direcao de movimento cada tecla corresponde.
    std::unordered_map<int, bool> keysPressed;
    std::unordered_map<int, double> keyPressedTime;
    std::unordered_map<int, Direction> moves;
};
