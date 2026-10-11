# Leviata

Leviata é uma linguagem de programação em português implementada em C, com lexer, parser, AST, compilador para bytecode, máquina virtual, garbage collector, biblioteca padrão e FFI nativa.

A versão atual, **2.2**, também possui um motor 2D com backend **Raylib**, câmera, colisão, spritesheets, animação, tilemaps e gerenciamento simples de cenas.

## Arquitetura

```text
codigo .pt
   ↓
Lexer
   ↓
Parser / AST
   ↓
Compilador de bytecode
   ↓
VM Leviata
   ↓
Biblioteca padrão + FFI C
   ↓
Motor 2D Leviata
   ↓
Raylib (opcional)
```

## Compilar

### Build padrão

Não exige biblioteca gráfica externa.

```bash
make
./leviata --demo
```

### Build com Raylib

Instale a Raylib e, de preferência, deixe `raylib.pc` disponível para `pkg-config`.

```bash
make raylib
./leviata exemplos/motor2d.pt
```

O Makefile tenta detectar automaticamente as flags da Raylib via `pkg-config`. Se não encontrar, usa `-lraylib` como fallback.

Também existe:

```bash
make raylib-demo
```

## CLI

```bash
./leviata programa.pt
./leviata
./leviata --demo
./leviata --ast programa.pt
./leviata --tokens programa.pt
```

Sem argumentos, o Leviata abre o REPL interativo.

## Exemplo básico

```text
var nome = "Leviata"
var x = 42

se (x > 10) {
    escrever(nome, "esta funcionando")
}

para n de 1 ate 5 {
    escrever(n)
}
```

## Loop de jogo

```text
jogo_iniciar("Meu jogo", 960, 540, 60)

var x = 430
var y = 240

enquanto (nao jogo_deve_fechar()) {
    var dt = jogo_delta()

    se (jogo_tecla(65)) { x = x - 200 * dt }
    se (jogo_tecla(68)) { x = x + 200 * dt }

    jogo_iniciar_frame()
    jogo_limpar_fundo(20, 24, 36)
    jogo_desenhar_retangulo(x, y, 70, 70, 90, 180, 255)
    jogo_desenhar_texto("Leviata + Raylib", 30, 30, 28, 240, 240, 245)
    jogo_finalizar_frame()
}

jogo_fechar()
```

## Motor 2D v2.2

### Câmera

A câmera funciona entre `jogo_camera_iniciar()` e `jogo_camera_finalizar()` dentro de um frame.

```text
jogo_camera_configurar(100, 100, 1, 0, 480, 270)

jogo_iniciar_frame()
jogo_limpar_fundo(20, 20, 30)

jogo_camera_iniciar()
jogo_desenhar_retangulo(100, 100, 50, 50, 80, 180, 255)
jogo_camera_finalizar()

jogo_finalizar_frame()
```

Funções:

- `jogo_camera_configurar(alvo_x, alvo_y, zoom, rotacao, offset_x?, offset_y?)`
- `jogo_camera_seguir(x, y)`
- `jogo_camera_iniciar()`
- `jogo_camera_finalizar()`

### Colisão

```text
var bateu = jogo_colisao_retangulos(
    jogador_x, jogador_y, 48, 48,
    parede_x, parede_y, 90, 260
)
```

Funções:

- `jogo_colisao_retangulos(ax, ay, aw, ah, bx, by, bw, bh)`
- `jogo_ponto_no_retangulo(px, py, rx, ry, rw, rh)`
- `jogo_colisao_circulos(ax, ay, raio_a, bx, by, raio_b)`
- `jogo_colisao_tilemap(x, y, w, h, mapa, colunas, tamanho_tile, origem_x?, origem_y?)`

No `jogo_colisao_tilemap`, valores de tile `>= 0` são tratados como sólidos e valores negativos como vazios.

### Spritesheet

Para desenhar uma região de uma textura:

```text
jogo_desenhar_sprite_regiao(
    "heroi.png",
    0, 0, 32, 32,
    100, 100, 64, 64,
    falso, falso
)
```

Assinatura:

```text
jogo_desenhar_sprite_regiao(caminho,
                            src_x, src_y, src_w, src_h,
                            x, y, largura, altura,
                            inverter_x?, inverter_y?)
```

### Animação

Animações horizontais de spritesheet podem ser desenhadas diretamente:

```text
jogo_desenhar_animacao(
    "heroi.png",
    100, 100,
    32, 32,
    6, 12,
    0, 2
)
```

Parâmetros:

```text
jogo_desenhar_animacao(caminho, x, y,
                       largura_frame, altura_frame,
                       quantidade_frames, fps,
                       linha?, escala?, inverter_x?)
```

A função retorna o índice do frame atual. Também existe:

```text
jogo_animacao_frame(quantidade_frames, fps)
```

### Tilemap

Um mapa é representado por uma lista plana. Exemplo com 4 colunas:

```text
var mapa = [0, 0, 0, 0,
            0, -1, -1, 0,
            0, -1, -1, 0,
            0, 0, 0, 0]

jogo_desenhar_tilemap(
    "tiles.png",
    mapa,
    4,
    16, 16,
    0, 0,
    2
)
```

Assinatura:

```text
jogo_desenhar_tilemap(caminho, mapa, colunas,
                      tile_w, tile_h,
                      origem_x, origem_y,
                      escala?, colunas_tileset?)
```

Tiles negativos não são desenhados.

### Cenas

O gerenciador de cenas mantém a cena atual e a anterior:

```text
cena_mudar("menu")

se (cena_e("menu")) {
    escrever("Estou no menu")
}

cena_mudar("jogo")
escrever(cena_atual())
escrever(cena_anterior())
```

Funções:

- `cena_mudar(nome)`
- `cena_atual()`
- `cena_anterior()`
- `cena_e(nome)`
- `cena_versao()` — incrementa quando a cena muda

## API base de jogos

- `jogo_iniciar(titulo, largura, altura, fps)`
- `jogo_fechar()`
- `jogo_deve_fechar()`
- `jogo_iniciar_frame()`
- `jogo_finalizar_frame()`
- `jogo_limpar_fundo(r, g, b, a?)`
- `jogo_desenhar_sprite(caminho, x, y, largura?, altura?)`
- `jogo_desenhar_retangulo(x, y, largura, altura, r, g, b, a?)`
- `jogo_desenhar_circulo(x, y, raio, r, g, b, a?)`
- `jogo_desenhar_texto(texto, x, y, tamanho, r, g, b, a?)`
- `jogo_tecla(codigo)`
- `jogo_tecla_apertada(codigo)`
- `jogo_tecla_solta(codigo)`
- `jogo_mouse_botao(botao)`
- `jogo_mouse_x()`
- `jogo_mouse_y()`
- `jogo_delta()`
- `jogo_fps()`
- `jogo_audio_iniciar()`
- `jogo_tocar_som(caminho)`
- `jogo_tocar_musica(caminho)`
- `jogo_volume(valor)`

## Teclas mais comuns

```text
A = 65
D = 68
S = 83
W = 87
Espaço = 32
Esc = 256
Enter = 257
Direita = 262
Esquerda = 263
Baixo = 264
Cima = 265
```

## Exemplos

- `exemplos/hello.pt`
- `exemplos/listas.pt`
- `exemplos/matematica.pt`
- `exemplos/jogo.pt`
- `exemplos/motor2d.pt`

## Debug

```bash
make debug
./leviata_debug --demo
```

O build de debug usa AddressSanitizer e UndefinedBehaviorSanitizer.
