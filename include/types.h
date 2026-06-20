// types.h
// Tipos basicos compartilhados por todo o projeto: o enum Direction (as 8
// direcoes N, S, E, O e diagonais) e a struct GridPos (linha e coluna). Ficam
// isolados aqui para qualquer classe usa-los sem depender umas das outras.
#pragma once

// As 8 direcoes de movimento: 4 cardeais (N, S, E, W) e 4 diagonais.
enum class Direction {
    N, S, E, W, NE, NW, SE, SW
};

// Uma posicao na grade do mapa (linha e coluna).
struct GridPos {
    int row = 0;
    int col = 0;
};
