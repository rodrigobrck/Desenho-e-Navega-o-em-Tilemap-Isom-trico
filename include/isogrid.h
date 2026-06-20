// isogrid.h
// Contrato do IsoGrid: a matematica que converte uma posicao de grade
// (linha, coluna) na posicao correspondente na tela (x, y) do layout staggered.
#pragma once

struct IsoGrid {
    int rows = 0;
    int cols = 0;
    int tileW = 0;
    int tileH = 0;
    int screenW = 0;
    int screenH = 0;

    float offsetX() const;
    float offsetY() const;
    float mapWidth() const;
    float mapHeight() const;
    void tileScreenPos(int row, int col, float& x, float& y) const;

private:
    void bounds(float& minX, float& minY, float& maxX, float& maxY) const;
};
