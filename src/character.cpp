#include "character.h"

namespace {
constexpr float CHAR_SCALE = 1.5f;
constexpr float FOOT_PADDING = 2.0f;
}

void Character::setSheet(const SpriteSheet* sheet) {
    sheet_ = sheet;
}

const SpriteSheet* Character::sheet() const {
    return sheet_;
}

void Character::setPaletteRow(int row) {
    paletteRow_ = row;
}

void Character::setFacing(Direction dir) {
    facing_ = dir;
}

Direction Character::facing() const {
    return facing_;
}

void Character::advanceIdleFrame() {
    idleFrame_ = (idleFrame_ + 1) % IDLE_FRAME_COUNT;
}

int Character::directionToColumn(Direction dir) const {
    switch (dir) {
        case Direction::N:  return 0;
        case Direction::NE: return 1;
        case Direction::E:  return 2;
        case Direction::SE: return 3;
        case Direction::S:  return 4;
        case Direction::SW: return 5;
        case Direction::W:  return 6;
        case Direction::NW: return 7;
    }
    return 4;
}

void Character::appendSprite(std::vector<DrawBatch>& batches,
                             float tileX, float tileY, int tileW, int tileH) const {
    if (!sheet_) {
        return;
    }

    const int col = directionToColumn(facing_);
    const int row = paletteRow_ + IDLE_ROW_OFFSET + idleFrame_;
    const float w = static_cast<float>(sheet_->tileW) * CHAR_SCALE;
    const float h = static_cast<float>(sheet_->tileH) * CHAR_SCALE;
    const float x = tileX + (tileW - w) * 0.5f;
    const float y = tileY + tileH - h - FOOT_PADDING;
    addSpriteToBatches(batches, sheet_, x, y, col, row, CHAR_SCALE, CHAR_SCALE);
}
