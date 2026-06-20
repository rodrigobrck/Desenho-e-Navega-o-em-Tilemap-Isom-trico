// spritesheet.h
// Define SpriteSheet (uma imagem com varios quadros) e as structs de desenho
// (SpriteVertex e DrawBatch). E a base de como recortamos e agrupamos os
// sprites antes de envia-los para a GPU.
#pragma once

#include "texture.h"

#include <string>
#include <vector>

// Um vertice de sprite: posicao na tela (x, y) e coordenada de textura (u, v).
struct SpriteVertex {
    float x, y;
    float u, v;
};

// Um lote de desenho: todos os vertices que compartilham a mesma textura.
struct DrawBatch {
    GLuint texture = 0;
    std::vector<SpriteVertex> vertices;
};

class SpriteSheet {
public:
    // A textura e a grade de quadros (colunas x linhas) com o tamanho de cada um.
    Texture texture;
    int cols = 1;
    int rows = 1;
    int tileW = 0;
    int tileH = 0;

    // Carrega informando a grade manualmente.
    bool load(const std::string& path, int columns, int rows, int tileWidth, int tileHeight);

    // Carrega deduzindo a grade pelo tamanho do quadro.
    bool loadAuto(const std::string& path, int tileWidth, int tileHeight);

    // Coordenadas de textura (UV) de um quadro.
    void uvRect(int col, int row, float& u0, float& v0, float& u1, float& v1) const;

    // Gera os vertices de um sprite e os acrescenta em 'out'.
    void addSprite(std::vector<SpriteVertex>& out,
                   float x, float y, int col, int row,
                   float scaleX, float scaleY) const;
};

// Acrescenta um sprite ao batch da sua textura (criando um novo batch se preciso).
void addSpriteToBatches(std::vector<DrawBatch>& batches, const SpriteSheet* sheet,
                        float x, float y, int col, int row,
                        float scaleX, float scaleY);

// Resolve o caminho de um asset procurando-o em alguns diretorios-base.
std::string resolveAssetPath(const std::string& relativePath);
