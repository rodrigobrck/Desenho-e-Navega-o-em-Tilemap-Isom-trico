// types.h
// Tipos basicos compartilhados por todo o projeto: o enum Direction (as 8
// direcoes N, S, E, O e diagonais) e a struct GridPos (linha e coluna). Ficam
// isolados aqui para qualquer classe usa-los sem depender umas das outras.
#pragma once

enum class Direction {
    N, S, E, W, NE, NW, SE, SW
};

struct GridPos {
    int row = 0;
    int col = 0;
};
