#include "tilemap.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

namespace {
constexpr int SCREEN_W = 800;
constexpr int SCREEN_H = 600;
constexpr float CHAR_SCALE = 1.5f;
constexpr float TILE_SRC_W = 256.0f;
constexpr float TILE_SRC_H = 128.0f;
constexpr float TILE_SCALE_X = static_cast<float>(Tilemap::TILE_W) / TILE_SRC_W;
constexpr float TILE_SCALE_Y = static_cast<float>(Tilemap::TILE_H) / TILE_SRC_H;
constexpr int CHAR_COLS = 8;
constexpr int CHAR_ANIM_ROW = 1;   // primeira linha de animacao (apos setas)
constexpr int CHAR_PALETTE_ROW = 4; // bloco azul (0=setas, 1-3=anim do 1o personagem)
}

bool Tilemap::isWalkable(int tileId) {
    // agua e floresta bloqueiam movimento
    return tileId != 4 && tileId != 6;
}

const char* Tilemap::tileName(int tileId) {
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


Tilemap::Tilemap() {
    // static map 20x10 (rows x cols) with fixed variety
    const int initial[20][10] = {
        {3,3,1,1,1,1,1,1,3,3},
        {3,1,1,1,1,1,1,1,1,3},
        {1,1,1,1,5,5,1,1,1,1},
        {1,1,6,6,1,1,6,6,1,1},
        {1,1,4,4,4,4,4,4,1,1},
        {1,1,4,4,4,4,4,4,1,1},
        {1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1},
        {1,1,1,2,2,2,1,1,1,1},
        {1,1,1,2,2,2,1,1,1,1},
        {5,5,1,1,1,1,1,1,5,5},
        {1,1,1,1,1,1,1,1,1,1},
        {6,6,6,1,1,1,1,6,6,6},
        {1,1,1,1,1,1,1,1,1,1},
        {1,2,2,2,1,1,2,2,2,1},
        {1,1,1,1,3,3,1,1,1,1},
        {3,3,1,1,1,1,1,1,3,3},
        {3,3,3,1,1,1,1,1,3,3},
        {0,0,1,1,1,1,1,1,0,0},
        {0,0,1,1,1,1,1,1,0,0}
    };

    rows = 20;
    cols = 10;
    map.resize(rows * cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            at(r, c) = initial[r][c];
        }
    }

    // place player near center
    player = {rows / 2, cols / 2};
}

void Tilemap::setTileSheets(const SpriteSheet* terrain1,
                            const SpriteSheet* terrain2,
                            const SpriteSheet* water,
                            const SpriteSheet* forest) {
    terrain1_ = terrain1;
    terrain2_ = terrain2;
    water_ = water;
    forest_ = forest;
}

void Tilemap::setCharacterSheet(const SpriteSheet* character) {
    character_ = character;
}

int& Tilemap::at(int row, int col) {
    return map[row * cols + col];
}

int Tilemap::at(int row, int col) const {
    return map[row * cols + col];
}

bool Tilemap::inBounds(int row, int col) const {
    return row >= 0 && row < rows && col >= 0 && col < cols;
}

GridPos Tilemap::offsetForDirection(Direction dir) const {
    const int i = player.row;
    const int j = player.col;
    const bool i_even = (i % 2 == 0);
    const bool j_even = (j % 2 == 0);

    switch (dir) {
        case Direction::N:  return {i - 2, j};
        case Direction::S:  return {i + 2, j};
        case Direction::E:  return {i, j + 1};
        case Direction::W:  return {i, j - 1};
        case Direction::NE: return {i - 1, i_even ? j : j + 1};
        case Direction::NW: return {i - 1, i_even ? j - 1 : j};
        case Direction::SE: return {i + 1, i_even ? j : j + 1};
        case Direction::SW: return {i + 1, i_even ? j - 1 : j};
    }

    return {i, j};
}

GridPos Tilemap::neighbor(Direction dir) const {
    return offsetForDirection(dir);
}

bool Tilemap::canMove(Direction dir) const {
    if (at(player.row, player.col) >= TILE_COUNT - 1) {
        return false;
    }

    GridPos next = neighbor(dir);
    if (!inBounds(next.row, next.col)) {
        return false;
    }

    return isWalkable(at(next.row, next.col));
}

bool Tilemap::move(Direction dir) {
    if (!canMove(dir)) {
        return false;
    }

    facing = dir;
    player = neighbor(dir);
    std::cout << "Player moved to (" << player.row << ", " << player.col << ") facing " << static_cast<int>(facing) << std::endl;
    return true;
}

