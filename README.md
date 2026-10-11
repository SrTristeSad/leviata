# Leviata

Leviata é uma linguagem de programação própria, escrita em português brasileiro e implementada em C. Possui lexer, parser, AST, compilador para bytecode, máquina virtual, garbage collector, biblioteca padrão, FFI nativa e uma camada 2D/World opcional baseada em Raylib.

A versão atual é **Leviata 3.1**.

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

| Conceito | Forma comum | Leviata |
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

As formas antigas como `retornar`, `quebrar`, `continuar`, `importar`, `herdar`, `escrever()` e os operadores `&&`/`||` não pertencem à superfície canônica do Leviata 3.

## Repetições

```leviata
para n de 1 ate 10 {
    escreva(n)
}

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

```leviata
txt(123)
int("42")
dec("3.14")
bool(1)
tipo("Leviata")
```

`tipo()` retorna nomes canônicos como `txt`, `int`, `dec`, `bool`, `lista`, `imagem`, `som`, `arquivo` e `nulo`.

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

Acesso por índice continua direto:

```leviata
var numeros = [10, 20, 30]
escreva(numeros[0])
```

## Matemática

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

## Objetos nativos — 3.1

O ponto agora possui dois significados definidos pelo compilador:

- API estática: `camera.seguir(...)`, `jogo.iniciar(...)`;
- membro de um valor: `foto.desenhe(...)`, `dados.leia()`, `efeito.volume`.

O compilador verifica se a cadeia existe como API global. Se não existir, trata o lado esquerdo como receptor de um campo ou método real.

### Imagem

```leviata
jogo.iniciar("Imagem", 960, 540, 60)

var foto = imagem("jogador.png")

escreva(tipo(foto))       /* imagem */
escreva(foto.caminho)
escreva(foto.largura)
escreva(foto.altura)

foto.desenhe(100, 80)
foto.desenhe(100, 80, 64, 64)
```

`largura` e `altura` são resolvidas de forma preguiçosa pelo backend. No backend gráfico real, a textura precisa poder ser carregada naquele momento.

### Som

```leviata
var tiro = som("tiro.wav")

tiro.volume(0.5)
escreva(tiro.volume)
tiro.toque()
```

O objeto preserva seu volume. O backend de áudio atual ainda utiliza volume mestre no momento da reprodução; a separação por canal/instância é uma evolução futura do backend, não da sintaxe.

### Arquivo

```leviata
var dados = arquivo("dados.txt")

dados.escreva("Olá")
dados.adicione("\nOutra linha")

var conteudo = dados.leia()
escreva(conteudo)
escreva(dados.caminho)
```

Também é válido operar diretamente sobre o retorno do construtor:

```leviata
arquivo("config.txt").escreva("modo=rapido")
```

A antiga forma `arquivo.leia("...")` foi retirada da superfície pública. `arquivo(...)` agora cria um recurso real.

## APIs especializadas

APIs grandes e sem receptor natural continuam agrupadas por domínio.

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

A interface pública não exige códigos numéricos de tecla:

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

### Áudio global

A API global continua útil para música e configuração geral:

```leviata
audio.iniciar()
audio.musica("musica.ogg")
audio.volume(0.8)
```

Para efeitos associados a um recurso, prefira `som("...").toque()`.

## Arquitetura

```text
codigo .pt
   ↓
Lexer
   ↓
Parser / AST
   ↓
Compilador de bytecode
   ├─ API qualificada: camera.seguir
   └─ objeto/método: foto.desenhe
   ↓
VM Leviata + GC
   ↓
Biblioteca canônica + objetos nativos
   ↓
Stdlib / FFI / Motor 2D / World
   ↓
Raylib opcional
```

Os bindings históricos continuam podendo existir internamente em C para reaproveitar a implementação. A superfície `.pt` é definida pelas camadas `leviata_canon` e `leviata_resources`.

## Compilar

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
./leviata testes/recursos.pt
```

Build com sanitizers:

```bash
make debug
./leviata_debug testes/nucleo.pt
./leviata_debug testes/gameplay.pt
./leviata_debug testes/recursos.pt
```

O CI executa build, exemplos, regressões do núcleo/gameplay/recursos e AddressSanitizer/UndefinedBehaviorSanitizer.
