// maploader.h
// Define a struct MapData (todos os dados de um mapa) e a funcao loadMapFile,
// que le esses dados de um arquivo de texto em Assets/Maps.
#pragma once

#include <string>
#include <vector>

// Todos os dados de um mapa lidos do arquivo: nome do tileset, quantidade e
// tamanho dos tiles, dimensoes da grade, os tiles em si e a mascara de caminhaveis.
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

// Le e valida um arquivo de mapa, preenchendo 'out'; devolve 'false' em caso de erro.
bool loadMapFile(const std::string& path, MapData& out);
