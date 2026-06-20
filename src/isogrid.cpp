// isogrid.cpp
// Coracao do visual isometrico: calcula a posicao de cada tile na tela. As
// linhas impares deslocam meio tile na horizontal e cada linha desce so metade
// da altura, o que cria a profundidade staggered. Tambem centraliza o mapa na
// janela calculando os limites (bounds) da grade.
#include "isogrid.h"

#include <limits>

namespace {
constexpr float VERTICAL_PADDING = 20.0f;
}

void IsoGrid::bounds(float& minX, float& minY, float& maxX, float& maxY) const {
    const float halfW = tileW * 0.5f;
    const float halfH = tileH * 0.5f;

    minX = minY = std::numeric_limits<float>::infinity();
    maxX = maxY = -std::numeric_limits<float>::infinity();

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const float x = c * tileW + (r % 2) * halfW;
            const float y = r * halfH;
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
    }
}

float IsoGrid::offsetX() const {
    float minX, minY, maxX, maxY;
    bounds(minX, minY, maxX, maxY);
    const float mapW = (maxX - minX) + static_cast<float>(tileW);
    return (screenW - mapW) * 0.5f - minX;
}

float IsoGrid::offsetY() const {
    float minX, minY, maxX, maxY;
    bounds(minX, minY, maxX, maxY);
    const float mapH = (maxY - minY) + static_cast<float>(tileH);
    return (screenH - mapH) * 0.5f - minY + VERTICAL_PADDING;
}

float IsoGrid::mapWidth() const {
    float minX, minY, maxX, maxY;
    bounds(minX, minY, maxX, maxY);
    return (maxX - minX) + static_cast<float>(tileW);
}

float IsoGrid::mapHeight() const {
    float minX, minY, maxX, maxY;
    bounds(minX, minY, maxX, maxY);
    return (maxY - minY) + static_cast<float>(tileH);
}

void IsoGrid::tileScreenPos(int row, int col, float& x, float& y) const {
    const float halfW = tileW * 0.5f;
    const float halfH = tileH * 0.5f;
    x = offsetX() + col * tileW + (row % 2) * halfW;
    y = offsetY() + row * halfH;
}
