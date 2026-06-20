#pragma once

#include "types.h"
#include "spritesheet.h"

class Character {
public:
    static constexpr int IDLE_FRAME_COUNT = 3;
    static constexpr int IDLE_ROW_OFFSET = 1;
    static constexpr int DEFAULT_PALETTE_ROW = 4;
    static constexpr int PALETTE_BLOCK_ROWS = 4;

    void setSheet(const SpriteSheet* sheet);
    const SpriteSheet* sheet() const;

    void setPaletteRow(int row);

    void setFacing(Direction dir);
    Direction facing() const;

    void advanceIdleFrame();

    void appendSprite(std::vector<DrawBatch>& batches,
                      float tileX, float tileY, int tileW, int tileH) const;

private:
    int directionToColumn(Direction dir) const;

    const SpriteSheet* sheet_ = nullptr;
    Direction facing_ = Direction::S;
    int idleFrame_ = 0;
    int paletteRow_ = DEFAULT_PALETTE_ROW;
};
