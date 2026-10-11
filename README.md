# Leviata

Leviata é uma linguagem de programação própria, escrita em português brasileiro e implementada em C. Possui lexer, parser, AST, compilador para bytecode, máquina virtual, garbage collector, biblioteca padrão, objetos nativos, classes, construtores, herança, módulos e uma camada 2D/World opcional baseada em Raylib.

A versão atual é **Leviata 3.6**.

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
| classe | `class` | `classe` |
| nova instância | `new` | `novo` |
| objeto atual | `this`, `self` | `este` |
| herança | `extends`, `inherit` | `herda` |
| classe pai | `super` | `super` |
| módulo | `import`, `use` | `usar` |
| exportação | `export` | `exportar` |
| alias de módulo | `as` | `como` |

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

## Classes e construtores — 3.4+

O método especial `iniciar(...)` recebe os argumentos usados em `novo Classe(...)`.

```leviata
classe Pessoa {
    var nome = ""
    var idade = 0

    funcao iniciar(nome, idade) {
        este.nome = nome
        este.idade = idade
    }

    funcao saudacao() {
        retorne "Oi " + este.nome
    }
}

var ana = novo Pessoa("Ana", 30)

escreva(ana.nome)
escreva(ana.idade)
escreva(ana.saudacao())
escreva(tipo(ana))
```

`novo` é uma expressão normal:

```leviata
classe Caixa {
    var valor = nulo

    funcao iniciar(valor) {
        este.valor = valor
    }
}

var caixa = novo Caixa(novo Pessoa("Bia", 25))
```

Regras:

- `iniciar(...)` é chamado automaticamente durante `novo Classe(...)`;
- a quantidade de argumentos deve corresponder aos parâmetros de `iniciar`;
- o retorno de `iniciar` é ignorado: `novo` sempre devolve a instância;
- classe sem `iniciar` possui construtor vazio e usa `novo Classe()`;
- campos são independentes por instância;
- listas usadas como valor padrão são clonadas por instância.

## Herança e `super` — 3.5+

Uma classe filha usa `super.metodo(...)` para chamar a implementação herdada começando pela classe pai.

```leviata
classe Pessoa {
    var nome = ""

    funcao iniciar(nome) {
        este.nome = nome
    }

    funcao saudacao() {
        retorne "Oi " + este.nome
    }
}

classe Funcionario herda Pessoa {
    var cargo = ""

    funcao iniciar(nome, cargo) {
        super.iniciar(nome)
        este.cargo = cargo
    }

    funcao saudacao() {
        retorne super.saudacao() + " / " + este.cargo
    }
}

var dev = novo Funcionario("Leo", "dev")
escreva(dev.saudacao())
```

Regras de `super`:

- só pode ser usado dentro de método de uma classe que possui pai;
- `super.metodo(...)` começa a busca na classe pai da classe onde o método foi declarado;
- funciona em cadeias de herança de vários níveis;
- `super.iniciar(...)` é a forma de chamar explicitamente o construtor do pai;
- construtores não são chamados automaticamente em cascata;
- acesso direto como `super.campo` ainda não faz parte da linguagem.

## Módulos básicos — 3.3+

A forma simples continua válida:

```leviata
usar "modulos/util.pt"

escreva(dobro(5))
```

O arquivo importado é executado uma única vez e seus símbolos globais ficam disponíveis para compatibilidade com o modelo 3.x.

Regras de carregamento:

- `usar` fica no escopo principal;
- caminho relativo é resolvido a partir do arquivo importador;
- imports aninhados usam o próprio módulo como base;
- caminhos equivalentes não executam o arquivo novamente;
- ciclos `A -> B -> A` são detectados;
- módulos são carregados antes da compilação do importador;
- a extensão é explícita, por exemplo `"util.pt"`.

## Exportação e alias de módulo — 3.6

Um módulo pode declarar explicitamente sua API pública:

```leviata
/* biblioteca.pt */
exportar const nome_biblioteca = "Ferramentas"

exportar funcao dobro(valor) {
    retorne valor * 2
}

exportar classe Caixa {
    var valor = 0

    funcao iniciar(valor) {
        este.valor = valor
    }
}

var segredo_interno = 123
```

O importador pode dar um nome ao módulo:

