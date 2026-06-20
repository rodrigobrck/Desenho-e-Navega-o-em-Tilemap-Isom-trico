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
};
