# Mapa Isométrico Staggered — OpenGL + GLAD

## Execução

```powershell
.\build-debug\staggered_map.exe
```

## Controles

| Tecla | Ação |
|-------|------|
| **Setas** ou **W A S D** | Norte, Oeste, Sul, Leste |
| **AS, AW, SD, SW** Move nas diagonais
| **TAB** | Alterna o tile da célula visitada (0→6→0) — modo **Editor** |
| **F1** | Liga/desliga **modo debug** (coordenadas `i,j` e tile id em cada célula) |
| **ESC** | Sair |

### Terrenos caminháveis
| ID | Terreno |
|----|---------|
| 0 | Terra |
| 1 | Grama |
| 2 | Pedra (trilha) |
| 3 | Areia |
| 5 | Barro |

### Terrenos bloqueados
| ID | Terreno |
|----|---------|
| 4 | Água |
| 6 | Floresta |

Ao tentar entrar em água ou floresta, o movimento é bloqueado e uma mensagem aparece no console.

## Tileset e personagem

Os assets ficam em `Assets/`:

- `Assets/Tiles/` — spritesheets Overworld (768×768, grade **3×6**, cada célula **256×128**, proporção 2:1)
- `Assets/Character/` — personagem 8 direções AxulArt (128×288, sprites **16×24**, grade **8×12**)

O jogo renderiza os tiles em **64×32** na tela (escala 0,25) e usa **chroma key** no shader para descartar fundo preto/magenta dos spritesheets.

Mapeamento dos tiles do mapa (0 a 6):

| Índice | Cor / uso |
|--------|-----------|
| 0 | Terra |
| 1 | Grama |
| 2 | Pedra |
| 3 | Areia |
| 4 | Água |
| 5 | Barro |
| 6 | Rosa — marcador da célula visitada |

## Projeção e desenho

- Projeção **ortográfica paralela** com janela do mundo **800×600** (1 unidade = 1 pixel).
- Layout **staggered**: linhas pares deslocam meio tile na horizontal.
- Ordem de desenho **back-to-front** (painter's algorithm) por `row + col`.

## Navegação staggered (8 vizinhos)

A vizinhança é calculada a partir das **posições na tela** de cada tile (coerente com o desenho staggered). Para cada direção, o jogo busca o tile cujo centro está alinhado visualmente com N, S, E, W, NE, NW, SE ou SW.