# Leviata

Leviata é uma linguagem de programação própria, escrita em português brasileiro e implementada em C. Possui lexer, parser, AST, compilador para bytecode, máquina virtual, garbage collector, biblioteca padrão, FFI nativa e uma camada 2D/World opcional baseada em Raylib.

A versão atual é **Leviata 3.0**.

O Leviata não é Python traduzido, C traduzido nem Portugol. Ele usa conceitos universais de programação, mas possui vocabulário e sintaxe canônicos próprios.

> **Uma ideia, uma forma oficial.**

## Exemplo rápido

```leviata
var nome = leia("Nome: ")
var idade = int(leia("Idade: "))

se idade >= 18 {
    escreva(nome, "é maior de idade")
} senao {
    escreva(nome, "é menor de idade")
}
```

## Vocabulário principal

| Conceito | Forma comum em inglês | Leviata |
|---|---|---|
| saída | `print` | `escreva` |
| entrada | `input`, `read` | `leia` |
| variável | `var`, `let` | `var` |
| constante | `const` | `const` |
| condição | `if` | `se` |
| alternativa | `else` | `senao` |
| repetição | `while` | `enquanto` |
| repetição | `for` | `para` |
| retorno | `return` | `retorne` |
| interromper laço | `break` | `pare` |
| próxima iteração | `continue` | `continue` |
| verdadeiro | `true` | `verdadeiro` |
| falso | `false` | `falso` |
| nulo | `null`, `nil`, `None` | `nulo` |
| E lógico | `and`, `&&` | `e` |
| OU lógico | `or`, `||` | `ou` |
| negação | `not`, `!` | `nao` |
| função | `function`, `def`, `fn` | `funcao` |

As palavras antigas como `retornar`, `quebrar`, `continuar`, `importar` e `herdar` não são a sintaxe canônica do Leviata 3.

## Repetições

Faixa numérica:

```leviata
para n de 1 ate 10 {
    escreva(n)
}
```

Coleção:

```leviata
var frutas = ["maçã", "uva", "banana"]

para fruta em frutas {
    escreva(fruta)
}
```

Controle do laço:

```leviata
para n de 1 ate 100 {
    se n == 3 {
        continue
    }

    se n == 10 {
        pare
    }

    escreva(n)
}
```

## Funções

```leviata
funcao soma(a, b) {
    retorne a + b
}

escreva(soma(5, 3))
```

## Tipos e conversões

As conversões frequentes usam nomes curtos:

```leviata
txt(123)
int("42")
dec("3.14")
bool(1)
tipo("Leviata")
```

`tipo()` retorna nomes canônicos como `txt`, `int`, `dec`, `bool`, `lista` e `nulo`.

## Listas

```leviata
var nomes = lista()

adicionar(nomes, "Ana")
adicionar(nomes, "Bia")
inserir(nomes, 1, "Caio")

escreva(tamanho(nomes))
escreva(contem(nomes, "Ana"))

remover(nomes, "Bia")
limpar(nomes)
```

Listas literais continuam simples:

```leviata
var numeros = [10, 20, 30]
escreva(numeros[0])
```

## Matemática

Operações autoexplicativas ficam no escopo global. Não existe a obrigação de escrever `mat.` antes delas.

```leviata
raiz(25)
potencia(2, 8)
abs(-10)
arredondar(3.7)
teto(3.2)
piso(3.8)
seno(0)
cos(0)
tan(0)
log(10)
max(5, 9)
min(5, 9)
aleatorio()
aleatorio_int(1, 100)
pi
```

## APIs especializadas

APIs maiores usam nomes curtos por domínio em vez de funções globais gigantes.

```leviata
jogo.iniciar("Meu jogo", 960, 540, 60)

var x = 100

 enquanto nao jogo.deve_fechar() {
    var dt = jogo.delta()

    se teclado.pressionada("A") {
        x = x - 200 * dt
    }

    jogo.quadro()
    desenho.fundo(20, 24, 36)
    desenho.retangulo(x, 100, 64, 64, 90, 180, 255)
    jogo.mostrar()
}

jogo.fechar()
```

A interface pública não exige decorar códigos como `65` para A ou `32` para espaço:

```leviata
teclado.pressionada("A")
teclado.apertada("espaco")
teclado.pressionada("esquerda")
```

### Câmera

```leviata
camera.configurar(100, 100, 1, 0)
camera.seguir(x, y)
camera.suavidade(7)
camera.limites(0, 0, 1600, 900)
```

### Desenho e colisão

```leviata
desenho.fundo(20, 24, 36)
desenho.retangulo(x, y, 64, 64, 90, 180, 255)
desenho.circulo(x, y, 10, 255, 255, 255)
desenho.texto("Olá", 20, 20, 24, 255, 255, 255)

colisao.retangulos(ax, ay, aw, ah, bx, by, bw, bh)
colisao.circulos(ax, ay, raio_a, bx, by, raio_b)
```

### Cenas

```leviata
cena.mudar("jogo")
escreva(cena.atual())
escreva(cena.anterior())
```

### Mapas

```leviata
var mundo = mapa.carregar("mapa.csv")
var largura = mapa.largura()
var altura = mapa.altura()
```

### Entidades

```leviata
var jogador = entidade.criar("jogador", 100, 80, 32, 48)

entidade.fisica(jogador, 980, 5)
entidade.colisor(jogador, verdadeiro)
entidade.velocidade(jogador, 180, 0)
entidade.atualizar(jogador, jogo.delta())

escreva(entidade.x(jogador), entidade.y(jogador))
```

### Áudio

```leviata
audio.iniciar()
audio.tocar("efeito.wav")
audio.musica("musica.ogg")
audio.volume(0.8)
```

### Arquivos

Entrada e saída de console usam `leia` e `escreva`. Arquivos usam um domínio explícito para não criar ambiguidade:

```leviata
var conteudo = arquivo.leia("dados.txt")
arquivo.escreva("dados.txt", "Olá")
arquivo.adicione("dados.txt", "\nOutra linha")
```

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
VM Leviata + GC
   ↓
Biblioteca canônica
   ↓
Stdlib / FFI / Motor 2D / World
   ↓
Raylib opcional
```

Os bindings antigos podem continuar existindo internamente em C para reutilização da implementação. A camada `leviata_canon` define a superfície pública usada pelo código `.pt`.

## Compilar

Build padrão sem dependência gráfica externa:

```bash
make
./leviata --demo
```

Com Raylib:

```bash
make raylib
./leviata exemplos/mundo.pt
```

## CLI

```bash
./leviata
./leviata programa.pt
./leviata --demo
./leviata --ast programa.pt
./leviata --tokens programa.pt
```

## Testes

```bash
./leviata testes/nucleo.pt
./leviata testes/gameplay.pt
```

Build com sanitizers:

```bash
make debug
./leviata_debug testes/nucleo.pt
./leviata_debug testes/gameplay.pt
```

O CI executa o build, exemplos, testes de regressão e AddressSanitizer/UndefinedBehaviorSanitizer.

## Exemplos

- `exemplos/hello.pt`
- `exemplos/listas.pt`
- `exemplos/matematica.pt`
- `exemplos/jogo.pt`
- `exemplos/motor2d.pt`
- `exemplos/mundo.pt`
- `exemplos/gameplay.pt`

## Especificação histórica

As decisões de nomenclatura e sintaxe são registradas em:

`docs/DECISOES_DA_LINGUAGEM.md`

Mudanças futuras devem manter o princípio de uma forma canônica por conceito e registrar o motivo da alteração.
