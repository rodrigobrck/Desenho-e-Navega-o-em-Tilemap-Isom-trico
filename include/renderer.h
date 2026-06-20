// renderer.h
// Contrato do Renderer: a classe central que junta todas as pecas (mapa,
// personagem, texturas e HUD), monta a geometria do quadro e a desenha na tela.
#pragma once

#include <glad/glad.h>

#include "hud.h"
#include "pegapega.h"
#include "screen.h"
#include "shader.h"
#include "spritesheet.h"
#include "tilemap.h"

#include <string>
#include <vector>

class Renderer {
public:
    // Spritesheets (tiles e personagem), o mapa e o HUD.
    SpriteSheet tileSheet;
    SpriteSheet character;

    Tilemap map;
    Hud hud;

    // Inicializa tudo (shaders, recursos, buffers) a partir de um arquivo de mapa.
    bool init(const std::string& mapFile);
    // Informa o mini-jogo Pega-Pega (para desenhar o alvo).
    void setPegaPega(const PegaPega* pega);
    // Atualiza o tamanho da janela usado no enquadramento.
    void setViewport(int width, int height);
    // Reconstroi a geometria do quadro (chamar apos qualquer mudanca visivel).
    void uploadMesh();
    // Desenha um quadro completo na tela.
    void draw();
    ~Renderer();

private:
    // Monta os batches de sprites na ordem de profundidade correta.
    void buildBatches();

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    Shader shader_;
    std::vector<DrawBatch> batches_;
    const PegaPega* pega_ = nullptr;
    int viewportW_ = SCREEN_W;
    int viewportH_ = SCREEN_H;
};
