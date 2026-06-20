// shader.cpp
// Implementa a classe Shader: compila os shaders de vertice e de fragmento,
// liga-os num programa de GPU e avisa com clareza caso ocorra erro de
// compilacao. Tambem oferece atalhos para enviar uniforms (matrizes, etc.).
#include "shader.h"

#include <iostream>

// Libera o programa de shader da GPU quando o objeto e destruido.
Shader::~Shader() {
    if (program != 0) {
        glDeleteProgram(program);
    }
}

// Compila um shader (vertice ou fragmento) a partir do codigo-fonte. Em caso de
// erro, imprime o log de compilacao e devolve 0.
GLuint Shader::compile(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Verifica se a compilacao deu certo.
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

// Compila os dois shaders e os liga num unico programa de GPU pronto para uso.
// Devolve 'false' se a compilacao ou a ligacao falhar.
bool Shader::loadFromSource(const char* vertexSrc, const char* fragmentSrc) {
    // Compila os shaders de vertice e de fragmento.
    GLuint vertex = compile(GL_VERTEX_SHADER, vertexSrc);
    GLuint fragment = compile(GL_FRAGMENT_SHADER, fragmentSrc);
    if (vertex == 0 || fragment == 0) {
        if (vertex) glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return false;
    }

    // Liga os dois shaders no programa final.
    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    // Os shaders ja podem ser descartados depois de ligados ao programa.
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Verifica se a ligacao deu certo.
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

// Ativa este programa para os proximos comandos de desenho.
void Shader::use() const {
    glUseProgram(program);
}

// Atalhos para enviar uniforms (variaveis do shader): uma matriz 4x4, um vec2 e um
// inteiro, respectivamente.
void Shader::setMat4(const char* name, const float* value) const {
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, value);
}

void Shader::setVec2(const char* name, float x, float y) const {
    glUniform2f(glGetUniformLocation(program, name), x, y);
}

void Shader::setInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(program, name), value);
}
