// isogrid.cpp
// Coracao do visual isometrico: calcula a posicao de cada tile na tela. As
// linhas impares deslocam meio tile na horizontal e cada linha desce so metade
// da altura, o que cria a profundidade staggered. Tambem centraliza o mapa na
// janela calculando os limites (bounds) da grade.
#include "isogrid.h"

#include <limits>

namespace {
// Folga vertical extra para o mapa nao colar no topo da janela.
constexpr float VERTICAL_PADDING = 20.0f;
}

// Calcula a caixa que envolve todos os tiles (sem offset de centralizacao),
// varrendo a grade e guardando os extremos das posicoes. As linhas impares somam
// meio tile em X (deslocamento staggered) e cada linha desce so meia altura.
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

// Deslocamento horizontal que centraliza o mapa na largura da janela.
float IsoGrid::offsetX() const {
    float minX, minY, maxX, maxY;
    bounds(minX, minY, maxX, maxY);
    const float mapW = (maxX - minX) + static_cast<float>(tileW);
    return (screenW - mapW) * 0.5f - minX;
}

// Deslocamento vertical que centraliza o mapa na altura, com a folga do topo.
float IsoGrid::offsetY() const {
    float minX, minY, maxX, maxY;
    bounds(minX, minY, maxX, maxY);
    const float mapH = (maxY - minY) + static_cast<float>(tileH);
    return (screenH - mapH) * 0.5f - minY + VERTICAL_PADDING;
}

// Largura total ocupada pelo mapa (extensao em X mais um tile).
float IsoGrid::mapWidth() const {
    float minX, minY, maxX, maxY;
    bounds(minX, minY, maxX, maxY);
    return (maxX - minX) + static_cast<float>(tileW);
}

// Altura total ocupada pelo mapa (extensao em Y mais um tile).
float IsoGrid::mapHeight() const {
    float minX, minY, maxX, maxY;
    bounds(minX, minY, maxX, maxY);
    return (maxY - minY) + static_cast<float>(tileH);
}

// Converte uma celula (row, col) na posicao (x, y) do canto superior esquerdo do
// tile na tela, ja com a centralizacao e o deslocamento staggered das linhas impares.
void IsoGrid::tileScreenPos(int row, int col, float& x, float& y) const {
    const float halfW = tileW * 0.5f;
    const float halfH = tileH * 0.5f;
    x = offsetX() + col * tileW + (row % 2) * halfW;
    y = offsetY() + row * halfH;
}
