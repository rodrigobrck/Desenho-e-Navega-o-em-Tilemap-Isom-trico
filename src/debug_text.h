#pragma once

#include "shader.h"
#include "tilemap.h"

#include <string>

class DebugText {
public:
    bool init();
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }
    void toggle();

    void buildTileLabels(const Tilemap& map);
    void setHud(const std::string& text);
    void clearHud();
    void draw(const float worldProjection[16], const float screenProjection[16]) const;

private:
    Shader shader_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    int vertexCount_ = 0;
    GLuint hudVao_ = 0;
    GLuint hudVbo_ = 0;
    int hudVertexCount_ = 0;
    bool enabled_ = false;
};
