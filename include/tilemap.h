// tilemap.h
// Contrato do Tilemap: o estado logico do mapa (a grade de tiles, a posicao do
// jogador e o modo de jogo) e as operacoes de movimento, que decidem se o
// jogador pode andar para um tile e tratam a paridade das linhas no staggered.
#pragma once

#include "character.h"
#include "types.h"
#include "isogrid.h"
#include "maploader.h"
#include "spritesheet.h"
#include "tileset.h"

#include <string>
#include <vector>

class Tilemap {
public:
    // Numero padrao de tipos de tile e o tamanho (em pixels) com que cada tile e
    // desenhado na tela.
    static constexpr int TILE_COUNT = 7;
    static constexpr int TILE_W = 64;
    static constexpr int TILE_H = 32;

    // Estado do mapa: dimensoes, grade de tiles, posicao do jogador, o personagem
    // e o conjunto de tiles (com a info de quais sao caminhaveis).
    int rows = 0;
    int cols = 0;
    std::vector<int> map;
    GridPos player{0, 0};
    Character character;
    TileSet tileset;

    // Metadados do tileset declarados no arquivo de mapa.
    std::string tilesetName;
    int tileCount = TILE_COUNT;
    int tileSrcW = 256;
    int tileSrcH = 128;

    Tilemap();

    // Carrega o mapa de um arquivo de texto.
    bool loadFromFile(const std::string& path);

    // Define o spritesheet usado pelos tiles.
    void setTileSheet(const SpriteSheet* sheet);

    // Acesso a um tile da grade e teste de limites.
    int& at(int row, int col);
    int at(int row, int col) const;
    bool inBounds(int row, int col) const;

    // Operacoes de movimento: vizinho numa direcao, se pode mover, mover de fato e
    // o motivo de um eventual bloqueio.
    GridPos neighbor(Direction dir) const;
    bool canMove(Direction dir) const;
    bool move(Direction dir);
    const char* blockedReason(Direction dir) const;

    // Troca o tile sob o jogador pelo proximo (dando a volta).
    void cycleTile();
    // Modos de jogo: navegacao livre, puzzle (evolui tiles) e o mini-jogo Pega-Pega.
    enum class GameMode { Navigation, Puzzle, PegaPega };
    void setGameMode(GameMode m);
    GameMode gameMode() const;
    // No modo Puzzle, evolui o tile visitado para o proximo terreno.
    void visitTile(int row, int col);

    // Sorteia um tile caminhavel (exceto 'exclude') para o alvo do Pega-Pega.
    GridPos randomWalkableTile(GridPos exclude) const;

    // Acesso a matematica de posicionamento isometrico.
    IsoGrid grid() const;
    void getTileScreenPos(int row, int col, float& x, float& y) const;

private:
    // Copia os dados de um MapData ja validado para este mapa.
    void adopt(const MapData& data);
    // Calcula a celula vizinha tratando a paridade das linhas (staggered).
    GridPos offsetForDirection(Direction dir) const;
    GameMode mode_ = GameMode::Navigation;
};
