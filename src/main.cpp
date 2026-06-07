#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "debug_text.h"
#include "shader.h"
#include "spritesheet.h"
#include "tilemap.h"
#include "input.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace {
constexpr int SCREEN_W = 800;
constexpr int SCREEN_H = 600;

const char* VERTEX_SHADER = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;

uniform mat4 uProjection;

out vec2 vTex;

void main() {
    vTex = aTex;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

const char* FRAGMENT_SHADER = R"(
#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    vec4 color = texture(uTexture, vTex);

    // chroma key: preto e magenta (fundo dos spritesheets)
    if (color.a < 0.1) discard;
    if (color.r < 0.05 && color.g < 0.05 && color.b < 0.05) discard;
    if (color.r > 0.9 && color.g < 0.1 && color.b > 0.9) discard;

    FragColor = color;
}
)";

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void orthoMat4(float left, float right, float bottom, float top, float out[16]) {
    std::fill(out, out + 16, 0.0f);
    out[0]  = 2.0f / (right - left);
    out[5]  = 2.0f / (top - bottom);
    out[10] = -1.0f;
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[15] = 1.0f;
}

struct Renderer {
    GLuint vao = 0;
    GLuint vbo = 0;
    Shader shader;

    SpriteSheet terrain1;
    SpriteSheet terrain2;
    SpriteSheet water;
    SpriteSheet forest;
    SpriteSheet character;

    Tilemap map;
    DebugText debug;
    std::vector<DrawBatch> batches;

    bool init() {
        if (!shader.loadFromSource(VERTEX_SHADER, FRAGMENT_SHADER)) {
            return false;
        }

        // Overworld sheets: 768x768, grade 3x6, cada celula 256x128 (proporcao 2:1)
        constexpr int SHEET_COLS = 3;
        constexpr int SHEET_ROWS = 6;
        constexpr int ISO_W = 256;
        constexpr int ISO_H = 128;

        bool ok = true;
        ok &= terrain1.load("Tiles/Overworld - Terrain 1 - Flat 256x128.png", SHEET_COLS, SHEET_ROWS, ISO_W, ISO_H);
        ok &= terrain2.load("Tiles/Overworld - Terrain 2 - Flat 256x128.png", SHEET_COLS, SHEET_ROWS, ISO_W, ISO_H);
        ok &= water.load("Tiles/Overworld - Water - Flat 256x128.png", SHEET_COLS, SHEET_ROWS, ISO_W, ISO_H);
        ok &= forest.load("Tiles/Overworld - Forest - Flat 256x128.png", SHEET_COLS, SHEET_ROWS, ISO_W, ISO_H);

        // Personagem AxulArt: 8 direcoes x 12 linhas, sprites 16x24
        ok &= character.load("Character/Small-8-Direction-Characters_by_AxulArt.png", 8, 12, 16, 24);

        if (!ok) {
            std::cerr << "Failed to load one or more sprite sheets.\n";
            return false;
        }

        map.setTileSheets(&terrain1, &terrain2, &water, &forest);
        map.setCharacterSheet(&character);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (!debug.init()) {
            return false;
        }

        uploadMesh();
        return true;
    }

    void uploadMesh() {
        map.buildBatches(batches);
        if (debug.isEnabled()) {
            debug.buildTileLabels(map);
        }
    }

    void draw() {
        glClearColor(0.15f, 0.18f, 0.22f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float proj[16];
        orthoMat4(0.0f, static_cast<float>(SCREEN_W),
                  static_cast<float>(SCREEN_H), 0.0f, proj);

        shader.use();
        shader.setMat4("uProjection", proj);
        shader.setInt("uTexture", 0);

        glBindVertexArray(vao);

        for (const auto& batch : batches) {
            if (batch.vertices.empty()) continue;

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptr>(batch.vertices.size() * sizeof(SpriteVertex)),
                         batch.vertices.data(), GL_DYNAMIC_DRAW);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, batch.texture);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(batch.vertices.size()));
        }

        glBindVertexArray(0);

        debug.draw(proj);
    }

    ~Renderer() {
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
    }
};

struct AppState {
    Renderer* renderer = nullptr;
    bool movedThisFrame = false;
    InputManager input;
};

} // namespace

