// shader.cpp
// Implementa a classe Shader: compila os shaders de vertice e de fragmento,
// liga-os num programa de GPU e avisa com clareza caso ocorra erro de
// compilacao. Tambem oferece atalhos para enviar uniforms (matrizes, etc.).
#include "shader.h"

#include <iostream>

Shader::~Shader() {
    if (program != 0) {
        glDeleteProgram(program);
    }
}

GLuint Shader::compile(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error:\n" << log << '\n';
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::loadFromSource(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertex = compile(GL_VERTEX_SHADER, vertexSrc);
    GLuint fragment = compile(GL_FRAGMENT_SHADER, fragmentSrc);
    if (vertex == 0 || fragment == 0) {
        if (vertex) glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return false;
    }

    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Program link error:\n" << log << '\n';
        glDeleteProgram(program);
        program = 0;
        return false;
    }

    return true;
}

void Shader::use() const {
    glUseProgram(program);
}

void Shader::setMat4(const char* name, const float* value) const {
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, value);
}

void Shader::setVec2(const char* name, float x, float y) const {
    glUniform2f(glGetUniformLocation(program, name), x, y);
}

void Shader::setInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(program, name), value);
}
