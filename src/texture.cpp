// texture.cpp
// Implementa a classe Texture. Usa a biblioteca stb_image para ler o arquivo PNG
// e envia esses pixels para a GPU como uma textura. Usamos filtro NEAREST de
// proposito, para manter o visual em pixel art nitido, sem borrar.
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

// Libera a textura da GPU quando o objeto e destruido.
Texture::~Texture() {
    if (id != 0) {
        glDeleteTextures(1, &id);
    }
}

// Le um arquivo de imagem do disco (via stb_image, forcando 4 canais RGBA) e o
// envia para a GPU. Devolve 'false' se a leitura falhar.
bool Texture::loadFromFile(const std::string& path) {
    int channels = 0;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << " (" << stbi_failure_reason() << ")\n";
        return false;
    }

    // Copia os pixels para um vetor proprio e libera o buffer da stb_image.
    std::vector<unsigned char> pixels(data, data + width * height * 4);
    stbi_image_free(data);
    return createFromPixels(width, height, pixels);
}

// Cria (ou atualiza) a textura na GPU a partir de pixels RGBA crus. Usa filtro
// NEAREST para manter a pixel art nitida, sem borrar ao ampliar.
bool Texture::createFromPixels(int w, int h, const std::vector<unsigned char>& pixels) {
    width = w;
    height = h;

    // Gera o id da textura na primeira vez.
    if (id == 0) {
        glGenTextures(1, &id);
    }

    // Configura os parametros e envia os pixels para a GPU.
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

// Ativa esta textura na unidade indicada, para o shader amostra-la ao desenhar.
void Texture::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}
