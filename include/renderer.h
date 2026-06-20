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
    SpriteSheet tileSheet;
    SpriteSheet character;

    Tilemap map;
    Hud hud;

    bool init(const std::string& mapFile);
    void setPegaPega(const PegaPega* pega);
    void setViewport(int width, int height);
    void uploadMesh();
    void draw();
    ~Renderer();

private:
    void buildBatches();

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    Shader shader_;
    std::vector<DrawBatch> batches_;
    const PegaPega* pega_ = nullptr;
    int viewportW_ = SCREEN_W;
    int viewportH_ = SCREEN_H;
};
