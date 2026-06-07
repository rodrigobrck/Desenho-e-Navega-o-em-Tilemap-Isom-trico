#pragma once

#include "spritesheet.h"

#include <vector>

enum class Direction {
    N, S, E, W, NE, NW, SE, SW
};

struct GridPos {
    int row = 0;
    int col = 0;
};

struct TileSpriteRef {
    const SpriteSheet* sheet = nullptr;
    int col = 0;
    int row = 0;
};

struct DrawBatch {
    GLuint texture = 0;
    std::vector<SpriteVertex> vertices;
};

class Tilemap {
public:
    static constexpr int TILE_COUNT = 7;
    // Render tiles at 64x32 on screen (matches spritesheet 256x128 with scale 0.25)
    static constexpr int TILE_W = 64;
    static constexpr int TILE_H = 32;

    int rows = 3;
    int cols = 3;
    std::vector<int> map;
    GridPos player{1, 1};
    Direction facing = Direction::S;

    Tilemap();

    void setTileSheets(const SpriteSheet* terrain1,
                       const SpriteSheet* terrain2,
                       const SpriteSheet* water,
                       const SpriteSheet* forest);

    void setCharacterSheet(const SpriteSheet* character);

    // 0=terra 1=grama 2=pedra 3=areia 4=agua 5=barro 6=floresta
    static bool isWalkable(int tileId);
    static const char* tileName(int tileId);

    int& at(int row, int col);
    int at(int row, int col) const;
    bool inBounds(int row, int col) const;

    GridPos neighbor(Direction dir) const;
    bool canMove(Direction dir) const;
    bool move(Direction dir);
    void cycleTile();
    void toggleMode();
    enum class GameMode { Navigation, Editor, Puzzle };
    void setGameMode(GameMode m);
    GameMode gameMode() const;
    // When visiting a tile in Puzzle mode, advance its state towards the blocking state
    void visitTile(int row, int col);
    // Change the current tile at the player position by delta (e.g., +1 or -1)
    void changeTile(int delta);

    void buildBatches(std::vector<DrawBatch>& batches) const;
    void getTileScreenPos(int row, int col, float& x, float& y) const;

private:
    const SpriteSheet* terrain1_ = nullptr;
    const SpriteSheet* terrain2_ = nullptr;
    const SpriteSheet* water_ = nullptr;
    const SpriteSheet* forest_ = nullptr;
    const SpriteSheet* character_ = nullptr;

    TileSpriteRef tileRef(int tileId) const;
    float mapOffsetX() const;
    float mapOffsetY() const;
    void tileScreenPos(int row, int col, float& x, float& y) const;
    GridPos offsetForDirection(Direction dir) const;
    int directionToColumn(Direction dir) const;
    void addToBatch(std::vector<DrawBatch>& batches, const SpriteSheet* sheet,
                    float x, float y, int col, int row,
                    float scaleX, float scaleY) const;
    GameMode mode_ = GameMode::Navigation;
};
