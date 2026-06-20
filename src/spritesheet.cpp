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
    const float tw = static_cast<float>(texture.width) / static_cast<float>(cols);
    const float th = static_cast<float>(texture.height) / static_cast<float>(rows);

    u0 = (col * tw) / static_cast<float>(texture.width);
    u1 = ((col + 1) * tw) / static_cast<float>(texture.width);
    v0 = (row * th) / static_cast<float>(texture.height);
    v1 = ((row + 1) * th) / static_cast<float>(texture.height);
}

void SpriteSheet::addSprite(std::vector<SpriteVertex>& out,
                            float x, float y, int col, int row,
                            float scaleX, float scaleY) const {
    float u0, v0, u1, v1;
    uvRect(col, row, u0, v0, u1, v1);

    const float w = static_cast<float>(tileW) * scaleX;
    const float h = static_cast<float>(tileH) * scaleY;

    out.push_back({x,     y,     u0, v0});
    out.push_back({x + w, y,     u1, v0});
    out.push_back({x + w, y + h, u1, v1});

    out.push_back({x,     y,     u0, v0});
    out.push_back({x + w, y + h, u1, v1});
    out.push_back({x,     y + h, u0, v1});
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
