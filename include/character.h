// character.h
// Contrato do Character: a qual spritesheet ele pertence, para que direcao olha
// e qual quadro de animacao mostrar. Serve tanto para o jogador quanto para o
// NPC do Pega-Pega.
#pragma once

#include "types.h"
#include "spritesheet.h"

class Character {
public:
    // IDLE_FRAME_COUNT: quadros da animacao "parado". IDLE_ROW_OFFSET: a partir de
    // qual linha da paleta comecam esses quadros. DEFAULT_PALETTE_ROW: paleta de cor
    // padrao. PALETTE_BLOCK_ROWS: quantas linhas ocupa cada paleta (separa jogador e NPC).
    static constexpr int IDLE_FRAME_COUNT = 3;
    static constexpr int IDLE_ROW_OFFSET = 1;
    static constexpr int DEFAULT_PALETTE_ROW = 4;
    static constexpr int PALETTE_BLOCK_ROWS = 4;

    // Define / consulta o spritesheet de onde os quadros sao recortados.
    void setSheet(const SpriteSheet* sheet);
    const SpriteSheet* sheet() const;

    // Escolhe a paleta de cor (usada para diferenciar o NPC do jogador).
    void setPaletteRow(int row);

    // Define / consulta a direcao para a qual o personagem olha.
    void setFacing(Direction dir);
    Direction facing() const;

    // Avanca um quadro da animacao "parado".
    void advanceIdleFrame();

    // Acrescenta o sprite do personagem aos batches, posicionado sobre um tile.
    void appendSprite(std::vector<DrawBatch>& batches,
                      float tileX, float tileY, int tileW, int tileH) const;

private:
    // Coluna do spritesheet correspondente a uma direcao.
    int directionToColumn(Direction dir) const;

    const SpriteSheet* sheet_ = nullptr;
    Direction facing_ = Direction::S;
    int idleFrame_ = 0;
    int paletteRow_ = DEFAULT_PALETTE_ROW;
};
