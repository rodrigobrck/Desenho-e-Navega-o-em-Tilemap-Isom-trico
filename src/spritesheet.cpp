// spritesheet.cpp
// Implementa o SpriteSheet: calcula as coordenadas de textura de um quadro
// especifico da imagem e monta os vertices daquele sprite. Tambem agrupa os
// sprites por textura (addSpriteToBatches) e resolve o caminho dos assets.
#include "spritesheet.h"

#include <filesystem>
#include <iostream>

// Carrega a imagem informando manualmente a grade (colunas x linhas) e o tamanho
// de cada quadro.
bool SpriteSheet::load(const std::string& path, int columns, int rows, int tileWidth, int tileHeight) {
    cols = columns;
    this->rows = rows;
    tileW = tileWidth;
    tileH = tileHeight;

    const std::string fullPath = resolveAssetPath(path);
    if (!texture.loadFromFile(fullPath)) {
        std::cerr << "SpriteSheet: failed to load " << fullPath << '\n';
        return false;
    }

    std::cout << "Loaded " << fullPath << " (" << texture.width << "x" << texture.height << ")\n";
    return true;
}

// Carrega a imagem e deduz automaticamente a grade (colunas x linhas) dividindo o
// tamanho da textura pelo tamanho de cada quadro.
bool SpriteSheet::loadAuto(const std::string& path, int tileWidth, int tileHeight) {
    if (tileWidth <= 0 || tileHeight <= 0) {
        std::cerr << "SpriteSheet: invalid cell size " << tileWidth << "x" << tileHeight
                  << " for " << path << '\n';
        return false;
    }

    tileW = tileWidth;
    tileH = tileHeight;

    const std::string fullPath = resolveAssetPath(path);
    if (!texture.loadFromFile(fullPath)) {
        std::cerr << "SpriteSheet: failed to load " << fullPath << '\n';
        return false;
    }

    // A textura precisa ser divisivel exatamente pelo tamanho do quadro.
    if (texture.width % tileWidth != 0 || texture.height % tileHeight != 0) {
        std::cerr << "SpriteSheet: " << fullPath << " size " << texture.width << "x" << texture.height
                  << " is not divisible by cell " << tileWidth << "x" << tileHeight << '\n';
        return false;
    }

    cols = texture.width / tileWidth;
    rows = texture.height / tileHeight;

    std::cout << "Loaded " << fullPath << " (" << texture.width << "x" << texture.height
              << ", grid " << cols << "x" << rows << ")\n";
    return true;
}

// Calcula as coordenadas de textura (UV) que delimitam o quadro (col, row) dentro
// da imagem, em valores normalizados de 0 a 1.
void SpriteSheet::uvRect(int col, int row, float& u0, float& v0, float& u1, float& v1) const {
    u0 = static_cast<float>(col) / static_cast<float>(cols);
    u1 = static_cast<float>(col + 1) / static_cast<float>(cols);
    v0 = static_cast<float>(row) / static_cast<float>(rows);
    v1 = static_cast<float>(row + 1) / static_cast<float>(rows);
}

// Gera os vertices de um sprite (um retangulo formado por dois triangulos) na
// posicao (x, y) e os adiciona em 'out'.
void SpriteSheet::addSprite(std::vector<SpriteVertex>& out,
                            float x, float y, int col, int row,
                            float scaleX, float scaleY) const {
    // UV do quadro e tamanho final ja escalado.
    float u0, v0, u1, v1;
    uvRect(col, row, u0, v0, u1, v1);

    const float w = static_cast<float>(tileW) * scaleX;
    const float h = static_cast<float>(tileH) * scaleY;

    // Os quatro cantos do retangulo.
    const SpriteVertex topLeft {x, y, u0, v0};
    const SpriteVertex topRight {x + w, y, u1, v0};
    const SpriteVertex bottomRight {x + w, y + h, u1, v1};
    const SpriteVertex bottomLeft {x, y + h, u0, v1};

    // Primeiro triangulo: superior-esquerdo, superior-direito, inferior-direito.
    out.push_back(topLeft);
    out.push_back(topRight);
    out.push_back(bottomRight);

    // Segundo triangulo: superior-esquerdo, inferior-direito, inferior-esquerdo.
    out.push_back(topLeft);
    out.push_back(bottomRight);
    out.push_back(bottomLeft);
}

// Adiciona um sprite ao batch da sua textura (agrupar por textura reduz as trocas
// de estado na GPU). Se ainda nao existir um batch para essa textura, cria um novo.
void addSpriteToBatches(std::vector<DrawBatch>& batches, const SpriteSheet* sheet,
                        float x, float y, int col, int row,
                        float scaleX, float scaleY) {
    if (!sheet || sheet->texture.id == 0) {
        return;
    }

    // Procura um batch ja existente com a mesma textura.
    for (auto& batch : batches) {
        if (batch.texture == sheet->texture.id) {
            sheet->addSprite(batch.vertices, x, y, col, row, scaleX, scaleY);
            return;
        }
    }

    // Nenhum batch compativel: cria um novo para esta textura.
    DrawBatch batch;
    batch.texture = sheet->texture.id;
    sheet->addSprite(batch.vertices, x, y, col, row, scaleX, scaleY);
    batches.push_back(std::move(batch));
}

// Resolve o caminho de um asset tentando alguns diretorios-base (a pasta atual e
// uma ou duas acima), o que faz o jogo achar os recursos seja qual for a pasta de
// onde foi executado. Se nada existir, devolve o caminho padrao.
std::string resolveAssetPath(const std::string& relativePath) {
    namespace fs = std::filesystem;

    const fs::path candidates[] = {
        fs::path("Assets") / relativePath,
        fs::path("..") / "Assets" / relativePath,
        fs::path("../..") / "Assets" / relativePath,
    };

    for (const auto& candidate : candidates) {
        if (fs::exists(candidate)) {
            return candidate.string();
        }
    }

    return (fs::path("Assets") / relativePath).string();
}