int main() {
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
    if (!renderer.init()) {
        std::cerr << "Failed to initialize renderer\n";
        glfwTerminate();
        return 1;
    }

    std::cout << "=== Mapa Isometrico Staggered (9x11) ===\n"
              << "Movimento (8 direcoes): setas ou WASD + diagonais E/Q/C/Z\n"
              << "Terrenos caminhaveis: terra, grama, pedra, areia, barro\n"
              << "Terrenos bloqueados: agua, floresta\n"
              << "TAB: alternar tile da celula (modo editor)\n"
              << "M: alternar entre modo Editor e Puzzle\n"
              << "F1: modo debug (coordenadas nos tiles)\n"
              << "ESC: sair\n";

    AppState appState;
    appState.renderer = &renderer;
    glfwSetWindowUserPointer(window, &appState);

    // Key callback: forward to InputManager and handle non-movement actions
    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        auto* app = static_cast<AppState*>(glfwGetWindowUserPointer(win));
        auto* r = app->renderer;

        // forward movement key presses/releases
        app->input.onKey(key, scancode, action, mods);

        // handle non-movement keys via InputManager
        InputAction act = app->input.handleNonMovementKey(key, action);
        if (act == InputAction::Quit) {
            glfwSetWindowShouldClose(win, true);
            return;
        }
        if (act == InputAction::CycleTile) {
            r->map.cycleTile();
            r->uploadMesh();
            return;
        }
        if (act == InputAction::ToggleDebug) {
            r->debug.toggle();
            std::cout << "Debug: " << (r->debug.isEnabled() ? "ON" : "OFF") << '\n';
            if (r->debug.isEnabled()) {
                r->debug.buildTileLabels(r->map);
            }
            return;
        }
        if (act == InputAction::ToggleMode) {
            // toggle between Navigation and Editor
            if (r->map.gameMode() == Tilemap::GameMode::Navigation) {
                r->map.setGameMode(Tilemap::GameMode::Editor);
                std::cout << "Mode: Editor\n";
            } else {
                r->map.setGameMode(Tilemap::GameMode::Navigation);
                std::cout << "Mode: Navigation\n";
            }
            r->uploadMesh();
            return;
        }

        // 'M' toggles between Editor and Puzzle modes when currently not in Navigation
        if (key == GLFW_KEY_M && action == GLFW_PRESS) {
            r->map.toggleMode();
            auto gm = r->map.gameMode();
            if (gm == Tilemap::GameMode::Editor) std::cout << "Submode: Editor\n";
            else if (gm == Tilemap::GameMode::Puzzle) std::cout << "Submode: Puzzle\n";
            r->uploadMesh();
            return;
        }

        // In Editor mode, use left/right arrows to change the tile at the selected cell
        if (r->map.gameMode() == Tilemap::GameMode::Editor) {
            if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
                r->map.changeTile(1);
                r->uploadMesh();
                return;
            }
            if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
                r->map.changeTile(-1);
                r->uploadMesh();
                return;
            }
        }
    });

    const double DEADZONE = 0.02; // seconds
    while (!glfwWindowShouldClose(window)) {
        appState.movedThisFrame = false;

        Direction chosen;
        // Disable movement while in Editor mode; only allow tile changes with arrows
        if (renderer.map.gameMode() != Tilemap::GameMode::Editor) {
            if (appState.input.pickMovement(chosen, DEADZONE)) {
                if (!appState.movedThisFrame) {
                    if (renderer.map.move(chosen)) {
                        appState.movedThisFrame = true;
                        if (renderer.map.gameMode() == Tilemap::GameMode::Puzzle) {
                            renderer.map.visitTile(renderer.map.player.row, renderer.map.player.col);
                        }
                        renderer.uploadMesh();
                    } else {
                        GridPos next = renderer.map.neighbor(chosen);
                        if (renderer.map.inBounds(next.row, next.col) &&
                            !Tilemap::isWalkable(renderer.map.at(next.row, next.col))) {
                            std::cout << "Bloqueado: " << Tilemap::tileName(renderer.map.at(next.row, next.col))
                                      << " nao e caminhavel.\n";
                        }
                    }
                }
            }
        }

        renderer.draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
