#include "renderer.h"

#include "gfx_math.h"

#include <algorithm>
#include <iostream>
#include <utility>

namespace {
constexpr int CHAR_SHEET_COLS = 8;
constexpr int CHAR_SHEET_ROWS = 12;
constexpr int CHAR_CELL_W = 16;
constexpr int CHAR_CELL_H = 24;

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

    if (color.a < 0.1) discard;
    if (color.r < 0.05 && color.g < 0.05 && color.b < 0.05) discard;
    if (color.r > 0.9 && color.g < 0.1 && color.b > 0.9) discard;

    FragColor = color;
}
)";
}

bool Renderer::init(const std::string& mapFile) {
    if (!shader_.loadFromSource(VERTEX_SHADER, FRAGMENT_SHADER)) {
        return false;
    }

    if (!character.load("Character/Small-8-Direction-Characters_by_AxulArt.png",
                        CHAR_SHEET_COLS, CHAR_SHEET_ROWS, CHAR_CELL_W, CHAR_CELL_H)) {
        std::cerr << "Failed to load character sprite sheet.\n";
        return false;
    }
    map.character.setSheet(&character);

    if (!map.loadFromFile(resolveAssetPath(std::string("Maps/") + mapFile))) {
        std::cerr << "Failed to load map '" << mapFile << "'.\n";
        return false;
    }

    if (!tileSheet.loadAuto("Tiles/" + map.tilesetName, map.tileSrcW, map.tileSrcH)) {
        std::cerr << "Failed to load tileset '" << map.tilesetName << "' declared in the map.\n";
        return false;
    }

    const int capacity = tileSheet.cols * tileSheet.rows;
    if (map.tileCount > capacity) {
        std::cerr << "Tileset '" << map.tilesetName << "' holds " << capacity
                  << " tiles (" << tileSheet.cols << "x" << tileSheet.rows
                  << ") but the map declares " << map.tileCount << ".\n";
        return false;
    }

    map.setTileSheet(&tileSheet);

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!hud.init()) {
        return false;
    }

    uploadMesh();
    return true;
}

void Renderer::setPegaPega(const PegaPega* pega) {
    pega_ = pega;
}

void Renderer::setViewport(int width, int height) {
    viewportW_ = std::max(width, 1);
    viewportH_ = std::max(height, 1);
}

void Renderer::buildBatches() {
    batches_.clear();

    std::vector<std::pair<int, int>> order;
    for (int r = 0; r < map.rows; ++r) {
        for (int c = 0; c < map.cols; ++c) {
            order.emplace_back(r, c);
        }
    }
    std::sort(order.begin(), order.end(), [](const auto& a, const auto& b) {
        const int sa = a.first + a.second;
        const int sb = b.first + b.second;
        if (sa != sb) return sa < sb;
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    });

    const float tileScaleX = static_cast<float>(Tilemap::TILE_W) / static_cast<float>(map.tileSrcW);
    const float tileScaleY = static_cast<float>(Tilemap::TILE_H) / static_cast<float>(map.tileSrcH);

    const bool pegaActive = pega_ != nullptr &&
                            map.gameMode() == Tilemap::GameMode::PegaPega &&
                            pega_->isRunning();

    for (const auto& [r, c] : order) {
        const TileSpriteRef ref = map.tileset.spriteRef(map.at(r, c));
        float x, y;
        map.getTileScreenPos(r, c, x, y);
        addSpriteToBatches(batches_, ref.sheet, x, y, ref.col, ref.row, tileScaleX, tileScaleY);

        if (pegaActive && r == pega_->position().row && c == pega_->position().col) {
            pega_->npc.appendSprite(batches_, x, y, Tilemap::TILE_W, Tilemap::TILE_H);
        }

        if (r == map.player.row && c == map.player.col) {
            map.character.appendSprite(batches_, x, y, Tilemap::TILE_W, Tilemap::TILE_H);
        }
    }
}

void Renderer::uploadMesh() {
    buildBatches();
}

void Renderer::draw() {
    glClearColor(0.15f, 0.18f, 0.22f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const IsoGrid grid = map.grid();
    const float mapW = grid.mapWidth();
    const float mapH = grid.mapHeight();
    const float margin = 1.08f;
    const float fit = std::max(mapW / SCREEN_W, mapH / SCREEN_H) * margin;
    const float zoom = std::max(fit, 1.0f);

    const float contentW = SCREEN_W * zoom;
    const float contentH = SCREEN_H * zoom;
    const float contentAspect = contentW / contentH;
    const float windowAspect = static_cast<float>(viewportW_) / static_cast<float>(viewportH_);

    float regionW = contentW;
    float regionH = contentH;
    if (windowAspect > contentAspect) {
        regionW = contentH * windowAspect;
    } else {
        regionH = contentW / windowAspect;
    }

    const float cx = SCREEN_W * 0.5f;
    const float cy = SCREEN_H * 0.5f;

    float worldProj[16];
    orthoMat4(cx - regionW * 0.5f, cx + regionW * 0.5f,
              cy + regionH * 0.5f, cy - regionH * 0.5f, worldProj);

    float screenProj[16];
    orthoMat4(0.0f, static_cast<float>(viewportW_),
              static_cast<float>(viewportH_), 0.0f, screenProj);

    shader_.use();
    shader_.setMat4("uProjection", worldProj);
    shader_.setInt("uTexture", 0);

    glBindVertexArray(vao_);

    for (const auto& batch : batches_) {
        if (batch.vertices.empty()) continue;

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(batch.vertices.size() * sizeof(SpriteVertex)),
                     batch.vertices.data(), GL_DYNAMIC_DRAW);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, batch.texture);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(batch.vertices.size()));
    }

    glBindVertexArray(0);

    hud.draw(screenProj);
}

Renderer::~Renderer() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}
