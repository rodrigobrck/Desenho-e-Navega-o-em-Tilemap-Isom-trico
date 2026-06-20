// tileset.cpp
// Implementa o TileSet. Responde duas perguntas simples: "esse terreno e
// caminhavel?" e "qual pedaco do spritesheet corresponde a esse tile?". Tambem
// devolve o nome de cada tipo de terreno (grama, agua, etc.).
#include "tileset.h"

void TileSet::setSheet(const SpriteSheet* sheet) {
    sheet_ = sheet;
}

void TileSet::setWalkable(const std::vector<bool>& mask) {
    walkable_ = mask;
}

TileSpriteRef TileSet::spriteRef(int tileId) const {
    if (!sheet_ || sheet_->cols <= 0) {
        return {sheet_, 0, 0};
    }
    return {sheet_, tileId % sheet_->cols, tileId / sheet_->cols};
}

bool TileSet::isWalkable(int tileId) const {
    if (walkable_.empty()) {
        return true;
    }
    if (tileId < 0 || tileId >= static_cast<int>(walkable_.size())) {
        return false;
    }
    return walkable_[tileId];
}

const char* TileSet::name(int tileId) {
    switch (tileId) {
        case 0: return "terra";
        case 1: return "grama";
        case 2: return "pedra";
        case 3: return "areia";
        case 4: return "agua";
        case 5: return "barro";
        case 6: return "floresta";
        default: return "desconhecido";
    }
}
