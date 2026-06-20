// isogrid.h
// Contrato do IsoGrid: a matematica que converte uma posicao de grade
// (linha, coluna) na posicao correspondente na tela (x, y) do layout staggered.
#pragma once

struct IsoGrid {
    // Dimensoes da grade, tamanho de cada tile e tamanho da tela.
    int rows = 0;
    int cols = 0;
    int tileW = 0;
    int tileH = 0;
    int screenW = 0;
    int screenH = 0;

    // Deslocamentos que centralizam o mapa na tela.
    float offsetX() const;
    float offsetY() const;
    // Largura e altura totais ocupadas pelo mapa.
    float mapWidth() const;
    float mapHeight() const;
    // Posicao (x, y) na tela do tile em (row, col).
    void tileScreenPos(int row, int col, float& x, float& y) const;

private:
    // Caixa que envolve todos os tiles (sem a centralizacao).
    void bounds(float& minX, float& minY, float& maxX, float& maxY) const;
};
