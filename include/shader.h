// shader.h
// Contrato da classe Shader: compilar e usar os programas de shader do OpenGL
// (os programinhas que rodam na GPU).
#pragma once

#include <glad/glad.h>
#include <string>

class Shader {
public:
    // Identificador do programa de shader na GPU.
    GLuint program = 0;

    Shader() = default;
    ~Shader();

    // Compila e liga os shaders de vertice e fragmento num programa.
    bool loadFromSource(const char* vertexSrc, const char* fragmentSrc);
    // Ativa o programa para os proximos desenhos.
    void use() const;
    // Envio de uniforms (variaveis do shader): matriz, vec2 e inteiro.
    void setMat4(const char* name, const float* value) const;
    void setVec2(const char* name, float x, float y) const;
    void setInt(const char* name, int value) const;

private:
    // Compila um unico shader e devolve seu id (0 em caso de erro).
    GLuint compile(GLenum type, const char* source);
};
