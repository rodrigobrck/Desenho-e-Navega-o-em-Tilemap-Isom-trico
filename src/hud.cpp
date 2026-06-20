// hud.cpp
// Desenha o texto do placar e do cronometro na tela, usando a stb_easy_font para
// gerar a geometria das letras. Tem seu proprio shader simples (cor solida) e
// desenha por cima da cena, em coordenadas de tela.
#include "hud.h"

#define STB_EASY_FONT_IMPLEMENTATION
#include <stb_easy_font.h>

#include <vector>

namespace {
// Fator de ampliacao do texto do HUD.
constexpr float HUD_SCALE = 1.6f;

// Vertice do HUD: posicao na tela mais uma cor RGBA (o texto e desenhado como
// geometria colorida, nao como textura).
struct ColorVertex {
    float x, y;
    float r, g, b, a;
};

// Shader de vertice do HUD: aplica a projecao de tela e repassa a cor.
const char* VERTEX_SHADER = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;

uniform mat4 uProjection;

out vec4 vColor;

void main() {
    vColor = aColor;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

// Shader de fragmento do HUD: apenas pinta com a cor recebida.
const char* FRAGMENT_SHADER = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main() {
    FragColor = vColor;
}
)";

// Gera a geometria de um texto: a stb_easy_font devolve quadrados (um por trecho de
// letra) que sao convertidos em triangulos, deslocados para (x, y) e escalados.
void appendText(float x, float y, const char* text,
                unsigned char r, unsigned char g, unsigned char b, unsigned char a,
                float scale, std::vector<ColorVertex>& out) {
    // Pede a stb_easy_font a lista de quadrados que formam o texto.
    unsigned char color[4] = {r, g, b, a};
    float buffer[4096];
    const int quads = stb_easy_font_print(0.0f, 0.0f, const_cast<char*>(text), color, buffer, sizeof(buffer));

    // Converte cada quadrado em dois triangulos (6 vertices).
    for (int q = 0; q < quads; ++q) {
        const float* quad = buffer + q * 16;
        const float tri[6][2] = {
            {quad[0], quad[1]}, {quad[4], quad[5]}, {quad[8], quad[9]},
            {quad[0], quad[1]}, {quad[8], quad[9]}, {quad[12], quad[13]},
        };

        // Posiciona e escala cada vertice e aplica a cor (normalizada para 0..1).
        for (const auto& v : tri) {
            ColorVertex cv;
            cv.x = x + v[0] * scale;
            cv.y = y + v[1] * scale;
            cv.r = r / 255.0f;
            cv.g = g / 255.0f;
            cv.b = b / 255.0f;
            cv.a = a / 255.0f;
            out.push_back(cv);
        }
    }
}
}

// Prepara o HUD: compila seu shader proprio e cria o VAO/VBO com o layout dos
// vertices (posicao + cor). Devolve 'false' se o shader falhar.
bool Hud::init() {
    if (!shader_.loadFromSource(VERTEX_SHADER, FRAGMENT_SHADER)) {
        return false;
    }

    // Ajusta o espacamento entre as letras da fonte.
    stb_easy_font_spacing(0.5f);

    // Cria os buffers e descreve o layout: posicao (2 floats) e cor (4 floats).
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    return true;
}

// Define o texto exibido: gera sua geometria (em amarelo, no canto superior
// esquerdo) e a envia para o buffer da GPU.
void Hud::setText(const std::string& text) {
    std::vector<ColorVertex> vertices;
    appendText(10.0f, 10.0f, text.c_str(), 255, 235, 90, 255, HUD_SCALE, vertices);

    vertexCount_ = static_cast<int>(vertices.size());

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(ColorVertex)),
                 vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Esconde o HUD zerando a contagem de vertices (nada e desenhado).
void Hud::clear() {
    vertexCount_ = 0;
}

// Desenha o texto do HUD usando a projecao de tela. Nao faz nada se estiver vazio.
void Hud::draw(const float screenProjection[16]) const {
    if (vertexCount_ <= 0) {
        return;
    }

    shader_.use();
    shader_.setMat4("uProjection", screenProjection);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    glBindVertexArray(0);
}

// Libera os buffers da GPU ao destruir o HUD.
Hud::~Hud() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}
