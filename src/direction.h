#pragma once

enum class Direction {
    N, S, E, W, NE, NW, SE, SW
};

struct GridPos {
    int row = 0;
    int col = 0;
};
