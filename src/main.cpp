#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "renderer.h"

#include <iostream>
#include <string>

namespace {

struct AppState {
    Renderer* renderer = nullptr;
    InputManager input;
    PegaPega pega;
};

void framebufferSizeCallback(GLFWwindow* win, int width, int height) {
    glViewport(0, 0, width, height);
    auto* app = static_cast<AppState*>(glfwGetWindowUserPointer(win));
    if (app != nullptr && app->renderer != nullptr) {
        app->renderer->setViewport(width, height);
    }
}

}

int main(int argc, char** argv) {
    const std::string mapFile = (argc > 1) ? argv[1] : "map.txt";

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_W, SCREEN_H,
                                          "Mapa Isometrico Staggered - OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return 1;
    }

    Renderer renderer;
    if (!renderer.init(mapFile)) {
        std::cerr << "Failed to initialize renderer\n";
        glfwTerminate();
        return 1;
    }

    std::cout << "=== Mapa Isometrico Staggered ===\n"
              << "Movimento (8 direcoes): setas ou WASD (diagonais = duas teclas juntas)\n"
              << "Tiles caminhaveis: definidos pela linha 'walkable' do mapa\n"
              << "Y: alternar Navegacao <-> Puzzle\n"
              << "TAB: alternar (ciclar) o tile da celula atual\n"
              << "P: iniciar o modo Pega-Pega\n"
              << "ESC: sair\n";

    AppState appState;
    appState.renderer = &renderer;
    renderer.setPegaPega(&appState.pega);
    glfwSetWindowUserPointer(window, &appState);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    renderer.setViewport(fbWidth, fbHeight);

    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        auto* app = static_cast<AppState*>(glfwGetWindowUserPointer(win));
        auto* r = app->renderer;

        app->input.onKey(key, scancode, action, mods);

        switch (app->input.handleNonMovementKey(key, action)) {
            case InputAction::Quit:
                glfwSetWindowShouldClose(win, true);
                return;
            case InputAction::CycleTile:
                r->map.cycleTile();
                r->uploadMesh();
                return;
            case InputAction::ToggleMode:
                if (r->map.gameMode() == Tilemap::GameMode::Puzzle) {
                    r->map.setGameMode(Tilemap::GameMode::Navigation);
                    std::cout << "Mode: Navigation\n";
                } else {
                    r->map.setGameMode(Tilemap::GameMode::Puzzle);
                    std::cout << "Mode: Puzzle\n";
                }
                r->uploadMesh();
                return;
            case InputAction::StartPegaPega:
                app->pega.start(r->map);
                std::cout << "Pega-Pega iniciado! Capture o alvo " << PegaPega::GOAL
                          << " vezes antes do tempo acabar.\n";
                r->uploadMesh();
                return;
            case InputAction::None:
                break;
        }
    });

    const double DEADZONE = 0.02;
    const double IDLE_FRAME_DELAY = 0.5;
    double lastTime = glfwGetTime();
    double lastIdleTime = lastTime;
    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        const double dt = now - lastTime;
        lastTime = now;

        if (now - lastIdleTime >= IDLE_FRAME_DELAY) {
            lastIdleTime += IDLE_FRAME_DELAY;
            renderer.map.character.advanceIdleFrame();
            renderer.uploadMesh();
        }

        if (renderer.map.gameMode() == Tilemap::GameMode::PegaPega) {
            appState.pega.update(dt);
            renderer.uploadMesh();
            renderer.hud.setText(appState.pega.hudText());
        } else {
            renderer.hud.clear();
        }

        Direction chosen;
        if (appState.input.pickMovement(chosen, DEADZONE)) {
            if (renderer.map.move(chosen)) {
                if (renderer.map.gameMode() == Tilemap::GameMode::Puzzle) {
                    renderer.map.visitTile(renderer.map.player.row, renderer.map.player.col);
                }
                if (appState.pega.onPlayerMoved(renderer.map)) {
                    const PegaPega& pega = appState.pega;
                    if (pega.status() == PegaPega::Status::Won) {
                        std::cout << "Pega-Pega: voce venceu com " << pega.catches() << " capturas!\n";
                    } else {
                        std::cout << "Pega-Pega: capturas " << pega.catches() << "/" << PegaPega::GOAL
                                  << " | tempo " << pega.timeLeft() << "s\n";
                    }
                }
                renderer.uploadMesh();
            } else if (renderer.map.blockedReason(chosen)) {
                std::cout << "Bloqueado: terreno nao e caminhavel.\n";
            }
        }

        renderer.draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
