#include "maploader.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

bool loadMapFile(const std::string& path, MapData& out) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "MapLoader: could not open map file '" << path << "'\n";
        return false;
    }

    auto nextLine = [&file](std::string& line) -> bool {
        while (std::getline(file, line)) {
            const size_t first = line.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) continue;
            if (line[first] == '#') continue;
            return true;
        }
        return false;
    };

    auto parseInt = [](const std::string& token, int& value) -> bool {
        std::istringstream is(token);
        return static_cast<bool>(is >> value) && is.eof();
    };

    std::string tileset;
    int count = 0, tw = 0, th = 0;
    {
        std::string headerLine;
        if (!nextLine(headerLine)) {
            std::cerr << "MapLoader: missing header in '" << path << "'\n";
            return false;
        }

        std::istringstream header(headerLine);
        std::vector<std::string> tokens;
        std::string token;
        while (header >> token) tokens.push_back(token);

        if (tokens.size() < 4) {
            std::cerr << "MapLoader: malformed header in '" << path
                      << "' (expected <tileset> <count> <w> <h>)\n";
            return false;
        }

        const size_t n = tokens.size();
        if (!parseInt(tokens[n - 3], count) || !parseInt(tokens[n - 2], tw) || !parseInt(tokens[n - 1], th)) {
            std::cerr << "MapLoader: malformed header in '" << path
                      << "' (count/width/height must be integers)\n";
            return false;
        }

        for (size_t i = 0; i + 3 < n; ++i) {
            if (i != 0) tileset += ' ';
            tileset += tokens[i];
        }

        if (count <= 0 || tw <= 0 || th <= 0) {
            std::cerr << "MapLoader: invalid header values in '" << path << "'\n";
            return false;
        }
    }

    std::vector<bool> walkable;
    int newRows = 0, newCols = 0;
    {
        std::string line;
        if (!nextLine(line)) {
            std::cerr << "MapLoader: missing dimensions in '" << path << "'\n";
            return false;
        }

        std::istringstream probe(line);
        std::string keyword;
        probe >> keyword;
        std::string lowered = keyword;
        for (char& ch : lowered) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        if (lowered == "walkable") {
            walkable.assign(count, false);
            int id;
            while (probe >> id) {
                if (id < 0 || id >= count) {
                    std::cerr << "MapLoader: walkable tile " << id << " out of range [0," << count - 1
                              << "] in '" << path << "'\n";
                    return false;
                }
                walkable[id] = true;
            }
            if (!nextLine(line)) {
                std::cerr << "MapLoader: missing dimensions in '" << path << "'\n";
                return false;
            }
        }

        std::istringstream dims(line);
        if (!(dims >> newRows >> newCols) || newRows <= 0 || newCols <= 0) {
            std::cerr << "MapLoader: invalid dimensions in '" << path << "'\n";
            return false;
        }
    }

    std::vector<int> tiles;
    tiles.reserve(static_cast<size_t>(newRows) * newCols);
    int value;
    while (tiles.size() < static_cast<size_t>(newRows) * newCols && (file >> value)) {
        if (value < 0 || value >= count) {
            std::cerr << "MapLoader: tile index " << value << " out of range [0," << count - 1
                      << "] in '" << path << "'\n";
            return false;
        }
        tiles.push_back(value);
    }

    if (tiles.size() != static_cast<size_t>(newRows) * newCols) {
        std::cerr << "MapLoader: expected " << newRows * newCols << " tiles but read "
                  << tiles.size() << " in '" << path << "'\n";
        return false;
    }

    out.tilesetName = tileset;
    out.tileCount = count;
    out.tileSrcW = tw;
    out.tileSrcH = th;
    out.rows = newRows;
    out.cols = newCols;
    out.tiles = std::move(tiles);
    out.walkable = std::move(walkable);
    return true;
}