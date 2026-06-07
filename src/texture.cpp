#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

Texture::~Texture() {
    if (id != 0) {
        glDeleteTextures(1, &id);
    }
}

bool Texture::loadFromFile(const std::string& path) {
    int channels = 0;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << " (" << stbi_failure_reason() << ")\n";
        return false;
    }

    std::vector<unsigned char> pixels(data, data + width * height * 4);
    stbi_image_free(data);
    return createFromPixels(width, height, pixels);
}

bool Texture::createFromPixels(int w, int h, const std::vector<unsigned char>& pixels) {
    width = w;
    height = h;

    if (id == 0) {
        glGenTextures(1, &id);
    }

    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void Texture::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}
