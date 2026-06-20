#pragma once

#include <string>
#include <vector>

struct MapData {
    std::string tilesetName;
    int tileCount = 0;
    int tileSrcW = 0;
    int tileSrcH = 0;
    int rows = 0;
    int cols = 0;
    std::vector<int> tiles;
    std::vector<bool> walkable;
};

bool loadMapFile(const std::string& path, MapData& out);
