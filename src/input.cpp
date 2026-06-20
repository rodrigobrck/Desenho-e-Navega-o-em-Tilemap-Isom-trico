// input.cpp
// Interpreta o teclado: registra quais teclas estao seguradas e quando, combina
// duas teclas numa direcao diagonal (com uma pequena deadzone de tempo) e trata
// as acoes especiais (P inicia o Pega-Pega, TAB cicla o tile, Y troca o modo,
// ESC sai).
#include "input.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>

// Monta o mapa de teclas -> direcao, aceitando tanto as setas quanto WASD.
InputManager::InputManager() {
    moves = {
        {GLFW_KEY_UP, Direction::N},    {GLFW_KEY_W, Direction::N},
        {GLFW_KEY_DOWN, Direction::S},  {GLFW_KEY_S, Direction::S},
        {GLFW_KEY_LEFT, Direction::W},  {GLFW_KEY_A, Direction::W},
        {GLFW_KEY_RIGHT, Direction::E}, {GLFW_KEY_D, Direction::E},
    };
}

// Registra o estado das teclas de movimento (pressionada/solta) e o instante em
// que cada uma foi pressionada. Detecta tambem a transicao de "nenhuma tecla" para
// "alguma tecla", que inicia uma nova janela de deadzone para combinar diagonais.
void InputManager::onKey(int key, int scancode, int action, int mods) {
    auto it = moves.find(key);
    if (it != moves.end()) {
        // Conta quantas teclas de movimento estavam seguradas antes desta mudanca.
        int beforeCount = 0;
        for (const auto& kv : moves) {
            auto it2 = keysPressed.find(kv.first);
            if (it2 != keysPressed.end() && it2->second) ++beforeCount;
        }

        // Atualiza o estado e o instante da tecla que acabou de mudar.
        if (action == GLFW_PRESS) {
            keysPressed[key] = true;
            keyPressedTime[key] = glfwGetTime();
        } else if (action == GLFW_RELEASE) {
            keysPressed[key] = false;
            keyPressedTime.erase(key);
        }

        // Conta novamente apos a mudanca.
        int afterCount = 0;
        for (const auto& kv : moves) {
            auto it2 = keysPressed.find(kv.first);
            if (it2 != keysPressed.end() && it2->second) ++afterCount;
        }

        // Comecou um novo toque (de zero para uma ou mais teclas): abre a janela de
        // espera para ver se uma segunda tecla chega e forma uma diagonal.
        if (beforeCount == 0 && afterCount > 0) {
            pendingMove = true;
            pendingStartTime = glfwGetTime();
            movedForCurrentPress = false;
        }

        // Soltou tudo: zera o estado para o proximo toque poder mover de novo.
        if (afterCount == 0) {
            pendingMove = false;
            movedForCurrentPress = false;
        }
    }
}

// Traduz as teclas de acao (que nao sao movimento) numa InputAction. So reage no
// momento do pressionar, para a acao nao se repetir enquanto a tecla fica segurada.
InputAction InputManager::handleNonMovementKey(int key, int action) const {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) return InputAction::Quit;
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) return InputAction::CycleTile;
    if (key == GLFW_KEY_Y && action == GLFW_PRESS) return InputAction::ToggleMode;
    if (key == GLFW_KEY_P && action == GLFW_PRESS) return InputAction::StartPegaPega;
    return InputAction::None;
}

// Combina uma direcao vertical (N/S) com uma horizontal (E/W) na diagonal
// correspondente. Se nao formarem uma diagonal valida, devolve a vertical.
static Direction combineDiag(Direction v, Direction h) {
    if ((v == Direction::N) && (h == Direction::E)) return Direction::NE;
    if ((v == Direction::N) && (h == Direction::W)) return Direction::NW;
    if ((v == Direction::S) && (h == Direction::E)) return Direction::SE;
    if ((v == Direction::S) && (h == Direction::W)) return Direction::SW;
    return v;
}

// Decide qual direcao mover neste quadro a partir das teclas seguradas. Espera a
// deadzone para permitir diagonais, garante um unico passo por toque e devolve
// 'false' quando nao ha movimento a aplicar.
bool InputManager::pickMovement(Direction& out, double deadzone) {
    // Reune todas as teclas de movimento atualmente seguradas.
    std::vector<std::pair<int, Direction>> pressedMoves;
    for (const auto& kv : moves) {
        int k = kv.first;
        auto it = keysPressed.find(k);
        if (it != keysPressed.end() && it->second) {
            pressedMoves.emplace_back(k, kv.second);
        }
    }
    if (pressedMoves.empty()) return false;

    // Acha a tecla vertical e a horizontal seguradas mais recentes, e a tecla mais
    // recente de todas (usada quando nao da para formar uma diagonal).
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

    // Um toque so gera um passo: se ja andou neste toque, nao anda de novo.
    if (movedForCurrentPress) return false;

    // Espera a deadzone terminar antes de confirmar (da tempo da 2a tecla chegar).
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

    // Se ha uma vertical e uma horizontal pressionadas quase juntas, forma diagonal;
    // caso contrario, segue a tecla mais recente.
    Direction chosen = mostRecentDir;
    if (vertKey != 0 && horKey != 0) {
        if (std::abs(vertTime - horTime) <= deadzone) {
            chosen = combineDiag(vertDir, horDir);
        } else {
            chosen = mostRecentDir;
        }
    }

    // Confirma o passo e marca o toque como ja usado.
    out = chosen;
    movedForCurrentPress = true;
    pendingMove = false;
    return true;
}
