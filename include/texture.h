// texture.h
// Contrato da classe Texture: carregar uma imagem e transforma-la numa textura
// de GPU pronta para uso.
#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>

class Texture {
public:
    // Identificador da textura na GPU e suas dimensoes em pixels.
    GLuint id = 0;
    int width = 0;
    int height = 0;

    ~Texture();

    // Carrega de um arquivo de imagem.
    bool loadFromFile(const std::string& path);
    // Cria a textura a partir de pixels RGBA crus.
    bool createFromPixels(int w, int h, const std::vector<unsigned char>& pixels);
    // Ativa a textura numa unidade para o shader usa-la.
    void bind(int unit = 0) const;
};
