// maploader.cpp
// Le o arquivo de mapa em texto (cabecalho com tileset/tamanho, lista de tiles
// caminhaveis, dimensoes e a grade de numeros) e valida tudo, reportando erros
// claros. E o que torna o jogo data-driven: da para criar mapas novos sem
// recompilar.
#include "maploader.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

// Le e valida um arquivo de mapa em texto, preenchendo 'out'. Em caso de erro,
// imprime uma mensagem clara e devolve 'false' sem alterar nada util.
bool loadMapFile(const std::string& path, MapData& out) {
    // Abre o arquivo.
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "MapLoader: could not open map file '" << path << "'\n";
        return false;
    }

    // Auxiliar: devolve a proxima linha "util", pulando linhas vazias e comentarios (#).
    auto nextLine = [&file](std::string& line) -> bool {
        while (std::getline(file, line)) {
            const size_t first = line.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) continue;
            if (line[first] == '#') continue;
            return true;
        }
        return false;
    };

    // Auxiliar: converte um token em inteiro, exigindo que o token seja so o numero.
    auto parseInt = [](const std::string& token, int& value) -> bool {
        std::istringstream is(token);
        return static_cast<bool>(is >> value) && is.eof();
    };

    // --- Cabecalho: <nome do tileset> <quantidade de tiles> <largura> <altura> ---
    // O nome do tileset pode conter espacos, por isso os 3 numeros sao lidos do fim.
    std::string tileset;
    int count = 0, tw = 0, th = 0;
    {
        std::string headerLine;
        if (!nextLine(headerLine)) {
            std::cerr << "MapLoader: missing header in '" << path << "'\n";
            return false;
        }

        // Quebra a linha do cabecalho em tokens separados por espaco.
        std::istringstream header(headerLine);
        std::vector<std::string> tokens;
        std::string token;
        while (header >> token) tokens.push_back(token);

        if (tokens.size() < 4) {
            std::cerr << "MapLoader: malformed header in '" << path
                      << "' (expected <tileset> <count> <w> <h>)\n";
            return false;
        }

        // Os tres ultimos tokens sao os numeros (quantidade, largura, altura).
        const size_t n = tokens.size();
        if (!parseInt(tokens[n - 3], count) || !parseInt(tokens[n - 2], tw) || !parseInt(tokens[n - 1], th)) {
            std::cerr << "MapLoader: malformed header in '" << path
                      << "' (count/width/height must be integers)\n";
            return false;
        }

        // O que sobra no inicio (podendo ter espacos) e o nome do tileset.
        for (size_t i = 0; i + 3 < n; ++i) {
            if (i != 0) tileset += ' ';
            tileset += tokens[i];
        }

        // Os valores numericos precisam ser positivos.
        if (count <= 0 || tw <= 0 || th <= 0) {
            std::cerr << "MapLoader: invalid header values in '" << path << "'\n";
            return false;
        }
    }

    // --- Linha opcional "walkable" e linha de dimensoes (linhas e colunas) ---
    std::vector<bool> walkable;
    int newRows = 0, newCols = 0;
    {
        std::string line;
        if (!nextLine(line)) {
            std::cerr << "MapLoader: missing dimensions in '" << path << "'\n";
            return false;
        }

        // Espia a primeira palavra da linha (em minusculas) para ver se e "walkable".
        std::istringstream probe(line);
        std::string keyword;
        probe >> keyword;
        std::string lowered = keyword;
        for (char& ch : lowered) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        // Se houver, le os ids de tiles caminhaveis e marca a mascara; depois avanca
        // para a proxima linha (a de dimensoes).
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

        // Le as dimensoes da grade (numero de linhas e de colunas).
        std::istringstream dims(line);
        if (!(dims >> newRows >> newCols) || newRows <= 0 || newCols <= 0) {
            std::cerr << "MapLoader: invalid dimensions in '" << path << "'\n";
            return false;
        }
    }

    // --- Grade de tiles: le exatamente rows*cols numeros, validando cada indice ---
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

    // Confere se a quantidade de tiles lidos bate com o tamanho declarado.
    if (tiles.size() != static_cast<size_t>(newRows) * newCols) {
        std::cerr << "MapLoader: expected " << newRows * newCols << " tiles but read "
                  << tiles.size() << " in '" << path << "'\n";
        return false;
    }

    // --- Tudo validado: entrega os dados pelo parametro de saida ---
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