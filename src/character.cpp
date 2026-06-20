// character.cpp
// Implementa o personagem: escolhe a coluna do spritesheet conforme a direcao
// em que ele olha, alterna os quadros da animacao "parado" e o posiciona em cima
// do tile (centrado e com os pes apoiados).
#include "character.h"

namespace {
// CHAR_SCALE: aumenta o sprite do personagem em relacao ao tamanho original.
// FOOT_PADDING: pequeno ajuste para "apoiar os pes" certinho sobre o tile.
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

// Avanca para o proximo quadro da animacao "parado", voltando ao inicio no fim.
void Character::advanceIdleFrame() {
    idleFrame_ = (idleFrame_ + 1) % IDLE_FRAME_COUNT;
}

// Mapeia cada direcao para a coluna correspondente no spritesheet (uma coluna por
// direcao, no sentido horario a partir do norte).
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

// Acrescenta o sprite do personagem aos batches de desenho, posicionado sobre o
// tile dado. A coluna vem da direcao em que olha; a linha, da paleta + quadro de
// animacao. Centraliza na horizontal e encosta os pes na base do tile.
void Character::appendSprite(std::vector<DrawBatch>& batches,
                             float tileX, float tileY, int tileW, int tileH) const {
    if (!sheet_) {
        return;
    }

    // Escolhe o quadro (coluna pela direcao, linha pela paleta e animacao).
    const int col = directionToColumn(facing_);
    const int row = paletteRow_ + IDLE_ROW_OFFSET + idleFrame_;
    // Tamanho do sprite ja escalado.
    const float w = static_cast<float>(sheet_->tileW) * CHAR_SCALE;
    const float h = static_cast<float>(sheet_->tileH) * CHAR_SCALE;
    // Centraliza no tile e apoia os pes na base.
    const float x = tileX + (tileW - w) * 0.5f;
    const float y = tileY + tileH - h - FOOT_PADDING;
    addSpriteToBatches(batches, sheet_, x, y, col, row, CHAR_SCALE, CHAR_SCALE);
}
