// renderer.cpp
// Junta todas as pecas do desenho. Carrega o tileset e o personagem, define os
// shaders, percorre o mapa montando a geometria do quadro (tiles + personagem +
// alvo) na ordem de profundidade correta (algoritmo do pintor) e faz o desenho
// final, alem de ajustar a camera para enquadrar o mapa na janela.
#include "renderer.h"

#include "gfx_math.h"

#include <algorithm>
#include <iostream>
#include <utility>

namespace {
// Dimensoes do spritesheet do personagem: 8 colunas (direcoes) x 12 linhas,
// cada quadro com 16x24 pixels.
constexpr int CHAR_SHEET_COLS = 8;
constexpr int CHAR_SHEET_ROWS = 12;
constexpr int CHAR_CELL_W = 16;
constexpr int CHAR_CELL_H = 24;

// Shader de vertice: aplica a projecao da camera e repassa a coordenada de textura.
const char* VERTEX_SHADER = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;

uniform mat4 uProjection;

out vec2 vTex;

void main() {
    vTex = aTex;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

// Shader de fragmento: amostra a textura e descarta pixels que devem ficar
// invisiveis (transparentes, totalmente pretos ou no rosa-magenta de "color key").
const char* FRAGMENT_SHADER = R"(
#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    vec4 color = texture(uTexture, vTex);

    if (color.a < 0.1) discard;
    if (color.r < 0.05 && color.g < 0.05 && color.b < 0.05) discard;
    if (color.r > 0.9 && color.g < 0.1 && color.b > 0.9) discard;

    FragColor = color;
}
)";
}

// Inicializa todo o renderer: compila shaders, carrega personagem/mapa/tileset,
// valida se o tileset comporta os tiles do mapa, prepara os buffers da GPU e o HUD.
bool Renderer::init(const std::string& mapFile) {
    // Compila e liga o programa de shader usado para desenhar os sprites.
    if (!shader_.loadFromSource(VERTEX_SHADER, FRAGMENT_SHADER)) {
        return false;
    }

    // Carrega o spritesheet do personagem e o entrega ao mapa.
    if (!character.load("Character/Small-8-Direction-Characters_by_AxulArt.png",
                        CHAR_SHEET_COLS, CHAR_SHEET_ROWS, CHAR_CELL_W, CHAR_CELL_H)) {
        std::cerr << "Failed to load character sprite sheet.\n";
        return false;
    }
    map.character.setSheet(&character);

    // Carrega o mapa do disco (define tileset, dimensoes e a grade de tiles).
    if (!map.loadFromFile(resolveAssetPath(std::string("Maps/") + mapFile))) {
        std::cerr << "Failed to load map '" << mapFile << "'.\n";
        return false;
    }

    // Carrega o tileset declarado no mapa, recortando-o pelo tamanho de cada tile.
    if (!tileSheet.loadAuto("Tiles/" + map.tilesetName, map.tileSrcW, map.tileSrcH)) {
        std::cerr << "Failed to load tileset '" << map.tilesetName << "' declared in the map.\n";
        return false;
    }

    // Garante que o tileset tem quadros suficientes para os tiles que o mapa usa.
    const int capacity = tileSheet.cols * tileSheet.rows;
    if (map.tileCount > capacity) {
        std::cerr << "Tileset '" << map.tilesetName << "' holds " << capacity
                  << " tiles (" << tileSheet.cols << "x" << tileSheet.rows
                  << ") but the map declares " << map.tileCount << ".\n";
        return false;
    }

    map.setTileSheet(&tileSheet);

    // Cria o VAO/VBO e descreve o layout dos vertices: posicao (2 floats) e
    // coordenada de textura (2 floats) intercaladas em cada SpriteVertex.
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Liga a transparencia alpha para os sprites se sobreporem corretamente.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Prepara o HUD e ja monta a primeira versao da malha do quadro.
    if (!hud.init()) {
        return false;
    }

    uploadMesh();
    return true;
}

void Renderer::setPegaPega(const PegaPega* pega) {
    pega_ = pega;
}

// Guarda o tamanho da janela (minimo 1 para evitar divisao por zero no enquadramento).
void Renderer::setViewport(int width, int height) {
    viewportW_ = std::max(width, 1);
    viewportH_ = std::max(height, 1);
}

