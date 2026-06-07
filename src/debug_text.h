#pragma once

#include "shader.h"
#include "tilemap.h"

class DebugText {
public:
    bool init();
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }
    void toggle();

    void buildTileLabels(const Tilemap& map);
    void draw(const float projection[16]) const;

private:
    Shader shader_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    int vertexCount_ = 0;
    bool enabled_ = false;
};
