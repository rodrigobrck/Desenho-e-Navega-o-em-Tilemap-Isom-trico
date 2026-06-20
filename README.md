# Mapa Isométrico Staggered — OpenGL + GLAD

Trabalho desenvolvido para a disciplina **Processamento Gráfico**.

## 👥 Integrantes do Grupo

- Rodrigo Brocker Nunes
- João Miguel Junges Oliveira
- Arthur Zani Lemos

## 🚀 Como executar

> Este programa foi feito em **linguagem C++ (C++17)** e usa **OpenGL 3.3**, **GLFW**, **GLAD** e **stb**. As dependências já acompanham o projeto na pasta `external/`.

**Opção 1 — Executar o binário já compilado:**

```powershell
.\build-debug\staggered_map.exe
# ou escolhendo um mapa (caminho relativo a Assets/):
.\build-debug\staggered_map.exe Maps\map_exemplo_3x3.txt
```

**Opção 2 — Compilar e executar pelo VS Code:**

1. Abra a pasta do projeto no **VS Code** (com o compilador **MinGW/g++** instalado).
2. Rode a tarefa de build **`build-mingw`** (menu *Terminal → Run Build Task...*, ou `Ctrl+Shift+B`).
3. Execute a tarefa **`run-mingw`** ou rode o `.exe` gerado em `build-debug/`.

**Opção 3 — Compilar pelo terminal (g++):**

```powershell
g++ -std=c++17 -static -static-libgcc -static-libstdc++ `
  src/*.cpp external/glad/src/glad.c `
  -I include -I external/glad/include -I external/GLFW/include -I external/stb `
  -L C:/msys64/mingw64/lib -lglfw3 -lopengl32 -lgdi32 -lwinmm `
  -o build-debug/staggered_map.exe
```

## 💡 Sobre o Programa

O programa renderiza um **mapa isométrico no layout *staggered*** (as linhas ímpares deslocam meio tile na horizontal) usando OpenGL moderno. O personagem se movimenta em **8 direções** sobre o mapa, e a aplicação possui diferentes modos de uso:

- **Navegação:** mova o personagem pelo mapa (apenas tiles caminháveis).
- **Puzzle:** os tiles mudam enquanto o jogar caminha pelo mapa. Ainda não há um objetivo claro nesse modo, a não ser mostrar o entendimento sobre o tileset.
- **Pega-Pega:** um NPC aparece em um tile aleatório e o objetivo é capturá-lo dentro do tempo, com um **HUD** mostrando o cronômetro e o número de capturas. Cada captura adiciona 2 seguntos ao timer, ao chegar em 10 capturas, o jogador ganha, se o timer zerar, o jogador perde.

Os mapas são **lidos de arquivos de configuração** em `Assets/Maps/`, definindo o tileset, a quantidade e o tamanho dos tiles, quais são caminháveis e a grade em si — então é possível criar novos mapas sem recompilar.

### Controles

| Tecla | Ação |
|-------|------|
| **Setas** ou **W A S D** | Norte, Sul, Oeste, Leste |
| **Setas/WASD combinadas** (ex. ↑+→) | Move nas diagonais (NE, NO, SE, SO) |
| **Y** | Alterna **Navegação ↔ Puzzle** |
| **TAB** | Alterna (cicla) o tile da célula atual |
| **P** | Inicia o modo **Pega-Pega** |
| **ESC** | Sair |

## 📌 Observações

- O código-fonte (`.cpp`) está em `src/` e os cabeçalhos (`.h`) em `include/`.
- Os assets (tilesets, personagem e mapas) ficam em `Assets/`.
- Não é necessário instalar bibliotecas extras: **GLFW**, **GLAD** e **stb** já acompanham o projeto em `external/`.
- O projeto é compilado com **C++17** e requer um compilador com suporte a OpenGL 3.3 (testado com **MinGW/g++** no Windows).
- Se o arquivo de mapa não existir ou estiver malformado, o **mapa embutido** (fallback) é mantido.

## 🙌 Créditos

- Personagem de 8 direções: **AxulArt** (*Small 8 Direction Characters*).
- Tileset de terrenos: **Overworld**.

## 📚 Referências

- [Guia básico de Markdown no GitHub](https://docs.github.com/pt/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax)
- [Documentação do OpenGL](https://www.khronos.org/opengl/)
- [GLFW](https://www.glfw.org/docs/latest/)
- [GLAD](https://glad.dav1d.de/)
- [stb (single-file libraries)](https://github.com/nothings/stb)
- [LearnOpenGL](https://learnopengl.com/)
