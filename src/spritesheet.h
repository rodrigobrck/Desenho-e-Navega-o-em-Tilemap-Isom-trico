#pragma once

#include "texture.h"

#include <string>
#include <vector>

struct SpriteVertex {
    float x, y;
    float u, v;
};

struct DrawBatch {
    GLuint texture = 0;
    std::vector<SpriteVertex> vertices;
};

class SpriteSheet {
public:
    Texture texture;
    int cols = 1;
    int rows = 1;
    int tileW = 0;
    int tileH = 0;

    bool load(const std::string& path, int columns, int rows, int tileWidth, int tileHeight);

    bool loadAuto(const std::string& path, int tileWidth, int tileHeight);

    void uvRect(int col, int row, float& u0, float& v0, float& u1, float& v1) const;

    void addSprite(std::vector<SpriteVertex>& out,
                   float x, float y, int col, int row,
                   float scaleX, float scaleY) const;
};

void addSpriteToBatches(std::vector<DrawBatch>& batches, const SpriteSheet* sheet,
                        float x, float y, int col, int row,
                        float scaleX, float scaleY);

std::string resolveAssetPath(const std::string& relativePath);
