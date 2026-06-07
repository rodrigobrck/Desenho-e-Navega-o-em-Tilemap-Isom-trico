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
