# Leviata

Leviata é uma linguagem de programação em português implementada em C, com lexer, parser, AST, compilador para bytecode, máquina virtual, garbage collector, biblioteca padrão e FFI nativa.

A versão atual, **2.3**, inclui um motor 2D com backend **Raylib**, câmera, colisão, spritesheets, animação, tilemaps, cenas, entidades, componentes, física básica e carregamento de mapas por arquivo.

## Arquitetura

```text
codigo .pt
   ↓
Lexer → Parser / AST → Compilador de bytecode
   ↓
VM Leviata + GC
   ↓
Biblioteca padrão + FFI C
   ↓
Motor 2D
   ↓
World: entidades + física + mapas + câmera limitada
   ↓
Raylib (opcional)
```

## Compilar

Build padrão, sem biblioteca gráfica externa:

```bash
make
./leviata --demo
```

Com Raylib:

```bash
make raylib
./leviata exemplos/mundo.pt
```

O Makefile tenta detectar Raylib com `pkg-config` e usa `-lraylib` como fallback.

## CLI

```bash
./leviata programa.pt
./leviata
./leviata --demo
./leviata --ast programa.pt
./leviata --tokens programa.pt
```

## Linguagem

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
    jogo_finalizar_frame()
}

jogo_fechar()
```

## World 2.3

### Entidades

```text
var jogador = entidade_criar("jogador", 100, 80, 26, 30)
entidade_fisica(jogador, 980, 5)
entidade_colisor(jogador, verdadeiro)
entidade_velocidade(jogador, 180, 0)
```

Cada entidade possui transform (`x`, `y`, largura e altura), velocidade e componentes opcionais de física e colisor.

Funções principais:

- `entidade_criar(nome, x, y, largura, altura)` → retorna ID
- `entidade_destruir(id)`
- `entidade_posicionar(id, x, y)`
- `entidade_velocidade(id, vx, vy)`
- `entidade_fisica(id, gravidade, arrasto?)`
- `entidade_colisor(id, ativo)`
- `entidade_atualizar(id, dt)`
- `entidade_atualizar_tilemap(id, dt, mapa, colunas, tile_w, tile_h, origem_x?, origem_y?)`
- `entidade_desenhar(id, r, g, b, a?)`
- `entidade_x(id)` / `entidade_y(id)`
- `entidade_vx(id)` / `entidade_vy(id)`
- `entidade_no_chao(id)`
- `entidade_nome(id)`
- `entidade_quantidade()`
- `entidade_tem_componente(id, "transform" | "fisica" | "colisor")`

`entidade_atualizar_tilemap` aplica gravidade/arrasto, move separadamente nos eixos X e Y e interrompe o movimento quando encontra tiles sólidos. Tiles `>= 0` são sólidos e valores negativos são vazios.

### Mapas externos

O formato é propositalmente simples: uma matriz de números separados por vírgula, ponto e vírgula ou espaço. Linhas iniciadas por `#` ou trechos após `#` são comentários.

```text
# -1 vazio, 0 sólido
-1,-1,-1,-1
-1,-1, 0,-1
 0, 0, 0, 0
```

Uso:

```text
var mapa = mapa_carregar("mapa.csv")
var largura = mapa_largura()
var altura = mapa_altura()
```

O loader valida se todas as linhas têm a mesma largura e devolve uma lista plana compatível com as APIs de tilemap existentes.

### Câmera limitada ao mapa

```text
jogo_camera_configurar(100, 100, 1, 0)
jogo_camera_limites(0, 0, largura * 32, altura * 32)
jogo_camera_seguir(entidade_x(jogador), entidade_y(jogador))
```

Funções:

- `jogo_camera_configurar(alvo_x, alvo_y, zoom, rotacao, offset_x?, offset_y?)`
- `jogo_camera_seguir(x, y)`
- `jogo_camera_limites(min_x, min_y, max_x, max_y)`
- `jogo_camera_limites_limpar()`
- `jogo_camera_iniciar()`
- `jogo_camera_finalizar()`

Com Raylib, os limites consideram o tamanho da viewport e o zoom para evitar que a câmera mostre área fora do mundo quando possível.

## Motor 2D 2.2+

### Colisão

- `jogo_colisao_retangulos(ax, ay, aw, ah, bx, by, bw, bh)`
- `jogo_ponto_no_retangulo(px, py, rx, ry, rw, rh)`
- `jogo_colisao_circulos(ax, ay, raio_a, bx, by, raio_b)`
- `jogo_colisao_tilemap(x, y, w, h, mapa, colunas, tamanho_tile, origem_x?, origem_y?)`

### Spritesheet e animação

```text
jogo_desenhar_sprite_regiao(
    "heroi.png",
    0, 0, 32, 32,
    100, 100, 64, 64,
    falso, falso
)

jogo_desenhar_animacao(
    "heroi.png",
    100, 100,
    32, 32,
    6, 12,
    0, 2
)
```

- `jogo_desenhar_sprite_regiao(...)`
- `jogo_desenhar_animacao(caminho, x, y, frame_w, frame_h, frames, fps, linha?, escala?, inverter_x?)`
- `jogo_animacao_frame(frames, fps)`

### Tilemaps

```text
var mapa = [0, 0, 0, 0,
            0, -1, -1, 0,
            0, 0, 0, 0]

jogo_desenhar_tilemap("tiles.png", mapa, 4, 16, 16, 0, 0, 2)
```

### Cenas

- `cena_mudar(nome)`
- `cena_atual()`
- `cena_anterior()`
- `cena_e(nome)`
- `cena_versao()`

## API base de jogos

- `jogo_iniciar(titulo, largura, altura, fps)`
- `jogo_fechar()`
- `jogo_deve_fechar()`
- `jogo_iniciar_frame()` / `jogo_finalizar_frame()`
- `jogo_limpar_fundo(r, g, b, a?)`
- `jogo_desenhar_sprite(caminho, x, y, largura?, altura?)`
- `jogo_desenhar_retangulo(x, y, largura, altura, r, g, b, a?)`
- `jogo_desenhar_circulo(x, y, raio, r, g, b, a?)`
- `jogo_desenhar_texto(texto, x, y, tamanho, r, g, b, a?)`
- `jogo_tecla(codigo)` / `jogo_tecla_apertada(codigo)` / `jogo_tecla_solta(codigo)`
- `jogo_mouse_botao(botao)` / `jogo_mouse_x()` / `jogo_mouse_y()`
- `jogo_delta()` / `jogo_fps()`
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
- `exemplos/mundo.pt`
- `exemplos/mapas/mundo.csv`

O exemplo `mundo.pt` demonstra mapa carregado do arquivo, entidade, gravidade, colisão, pulo e câmera limitada.

## Debug

```bash
make debug
./leviata_debug --demo
```

O build de debug usa AddressSanitizer e UndefinedBehaviorSanitizer.
