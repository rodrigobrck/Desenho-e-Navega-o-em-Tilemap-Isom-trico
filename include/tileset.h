// tileset.h
// Contrato do TileSet: saber quais terrenos sao caminhaveis e onde cada tile
// esta no spritesheet. Tambem da um nome a cada tipo de terreno.
#pragma once

#include "spritesheet.h"

#include <vector>

// Onde encontrar o sprite de um tile: o spritesheet e o quadro (col, row).
struct TileSpriteRef {
    const SpriteSheet* sheet = nullptr;
    int col = 0;
    int row = 0;
};

class TileSet {
public:
    // Define o spritesheet dos tiles e a mascara de quais sao caminhaveis.
    void setSheet(const SpriteSheet* sheet);
    void setWalkable(const std::vector<bool>& mask);

    // Recorte do spritesheet correspondente a um id de tile.
    TileSpriteRef spriteRef(int tileId) const;

    // O tile e caminhavel? E qual o nome do seu terreno.
    bool isWalkable(int tileId) const;
    static const char* name(int tileId);

private:
    const SpriteSheet* sheet_ = nullptr;
    std::vector<bool> walkable_;
};