```leviata
usar "biblioteca.pt" como util

escreva(util.nome_biblioteca)
escreva(util.dobro(21))

var caixa = novo util.Caixa(42)
```

`util.segredo_interno` não existe porque o símbolo não foi marcado com `exportar`.

A versão 3.6 aceita:

```text
exportar var nome = ...
exportar const nome = ...
exportar funcao nome(...) { ... }
exportar classe Nome { ... }
```

Também é possível importar o mesmo módulo com mais de um alias; o arquivo continua sendo executado apenas uma vez.

### Importante sobre isolamento

`usar ... como ...` cria uma **visão qualificada dos símbolos exportados**. Ele ainda não cria uma VM ou tabela global isolada por arquivo. Para compatibilidade com o bytecode e com o Leviata 3.x, os símbolos originais do módulo continuam existindo no namespace global compartilhado.

Portanto, no 3.6:

```leviata
usar "biblioteca.pt" como util

util.dobro(5)  /* API explícita */
dobro(5)       /* ainda existe por compatibilidade 3.x */
```

A diferença é que somente símbolos com `exportar` aparecem em `util.*`.

`exportar` e `usar` só podem aparecer no escopo principal.

## Tipos e conversões

```leviata
txt(123)
int("42")
dec("3.14")
bool(1)
tipo("Leviata")
```

`tipo()` retorna nomes canônicos como `txt`, `int`, `dec`, `bool`, `lista`, `imagem`, `som`, `arquivo`, `classe` e, para uma instância, o nome de sua classe.

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

## Objetos nativos — 3.1+

### Imagem

```leviata
var foto = imagem("jogador.png")
escreva(foto.caminho)
escreva(foto.largura)
escreva(foto.altura)
foto.desenhe(100, 80)
```

### Som

```leviata
var tiro = som("tiro.wav")
tiro.volume(0.5)
escreva(tiro.volume)
tiro.toque()
```

### Arquivo

```leviata
var dados = arquivo("dados.txt")
dados.escreva("Olá")
dados.adicione("\nOutra linha")
escreva(dados.leia())
```

Também é válido:

```leviata
arquivo("config.txt").escreva("modo=rapido")
```

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
```

### Áudio global

```leviata
audio.iniciar()
audio.musica("musica.ogg")
audio.volume(0.8)
```

## Arquitetura

```text
arquivo principal .pt
   ↓
resolver `usar` / `exportar` / alias
   ↓
front-end Leviata
   ├─ normalização de `novo Classe(args...)`
   ├─ Lexer
   └─ Parser / AST
   ↓
Compilador de bytecode
   ├─ construtores gerados por classe
   ├─ `super` lexical
   ├─ APIs qualificadas: camera.seguir
   ├─ recursos: foto.desenhe
   └─ classes: objeto.metodo / objeto.campo
   ↓
VM Leviata + GC
   ↓
Biblioteca canônica + recursos + classes
   ↓
Stdlib / FFI / Motor 2D / World
   ↓
Raylib opcional
```

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

## Exemplos

```text
exemplos/hello.pt
exemplos/listas.pt
exemplos/matematica.pt
exemplos/classes.pt
exemplos/construtores.pt
exemplos/modulos.pt
exemplos/modulos_alias.pt
exemplos/jogo.pt
exemplos/motor2d.pt
exemplos/mundo.pt
exemplos/gameplay.pt
```

## Testes

```bash
./leviata testes/nucleo.pt
./leviata testes/gameplay.pt
./leviata testes/recursos.pt
./leviata testes/classes.pt
./leviata testes/construtores.pt
./leviata testes/super.pt
./leviata testes/modulos_main.pt
./leviata testes/modulos_alias_main.pt
```

Build com sanitizers:

```bash
make debug
./leviata_debug testes/nucleo.pt
./leviata_debug testes/gameplay.pt
./leviata_debug testes/recursos.pt
./leviata_debug testes/classes.pt
./leviata_debug testes/construtores.pt
./leviata_debug testes/super.pt
./leviata_debug testes/modulos_main.pt
./leviata_debug testes/modulos_alias_main.pt
```

O CI executa build, exemplos, regressões do núcleo/gameplay/recursos/classes/construtores/herança/módulos, testes negativos, AddressSanitizer e UndefinedBehaviorSanitizer.
