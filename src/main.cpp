// main.cpp
// Ponto de entrada do programa. Inicializa GLFW e GLAD, cria a janela e o
// contexto OpenGL e roda o LOOP PRINCIPAL: a cada quadro le o teclado, atualiza
// a logica (movimento, modos, Pega-Pega) e manda o Renderer desenhar a cena.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "renderer.h"

#include <iostream>
#include <string>

namespace {

// Estado global da aplicacao guardado no "user pointer" da janela GLFW, para que
// os callbacks de teclado e de redimensionamento possam alcancar o renderer, o
// leitor de teclado e o mini-jogo Pega-Pega.
struct AppState {
    Renderer* renderer = nullptr;
    InputManager input;
    PegaPega pega;
};

// Callback chamado pela GLFW quando a janela muda de tamanho: ajusta a area de
// desenho do OpenGL e avisa o renderer para reenquadrar a cena.
void framebufferSizeCallback(GLFWwindow* win, int width, int height) {
    glViewport(0, 0, width, height);
    auto* app = static_cast<AppState*>(glfwGetWindowUserPointer(win));
    if (app != nullptr && app->renderer != nullptr) {
        app->renderer->setViewport(width, height);
    }
}

}

int main(int argc, char** argv) {
    // Aceita o nome do mapa como argumento de linha de comando; senao usa o padrao.
    const std::string mapFile = (argc > 1) ? argv[1] : "map.txt";

    // Inicializa a GLFW (biblioteca de janela e contexto OpenGL).
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return 1;
    }

    // Pede um contexto OpenGL 3.3 Core (versao usada pelos nossos shaders).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Cria a janela e o contexto OpenGL.
    GLFWwindow* window = glfwCreateWindow(SCREEN_W, SCREEN_H,
                                          "Mapa Isometrico Staggered - OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return 1;
    }

    // Ativa o contexto, liga o V-Sync (1 = sincroniza com o monitor) e registra o
    // callback de redimensionamento.
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Carrega os ponteiros das funcoes do OpenGL via GLAD (precisa do contexto ativo).
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return 1;
    }

    // Cria o renderer e carrega o mapa, os tiles e o personagem.
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

    // Monta o estado da aplicacao e o pendura na janela para os callbacks acessarem.
    AppState appState;
    appState.renderer = &renderer;
    renderer.setPegaPega(&appState.pega);
    glfwSetWindowUserPointer(window, &appState);

    // Define o viewport inicial conforme o tamanho real do framebuffer.
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    renderer.setViewport(fbWidth, fbHeight);

    // Callback de teclado: trata as teclas de acao (sair, ciclar tile, trocar modo,
    // iniciar Pega-Pega). O movimento em si e lido no loop principal, nao aqui.
    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        auto* app = static_cast<AppState*>(glfwGetWindowUserPointer(win));
        auto* r = app->renderer;

        // Atualiza o registro de teclas de movimento seguradas.
        app->input.onKey(key, scancode, action, mods);

        // Traduz a tecla numa acao especial e a executa.
        switch (app->input.handleNonMovementKey(key, action)) {
            case InputAction::Quit:
                glfwSetWindowShouldClose(win, true);
                return;
            case InputAction::CycleTile:
                // Troca o tile sob o jogador pelo proximo do tileset e remonta a malha.
                r->map.cycleTile();
                r->uploadMesh();
                return;
            case InputAction::ToggleMode:
                // Alterna entre os modos Navegacao e Puzzle (Y).
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
                // Inicia o mini-jogo Pega-Pega (P).
                app->pega.start(r->map);
                std::cout << "Pega-Pega iniciado! Capture o alvo " << PegaPega::GOAL
                          << " vezes antes do tempo acabar.\n";
                r->uploadMesh();
                return;
            case InputAction::None:
                break;
        }
    });

    // DEADZONE: janela de tempo para combinar duas teclas numa diagonal.
    // IDLE_FRAME_DELAY: intervalo (em segundos) entre os quadros da animacao parada.
    const double DEADZONE = 0.02;
    const double IDLE_FRAME_DELAY = 0.5;
    double lastTime = glfwGetTime();
    double lastIdleTime = lastTime;
    // Loop principal: roda uma vez por quadro ate a janela ser fechada.
    while (!glfwWindowShouldClose(window)) {
        // Calcula o tempo decorrido (dt) desde o quadro anterior.
        const double now = glfwGetTime();
        const double dt = now - lastTime;
        lastTime = now;

        // Avanca a animacao "parado" do personagem em intervalos fixos.
        if (now - lastIdleTime >= IDLE_FRAME_DELAY) {
            lastIdleTime += IDLE_FRAME_DELAY;
            renderer.map.character.advanceIdleFrame();
            renderer.uploadMesh();
        }

        // No modo Pega-Pega, atualiza o cronometro e o texto do HUD; fora dele,
        // limpa o HUD.
        if (renderer.map.gameMode() == Tilemap::GameMode::PegaPega) {
            appState.pega.update(dt);
            renderer.uploadMesh();
            renderer.hud.setText(appState.pega.hudText());
        } else {
            renderer.hud.clear();
        }

        // Le o teclado e decide se ha um movimento a aplicar neste quadro.
        Direction chosen;
        if (appState.input.pickMovement(chosen, DEADZONE)) {
            if (renderer.map.move(chosen)) {
                // No modo Puzzle, pisar num tile o "evolui" para o proximo terreno.
                if (renderer.map.gameMode() == Tilemap::GameMode::Puzzle) {
                    renderer.map.visitTile(renderer.map.player.row, renderer.map.player.col);
                }
                // Avisa o Pega-Pega do movimento; se houve captura, imprime o placar.
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
                // O movimento falhou porque o destino e um terreno nao caminhavel.
                std::cout << "Bloqueado: terreno nao e caminhavel.\n";
            }
        }

        // Desenha o quadro, troca os buffers (mostra na tela) e processa eventos.
        renderer.draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Encerra a GLFW e libera os recursos da janela.
    glfwTerminate();
    return 0;
}
