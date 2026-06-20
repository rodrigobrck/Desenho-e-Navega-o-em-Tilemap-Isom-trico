#include "isogrid.h"

#include <limits>

float IsoGrid::offsetX() const {
    const float halfW = tileW * 0.5f;
    float minX = std::numeric_limits<float>::infinity();
    float maxX = -std::numeric_limits<float>::infinity();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const float x = c * tileW + (r % 2) * halfW;
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
        }
    }
    const float mapW = (maxX - minX) + static_cast<float>(tileW);
    return (screenW - mapW) * 0.5f - minX;
}

float IsoGrid::offsetY() const {
    const float halfH = tileH * 0.5f;
    float minY = std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const float y = r * halfH;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
    }
    const float mapH = (maxY - minY) + static_cast<float>(tileH);
    return (screenH - mapH) * 0.5f - minY + 20.0f;
}

float IsoGrid::mapWidth() const {
    const float halfW = tileW * 0.5f;
    float minX = std::numeric_limits<float>::infinity();
    float maxX = -std::numeric_limits<float>::infinity();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const float x = c * tileW + (r % 2) * halfW;
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
        }
    }
    return (maxX - minX) + static_cast<float>(tileW);
}

float IsoGrid::mapHeight() const {
    const float halfH = tileH * 0.5f;
    float minY = std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const float y = r * halfH;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
    }
    return (maxY - minY) + static_cast<float>(tileH);
}

void IsoGrid::tileScreenPos(int row, int col, float& x, float& y) const {
    const float halfW = tileW * 0.5f;
    const float halfH = tileH * 0.5f;
    x = offsetX() + col * tileW + (row % 2) * halfW;
    y = offsetY() + row * halfH;
}
