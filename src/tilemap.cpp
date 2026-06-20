// tilemap.cpp
// A logica do mapa. Decide se o jogador PODE se mover (consultando o terreno
// caminhavel), trata a paridade das linhas no movimento diagonal staggered,
// guarda o modo de jogo atual (Navegacao, Puzzle, Pega-Pega) e sorteia tiles
// caminhaveis para o alvo.
#include "tilemap.h"

#include "screen.h"

#include <random>
#include <vector>

// Copia para o Tilemap os dados ja validados pelo leitor de mapa e posiciona o
// jogador no centro da grade, olhando para o sul.
void Tilemap::adopt(const MapData& data) {
    tilesetName = data.tilesetName;
    tileCount = data.tileCount;
    tileSrcW = data.tileSrcW;
    tileSrcH = data.tileSrcH;
    rows = data.rows;
    cols = data.cols;
    map = data.tiles;
    tileset.setWalkable(data.walkable);
    player = {rows / 2, cols / 2};
    character.setFacing(Direction::S);
}

Tilemap::Tilemap() {
}

// Le o arquivo de mapa do disco e, se der certo, adota seus dados.
bool Tilemap::loadFromFile(const std::string& path) {
    MapData data;
    if (!loadMapFile(path, data)) {
        return false;
    }
    adopt(data);
    return true;
}

void Tilemap::setTileSheet(const SpriteSheet* sheet) {
    tileset.setSheet(sheet);
}

// Acesso ao tile em (row, col): a grade e guardada num vetor linear (row-major).
int& Tilemap::at(int row, int col) {
    return map[row * cols + col];
}

int Tilemap::at(int row, int col) const {
    return map[row * cols + col];
}

bool Tilemap::inBounds(int row, int col) const {
    return row >= 0 && row < rows && col >= 0 && col < cols;
}

// Calcula a celula vizinha numa direcao. Por ser um mapa staggered, as linhas
// impares ficam meio tile deslocadas, entao as diagonais dependem da paridade da
// linha atual (i_even) para escolher a coluna certa.
GridPos Tilemap::offsetForDirection(Direction dir) const {
    const int i = player.row;
    const int j = player.col;
    const bool i_even = (i % 2 == 0);

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

// Pode mover para a direcao? So se o vizinho existir e for um terreno caminhavel.
bool Tilemap::canMove(Direction dir) const {
    GridPos next = neighbor(dir);
    if (!inBounds(next.row, next.col)) {
        return false;
    }

    return tileset.isWalkable(at(next.row, next.col));
}

// Tenta mover o jogador: vira o personagem para a direcao e o reposiciona se o
// movimento for permitido. Devolve 'false' se estiver bloqueado.
bool Tilemap::move(Direction dir) {
    if (!canMove(dir)) {
        return false;
    }

    character.setFacing(dir);
    player = neighbor(dir);
    return true;
}

// Quando um movimento e bloqueado por terreno, devolve o nome desse terreno (para
// mensagens); devolve nullptr se o bloqueio foi por sair do mapa ou se nao houve.
const char* Tilemap::blockedReason(Direction dir) const {
    GridPos next = neighbor(dir);
    if (inBounds(next.row, next.col) && !tileset.isWalkable(at(next.row, next.col))) {
        return TileSet::name(at(next.row, next.col));
    }
    return nullptr;
}

void Tilemap::setGameMode(GameMode m) {
    mode_ = m;
}

Tilemap::GameMode Tilemap::gameMode() const {
    return mode_;
}

// Modo Puzzle: "evolui" o tile visitado para o proximo terreno, sem passar do
// ultimo (nao da a volta, ao contrario de cycleTile).
void Tilemap::visitTile(int row, int col) {
    if (!inBounds(row, col)) return;
    int& t = at(row, col);
    if (t < tileCount - 1) {
        ++t;
    }
}

// Troca o tile sob o jogador pelo proximo do tileset, dando a volta no fim (TAB).
void Tilemap::cycleTile() {
    int& tile = at(player.row, player.col);
    tile = (tile + 1) % tileCount;
}

// Sorteia um tile caminhavel qualquer (exceto a celula 'exclude'), usado para
// reposicionar o alvo do Pega-Pega longe do jogador.
GridPos Tilemap::randomWalkableTile(GridPos exclude) const {
    // Junta todos os tiles caminhaveis candidatos.
    std::vector<GridPos> candidates;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (r == exclude.row && c == exclude.col) continue;
            if (tileset.isWalkable(at(r, c))) candidates.push_back({r, c});
        }
    }
    if (candidates.empty()) return exclude;

    // Escolhe um candidato ao acaso (gerador criado uma unica vez).
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
    return candidates[dist(rng)];
}

// Monta o objeto IsoGrid (a matematica de posicionamento) para as dimensoes atuais.
IsoGrid Tilemap::grid() const {
    return IsoGrid{rows, cols, TILE_W, TILE_H, SCREEN_W, SCREEN_H};
}

// Converte uma celula (row, col) na sua posicao (x, y) na tela.
void Tilemap::getTileScreenPos(int row, int col, float& x, float& y) const {
    grid().tileScreenPos(row, col, x, y);
}
