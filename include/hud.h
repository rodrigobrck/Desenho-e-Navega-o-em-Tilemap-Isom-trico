// hud.h
// Contrato do Hud: desenhar texto na tela (o placar e o cronometro do
// Pega-Pega), por cima da cena do jogo.
#pragma once

#include <glad/glad.h>

#include "shader.h"

#include <string>

class Hud {
public:
    // Prepara o HUD (shader e buffers).
    bool init();
    // Define o texto exibido / esconde o HUD.
    void setText(const std::string& text);
    void clear();
    // Desenha o texto usando a projecao de tela (em pixels).
    void draw(const float screenProjection[16]) const;
    ~Hud();

private:
    Shader shader_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    int vertexCount_ = 0;
};
