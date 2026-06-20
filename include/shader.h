// shader.h
// Contrato da classe Shader: compilar e usar os programas de shader do OpenGL
// (os programinhas que rodam na GPU).
#pragma once

#include <glad/glad.h>
#include <string>

class Shader {
public:
    GLuint program = 0;

    Shader() = default;
    ~Shader();

    bool loadFromSource(const char* vertexSrc, const char* fragmentSrc);
    void use() const;
    void setMat4(const char* name, const float* value) const;
    void setVec2(const char* name, float x, float y) const;
    void setInt(const char* name, int value) const;

private:
    GLuint compile(GLenum type, const char* source);
};
