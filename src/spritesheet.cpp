// spritesheet.cpp
// Implementa o SpriteSheet: calcula as coordenadas de textura de um quadro
// especifico da imagem e monta os vertices daquele sprite. Tambem agrupa os
// sprites por textura (addSpriteToBatches) e resolve o caminho dos assets.
#include "spritesheet.h"

#include <filesystem>
#include <iostream>

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

void SpriteSheet::uvRect(int col, int row, float& u0, float& v0, float& u1, float& v1) const {
    u0 = static_cast<float>(col) / static_cast<float>(cols);
    u1 = static_cast<float>(col + 1) / static_cast<float>(cols);
    v0 = static_cast<float>(row) / static_cast<float>(rows);
    v1 = static_cast<float>(row + 1) / static_cast<float>(rows);
}

void SpriteSheet::addSprite(std::vector<SpriteVertex>& out,
                            float x, float y, int col, int row,
                            float scaleX, float scaleY) const {
    float u0, v0, u1, v1;
    uvRect(col, row, u0, v0, u1, v1);

    const float w = static_cast<float>(tileW) * scaleX;
    const float h = static_cast<float>(tileH) * scaleY;

    const SpriteVertex topLeft {x, y, u0, v0};
    const SpriteVertex topRight {x + w, y, u1, v0};
    const SpriteVertex bottomRight {x + w, y + h, u1, v1};
    const SpriteVertex bottomLeft {x, y + h, u0, v1};

    out.push_back(topLeft);
    out.push_back(topRight);
    out.push_back(bottomRight);

    out.push_back(topLeft);
    out.push_back(bottomRight);
    out.push_back(bottomLeft);
}

void addSpriteToBatches(std::vector<DrawBatch>& batches, const SpriteSheet* sheet,
                        float x, float y, int col, int row,
                        float scaleX, float scaleY) {
    if (!sheet || sheet->texture.id == 0) {
        return;
    }

    for (auto& batch : batches) {
        if (batch.texture == sheet->texture.id) {
            sheet->addSprite(batch.vertices, x, y, col, row, scaleX, scaleY);
            return;
        }
    }

    DrawBatch batch;
    batch.texture = sheet->texture.id;
    sheet->addSprite(batch.vertices, x, y, col, row, scaleX, scaleY);
    batches.push_back(std::move(batch));
}

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
