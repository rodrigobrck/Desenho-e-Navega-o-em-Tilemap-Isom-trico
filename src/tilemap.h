#pragma once

#include "character.h"
#include "direction.h"
#include "isogrid.h"
#include "maploader.h"
#include "spritesheet.h"
#include "tileset.h"

#include <string>
#include <vector>

class Tilemap {
public:
    static constexpr int TILE_COUNT = 7;
    static constexpr int TILE_W = 64;
    static constexpr int TILE_H = 32;

    int rows = 3;
    int cols = 3;
    std::vector<int> map;
    GridPos player{1, 1};
    Character character;
    TileSet tileset;

    std::string tilesetName;
    int tileCount = TILE_COUNT;
    int tileSrcW = 256;
    int tileSrcH = 128;

    Tilemap();

    bool loadFromFile(const std::string& path);

    void setTileSheet(const SpriteSheet* sheet);

    int& at(int row, int col);
    int at(int row, int col) const;
    bool inBounds(int row, int col) const;

    GridPos neighbor(Direction dir) const;
    bool canMove(Direction dir) const;
    bool move(Direction dir);
    const char* blockedReason(Direction dir) const;

    void cycleTile();
    void toggleMode();
    enum class GameMode { Navigation, Editor, Puzzle, PegaPega };
    void setGameMode(GameMode m);
    GameMode gameMode() const;
    void visitTile(int row, int col);
    void changeTile(int delta);

    GridPos randomWalkableTile(GridPos exclude) const;

    IsoGrid grid() const;
    void getTileScreenPos(int row, int col, float& x, float& y) const;

private:
    void adopt(const MapData& data);
    GridPos offsetForDirection(Direction dir) const;
    GameMode mode_ = GameMode::Navigation;
};
