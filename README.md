# Leviata

Leviata é uma linguagem de programação em português implementada em C, com lexer, parser, AST, compilador para bytecode, máquina virtual, garbage collector, biblioteca padrão e FFI nativa.

A versão atual também possui um backend 2D real com **Raylib** para criar jogos usando arquivos `.pt`.

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

Instale a Raylib e, de preferência, deixe o arquivo `raylib.pc` disponível para `pkg-config`.

```bash
make raylib
./leviata exemplos/jogo.pt
```

O Makefile tenta detectar automaticamente as flags da Raylib via `pkg-config`. Se não encontrar, usa `-lraylib` como fallback.

Também existe:

```bash
make raylib-demo
```

que compila e executa diretamente o exemplo gráfico.

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

## Jogo 2D

Com o build Raylib, a linguagem possui um loop gráfico real:

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

O exemplo completo está em `exemplos/jogo.pt`.

## API de jogos

Compatibilidade com a API anterior:

- `jogo_iniciar(titulo, largura, altura, fps)`
- `jogo_fechar()`
- `jogo_deve_fechar()`
- `jogo_iniciar_frame()`
- `jogo_finalizar_frame()`
- `jogo_limpar_fundo(r, g, b, a?)`
- `jogo_desenhar_sprite(caminho, x, y, largura?, altura?)`
- `jogo_tecla(codigo)`
- `jogo_mouse_x()`
- `jogo_mouse_y()`
- `jogo_delta()`

API nova:

- `jogo_desenhar_retangulo(x, y, largura, altura, r, g, b, a?)`
- `jogo_desenhar_circulo(x, y, raio, r, g, b, a?)`
- `jogo_desenhar_texto(texto, x, y, tamanho, r, g, b, a?)`
- `jogo_tecla_apertada(codigo)`
- `jogo_tecla_solta(codigo)`
- `jogo_mouse_botao(botao)`
- `jogo_fps()`
- `jogo_audio_iniciar()`
- `jogo_tocar_som(caminho)`
- `jogo_tocar_musica(caminho)`
- `jogo_volume(valor)` — valor entre `0.0` e `1.0`

As texturas usadas por `jogo_desenhar_sprite` são mantidas em cache durante a execução e liberadas quando `jogo_fechar()` é chamado.

## Teclas mais comuns

Os códigos seguem os valores usados pela Raylib para as teclas principais:

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

## Debug

```bash
make debug
./leviata_debug --demo
```

O build de debug usa AddressSanitizer e UndefinedBehaviorSanitizer.
