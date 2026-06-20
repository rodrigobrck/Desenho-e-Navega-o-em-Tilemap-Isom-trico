# Mapa Isométrico Staggered — OpenGL + GLAD

## Execução

```powershell
.\build-debug\staggered_map.exe
# ou escolhendo um mapa (caminho relativo a Assets/):
.\build-debug\staggered_map.exe Maps\map_exemplo_3x3.txt
```

## Mapas em arquivos de configuração

Os mapas são lidos de **arquivos de configuração** em `Assets/Maps/`. Por padrão o jogo carrega `Assets/Maps/map.txt`; é possível passar outro arquivo como primeiro argumento na linha de comando (caminho relativo a `Assets/`). Se o arquivo não existir ou estiver malformado, o mapa embutido (fallback no construtor de `Tilemap`) é mantido.

### Formato

```
<tileset> <qtd_tiles> <largura_tile> <altura_tile>
walkable <índices caminháveis...>        (linha opcional)
<linhas> <colunas>
<grade com linhas x colunas índices de tile, separados por espaço/quebra de linha>
```

- **Linha 1 (cabeçalho):** nome do arquivo de tileset (pode conter espaços — os três últimos campos da linha são sempre `qtd_tiles`, `largura` e `altura`), número de tiles (indexados de `0` a `qtd_tiles-1`), e largura/altura de cada célula **em pixels** dentro da imagem.
- **Linha `walkable` (opcional):** começa com a palavra-chave `walkable` (maiúsculas/minúsculas indiferentes) seguida dos **índices de tile que são caminháveis**; todos os demais ficam bloqueados. Se a linha for omitida, **todos os tiles são caminháveis**. Os índices precisam estar em `[0, qtd_tiles-1]`.
- **Linha seguinte:** dimensões do mapa (`linhas colunas`).
- **Demais linhas:** a grade de índices.
- Linhas em branco e linhas iniciadas por `#` são ignoradas (comentários).

A imagem do tileset é **carregada do arquivo nomeado no cabeçalho** (procurado em `Assets/Tiles/`). A grade da imagem é deduzida automaticamente: `colunas = largura_imagem / largura_tile`, `linhas = altura_imagem / altura_tile`. O tile de índice `i` mapeia para a célula `(coluna = i % colunas, linha = i / colunas)`.

O carregamento é **validado** em duas etapas:

- **Formato do mapa** (`MapLoader`): o cabeçalho precisa ter os quatro campos com `qtd_tiles`/`largura`/`altura` inteiros e positivos; cada índice da grade precisa estar em `[0, qtd_tiles-1]`; e a quantidade de índices precisa bater com `linhas × colunas`. Se falhar, uma mensagem é exibida e o **mapa embutido** (que usa `Overworld_7Terrain_Tileset.png`) é mantido.
- **Imagem do tileset** (`Renderer`): a imagem nomeada no cabeçalho precisa existir em `Assets/Tiles/`, ser divisível pelo tamanho de célula informado e comportar ao menos `qtd_tiles` células. Se falhar, uma mensagem de erro é exibida e a aplicação encerra.

Exemplo (`Assets/Maps/map_exemplo_3x3.txt`, mapa 3×3 — equivalente ao enunciado):

```
Overworld - Forest - Flat 256x128.png 7 256 128
walkable 0 1 2 3 5
3 3
1 1 4
4 1 4
4 4 1
```

> Índices de tile (`0..6`) seguem a tabela de terrenos abaixo. A largura/altura no cabeçalho define a escala de desenho (os tiles são reduzidos do tamanho de origem para `64×32` na tela).

## Controles

| Tecla | Ação |
|-------|------|
| **Setas** ou **W A S D** | Norte, Sul, Oeste, Leste |
| **Setas/WASD combinadas** (ex. ↑+→) | Move nas diagonais (NE, NO, SE, SO) |
| **Y** | Alterna **Navegação ↔ Editor** |
| **M** | Alterna **Editor ↔ Puzzle** |
| **TAB** | Alterna (cicla) o tile da célula atual (`0→…→0`) |
| **P** | Inicia o modo **Pega-Pega** |
| **F1** | Liga/desliga **modo debug** (coordenadas `i,j` e tile id em cada célula) |
| **ESC** | Sair |

## Modo Pega-Pega

Pressione **P** para iniciar. Um NPC (o mesmo personagem, com a paleta 4 linhas abaixo na spritesheet) aparece **parado** em um tile aleatório do mapa. O objetivo é alcançar o tile do NPC para capturá-lo.

- O cronômetro começa em **10 segundos** e é exibido no HUD junto com o número de capturas.
- Cada captura **soma 2 segundos** ao cronômetro, e o NPC reaparece em outro tile aleatório.
- Capturar o NPC **10 vezes** vence o jogo; se o cronômetro chegar a **0**, o jogador perde.

### Tiles caminháveis

Quais tiles são caminháveis é definido pela linha `walkable` de cada mapa (ver *Mapas em arquivos de configuração*). Os mapas inclusos usam `walkable 0 1 2 3 5`, ou seja, **terra, grama, pedra, areia e barro** são caminháveis, enquanto **água (4)** e **floresta (6)** ficam bloqueadas. Ao tentar entrar em um tile bloqueado, o movimento é impedido e uma mensagem aparece no console.

## Tileset e personagem

Os assets ficam em `Assets/`:

- `Assets/Tiles/` — imagens de tileset. O tileset padrão é `Overworld_7Terrain_Tileset.png` (1024×256, grade **4×2**, células **256×128**), em que os índices `0..6` ficam dispostos em ordem. Qualquer imagem pode ser usada desde que seja nomeada no cabeçalho do mapa e divisível pelo tamanho de célula informado.
- `Assets/Character/` — personagem 8 direções AxulArt (128×288, sprites **16×24**, grade **8×12**)

O tileset de cada mapa é definido pelo próprio arquivo de configuração (ver seção *Mapas em arquivos de configuração*). O jogo renderiza os tiles em **64×32** na tela (escala 0,25) e usa **chroma key** no shader para descartar fundo preto/magenta dos spritesheets.

Mapeamento dos tiles do mapa (0 a 6):

| Índice | Cor / uso |
|--------|-----------|
| 0 | Terra |
| 1 | Grama |
| 2 | Pedra |
| 3 | Areia |
| 4 | Água |
| 5 | Barro |
| 6 | Floresta |

## Projeção e desenho

- Projeção **ortográfica paralela** com janela de **1280×720** (1 unidade = 1 pixel; ver `include/screen.h`).
- Layout **staggered**: linhas ímpares deslocam meio tile na horizontal.
- Ordem de desenho **back-to-front** (painter's algorithm) por `row + col`.
- **Zoom automático**: se o mapa for maior que a janela, a projeção do mundo é reduzida para enquadrá-lo por completo (o HUD permanece em tamanho fixo).

## Navegação staggered (8 vizinhos)

A vizinhança é calculada por **aritmética de paridade de linha/coluna** (`Tilemap::offsetForDirection`), coerente com o layout staggered: o deslocamento das diagonais depende de a linha atual ser par ou ímpar. As direções suportadas são N, S, E, W, NE, NW, SE e SW.