void Tilemap::toggleMode() {
    if (mode_ == GameMode::Editor) mode_ = GameMode::Puzzle;
    else if (mode_ == GameMode::Puzzle) mode_ = GameMode::Editor;
}

void Tilemap::setGameMode(GameMode m) {
    mode_ = m;
}

Tilemap::GameMode Tilemap::gameMode() const {
    return mode_;
}

void Tilemap::visitTile(int row, int col) {
    if (!inBounds(row, col)) return;
    int& t = at(row, col);
    if (t < TILE_COUNT - 1) {
        ++t;
    }
}

void Tilemap::cycleTile() {
    int& tile = at(player.row, player.col);
    tile = (tile + 1) % TILE_COUNT;
}

void Tilemap::changeTile(int delta) {
    int& tile = at(player.row, player.col);
    int val = tile + delta;
    // wrap within [0, TILE_COUNT-1]
    val %= TILE_COUNT;
    if (val < 0) val += TILE_COUNT;
    tile = val;
}

TileSpriteRef Tilemap::tileRef(int tileId) const {
    // grade 3x6 por sheet (celula 256x128)
    switch (tileId) {
        case 0: return {terrain1_, 0, 2}; // terra
        case 1: return {terrain1_, 1, 0}; // grama
        case 2: return {terrain1_, 2, 3}; // pedra
        case 3: return {terrain2_, 0, 4}; // areia
        case 4: return {water_,   2, 5}; // agua
        case 5: return {terrain2_, 1, 2}; // barro
        case 6: return {forest_,  1, 1}; // floresta
        default: return {terrain1_, 1, 0};
    }
}

float Tilemap::mapOffsetX() const {
    const float halfW = TILE_W * 0.5f;
    float minX = std::numeric_limits<float>::infinity();
    float maxX = -std::numeric_limits<float>::infinity();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const float x = c * TILE_W + (r % 2) * halfW;
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
        }
    }
    const float mapW = (maxX - minX) + static_cast<float>(TILE_W);
    return (SCREEN_W - mapW) * 0.5f - minX;
}

float Tilemap::mapOffsetY() const {
    const float halfH = TILE_H * 0.5f;
    float minY = std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const float y = r * halfH;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
    }
    const float mapH = (maxY - minY) + static_cast<float>(TILE_H);
    return (SCREEN_H - mapH) * 0.5f - minY + 20.0f;
}

void Tilemap::tileScreenPos(int row, int col, float& x, float& y) const {
    const float halfW = TILE_W * 0.5f;
    const float halfH = TILE_H * 0.5f;
    x = mapOffsetX() + col * TILE_W + (row % 2) * halfW;
    y = mapOffsetY() + row * halfH;
}

void Tilemap::getTileScreenPos(int row, int col, float& x, float& y) const {
    tileScreenPos(row, col, x, y);
}

int Tilemap::directionToColumn(Direction dir) const {
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

void Tilemap::addToBatch(std::vector<DrawBatch>& batches, const SpriteSheet* sheet,
                         float x, float y, int col, int row,
                         float scaleX, float scaleY) const {
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

void Tilemap::buildBatches(std::vector<DrawBatch>& batches) const {
    batches.clear();

    std::vector<std::pair<int, int>> order;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            order.emplace_back(r, c);
        }
    }
    std::sort(order.begin(), order.end(), [](const auto& a, const auto& b) {
        const int sa = a.first + a.second;
        const int sb = b.first + b.second;
        if (sa != sb) return sa < sb;
        // tie-break deterministically by row then column to ensure stable draw order
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    });

    for (const auto& [r, c] : order) {
        const TileSpriteRef ref = tileRef(at(r, c));
        float x, y;
        tileScreenPos(r, c, x, y);
        addToBatch(batches, ref.sheet, x, y, ref.col, ref.row, TILE_SCALE_X, TILE_SCALE_Y);

        if (r == player.row && c == player.col && character_) {
            const int charCol = directionToColumn(facing);
            const int charRow = CHAR_PALETTE_ROW + CHAR_ANIM_ROW;
            const float charW = static_cast<float>(character_->tileW) * CHAR_SCALE;
            const float charH = static_cast<float>(character_->tileH) * CHAR_SCALE;
            const float charX = x + (TILE_W - charW) * 0.5f;

        
            const float charY = y + TILE_H - charH - 2.0f;
            addToBatch(batches, character_, charX, charY, charCol, charRow, CHAR_SCALE, CHAR_SCALE);
        }
    }
}