// Reconstroi a geometria do quadro: percorre todos os tiles na ordem de
// profundidade (algoritmo do pintor) e empilha, em cada celula, o tile e, se for o
// caso, o alvo e o jogador.
void Renderer::buildBatches() {
    batches_.clear();

    // Lista todas as celulas da grade.
    std::vector<std::pair<int, int>> order;
    for (int r = 0; r < map.rows; ++r) {
        for (int c = 0; c < map.cols; ++c) {
            order.emplace_back(r, c);
        }
    }
    // Ordena por profundidade: quem tem menor (linha+coluna) e desenhado antes
    // (fica "atras"); empates sao resolvidos por linha e depois por coluna.
    std::sort(order.begin(), order.end(), [](const auto& a, const auto& b) {
        const int sa = a.first + a.second;
        const int sb = b.first + b.second;
        if (sa != sb) return sa < sb;
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    });

    // Fator de escala do tile de origem para o tamanho desejado na tela.
    const float tileScaleX = static_cast<float>(Tilemap::TILE_W) / static_cast<float>(map.tileSrcW);
    const float tileScaleY = static_cast<float>(Tilemap::TILE_H) / static_cast<float>(map.tileSrcH);

    // O alvo do Pega-Pega so e desenhado quando o mini-jogo esta de fato rodando.
    const bool pegaActive = pega_ != nullptr &&
                            map.gameMode() == Tilemap::GameMode::PegaPega &&
                            pega_->isRunning();

    // Para cada celula em ordem de profundidade, acrescenta os sprites ao batch.
    for (const auto& [r, c] : order) {
        // Sprite do terreno desta celula.
        const TileSpriteRef ref = map.tileset.spriteRef(map.at(r, c));
        float x, y;
        map.getTileScreenPos(r, c, x, y);
        addSpriteToBatches(batches_, ref.sheet, x, y, ref.col, ref.row, tileScaleX, tileScaleY);

        // Sobre a celula do alvo (se ativo), desenha o NPC do Pega-Pega.
        if (pegaActive && r == pega_->position().row && c == pega_->position().col) {
            pega_->npc.appendSprite(batches_, x, y, Tilemap::TILE_W, Tilemap::TILE_H);
        }

        // Sobre a celula do jogador, desenha o personagem.
        if (r == map.player.row && c == map.player.col) {
            map.character.appendSprite(batches_, x, y, Tilemap::TILE_W, Tilemap::TILE_H);
        }
    }
}

void Renderer::uploadMesh() {
    buildBatches();
}

// Desenha um quadro: limpa a tela, calcula a camera que enquadra o mapa, envia a
// projecao ao shader, desenha cada batch de sprites e, por fim, o HUD.
void Renderer::draw() {
    // Limpa a tela com a cor de fundo.
    glClearColor(0.15f, 0.18f, 0.22f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Calcula um zoom que faca o mapa inteiro caber na tela, com uma pequena margem.
    const IsoGrid grid = map.grid();
    const float mapW = grid.mapWidth();
    const float mapH = grid.mapHeight();
    const float margin = 1.08f;
    const float fit = std::max(mapW / SCREEN_W, mapH / SCREEN_H) * margin;
    const float zoom = std::max(fit, 1.0f);

    // Ajusta a regiao visivel ao formato da janela para nao distorcer o conteudo:
    // alarga em largura ou em altura conforme a proporcao da janela vs. do conteudo.
    const float contentW = SCREEN_W * zoom;
    const float contentH = SCREEN_H * zoom;
    const float contentAspect = contentW / contentH;
    const float windowAspect = static_cast<float>(viewportW_) / static_cast<float>(viewportH_);

    float regionW = contentW;
    float regionH = contentH;
    if (windowAspect > contentAspect) {
        regionW = contentH * windowAspect;
    } else {
        regionH = contentW / windowAspect;
    }

    // Centro do mundo, usado para centralizar a camera.
    const float cx = SCREEN_W * 0.5f;
    const float cy = SCREEN_H * 0.5f;

    // Projecao do mundo: enquadra a regiao calculada (Y invertido, origem no topo).
    float worldProj[16];
    orthoMat4(cx - regionW * 0.5f, cx + regionW * 0.5f,
              cy + regionH * 0.5f, cy - regionH * 0.5f, worldProj);

    // Projecao da tela: em pixels da janela, usada para desenhar o HUD por cima.
    float screenProj[16];
    orthoMat4(0.0f, static_cast<float>(viewportW_),
              static_cast<float>(viewportH_), 0.0f, screenProj);

    // Ativa o shader e envia a projecao do mundo e a unidade de textura 0.
    shader_.use();
    shader_.setMat4("uProjection", worldProj);
    shader_.setInt("uTexture", 0);

    glBindVertexArray(vao_);

    // Desenha cada batch (um por textura): envia seus vertices a GPU e os desenha.
    for (const auto& batch : batches_) {
        if (batch.vertices.empty()) continue;

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(batch.vertices.size() * sizeof(SpriteVertex)),
                     batch.vertices.data(), GL_DYNAMIC_DRAW);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, batch.texture);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(batch.vertices.size()));
    }

    glBindVertexArray(0);

    // Desenha o HUD por ultimo, para ficar acima da cena.
    hud.draw(screenProj);
}

// Libera os buffers da GPU ao destruir o renderer.
Renderer::~Renderer() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}
