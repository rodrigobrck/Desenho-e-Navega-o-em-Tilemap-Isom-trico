#pragma once

#include "spritesheet.h"

#include <vector>

struct TileSpriteRef {
    const SpriteSheet* sheet = nullptr;
    int col = 0;
    int row = 0;
};

class TileSet {
public:
    void setSheet(const SpriteSheet* sheet);
    void setWalkable(const std::vector<bool>& mask);

    TileSpriteRef spriteRef(int tileId) const;

    bool isWalkable(int tileId) const;
    static const char* name(int tileId);

private:
    const SpriteSheet* sheet_ = nullptr;
    std::vector<bool> walkable_;
};
