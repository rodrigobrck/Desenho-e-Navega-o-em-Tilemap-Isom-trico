// texture.h
// Contrato da classe Texture: carregar uma imagem e transforma-la numa textura
// de GPU pronta para uso.
#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>

class Texture {
public:
    GLuint id = 0;
    int width = 0;
    int height = 0;

    ~Texture();

    bool loadFromFile(const std::string& path);
    bool createFromPixels(int w, int h, const std::vector<unsigned char>& pixels);
    void bind(int unit = 0) const;
};
