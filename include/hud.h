#pragma once

#include <glad/glad.h>

#include "shader.h"

#include <string>

class Hud {
public:
    bool init();
    void setText(const std::string& text);
    void clear();
    void draw(const float screenProjection[16]) const;
    ~Hud();

private:
    Shader shader_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    int vertexCount_ = 0;
};
