#include "debug_text.h"

#define STB_EASY_FONT_IMPLEMENTATION
#include <stb_easy_font.h>

#include <cstdio>
#include <vector>

namespace {
constexpr float TEXT_SCALE = 0.75f;

struct ColorVertex {
    float x, y;
    float r, g, b, a;
};

const char* VERTEX_SHADER = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;

uniform mat4 uProjection;

out vec4 vColor;

void main() {
    vColor = aColor;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

const char* FRAGMENT_SHADER = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main() {
    FragColor = vColor;
}
)";
}

bool DebugText::init() {
    if (!shader_.loadFromSource(VERTEX_SHADER, FRAGMENT_SHADER)) {
        return false;
    }

    stb_easy_font_spacing(0.5f);

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glGenVertexArrays(1, &hudVao_);
    glGenBuffers(1, &hudVbo_);

    glBindVertexArray(hudVao_);
    glBindBuffer(GL_ARRAY_BUFFER, hudVbo_);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    return true;
}

void DebugText::toggle() {
    enabled_ = !enabled_;
}

static void appendText(float x, float y, const char* text,
                       unsigned char r, unsigned char g, unsigned char b, unsigned char a,
                       float scale, std::vector<ColorVertex>& out) {
    unsigned char color[4] = {r, g, b, a};
    float buffer[4096];
    const int quads = stb_easy_font_print(0.0f, 0.0f, const_cast<char*>(text), color, buffer, sizeof(buffer));

    for (int q = 0; q < quads; ++q) {
        const float* quad = buffer + q * 16;
        const float tri[6][2] = {
            {quad[0], quad[1]}, {quad[4], quad[5]}, {quad[8], quad[9]},
            {quad[0], quad[1]}, {quad[8], quad[9]}, {quad[12], quad[13]},
        };

        for (const auto& v : tri) {
            ColorVertex cv;
            cv.x = x + v[0] * scale;
            cv.y = y + v[1] * scale;
            cv.r = r / 255.0f;
            cv.g = g / 255.0f;
            cv.b = b / 255.0f;
            cv.a = a / 255.0f;
            out.push_back(cv);
        }
    }
}

void DebugText::buildTileLabels(const Tilemap& map) {
    constexpr int EST_VERTS_PER_TILE = 200;
    std::vector<ColorVertex> vertices;
    vertices.reserve(static_cast<size_t>(map.rows * map.cols * EST_VERTS_PER_TILE));

    char label[64];

    for (int i = 0; i < map.rows; ++i) {
        for (int j = 0; j < map.cols; ++j) {
            float tx, ty;
            map.getTileScreenPos(i, j, tx, ty);

            const int tileId = map.at(i, j);
            std::snprintf(label, sizeof(label), "%d,%d [%d]", i, j, tileId);

            const bool isPlayer = (i == map.player.row && j == map.player.col);
            const bool walkable = map.tileset.isWalkable(tileId);

            unsigned char cr = 255;
            unsigned char cg = isPlayer ? 255 : (walkable ? 255 : 120);
            unsigned char cb = isPlayer ? 80  : (walkable ? 255 : 120);

            const float textW = static_cast<float>(stb_easy_font_width(label)) * TEXT_SCALE;
            const float textX = tx + (Tilemap::TILE_W - textW) * 0.5f;
            const float textY = ty + Tilemap::TILE_H * 0.5f - 4.0f;

            appendText(textX, textY, label, cr, cg, cb, 230, TEXT_SCALE, vertices);
        }
    }

    vertexCount_ = static_cast<int>(vertices.size());

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(ColorVertex)),
                 vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugText::setHud(const std::string& text) {
    std::vector<ColorVertex> vertices;
    appendText(10.0f, 10.0f, text.c_str(), 255, 235, 90, 255, 1.6f, vertices);

    hudVertexCount_ = static_cast<int>(vertices.size());

    glBindBuffer(GL_ARRAY_BUFFER, hudVbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(ColorVertex)),
                 vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugText::clearHud() {
    hudVertexCount_ = 0;
}

void DebugText::draw(const float worldProjection[16], const float screenProjection[16]) const {
    const bool drawLabels = enabled_ && vertexCount_ > 0;
    const bool drawHud = hudVertexCount_ > 0;
    if (!drawLabels && !drawHud) {
        return;
    }

    shader_.use();

    if (drawLabels) {
        shader_.setMat4("uProjection", worldProjection);
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    }
    if (drawHud) {
        shader_.setMat4("uProjection", screenProjection);
        glBindVertexArray(hudVao_);
        glDrawArrays(GL_TRIANGLES, 0, hudVertexCount_);
    }
    glBindVertexArray(0);
}
