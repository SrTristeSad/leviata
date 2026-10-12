# Leviata

Leviata é uma linguagem de programação própria, escrita em português brasileiro e implementada em C. Possui lexer, parser, AST, compilador para bytecode, VM, garbage collector, biblioteca padrão, listas, mapas, objetos nativos, classes, herança, módulos isolados e uma camada 2D/World opcional baseada em Raylib.

A versão atual é **Leviata 3.10**.

Leviata não é Python traduzido, C traduzido nem Portugol. A escrita pública segue uma regra central:

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

| Conceito | Leviata |
|---|---|
| saída | `escreva` |
| entrada | `leia` |
| variável / constante | `var` / `const` |
| condição | `se` / `senao` |
| laço condicional | `enquanto` |
| repetição | `para` |
| função / retorno | `funcao` / `retorne` |
| break / continue | `pare` / `continue` |
| booleanos / nulo | `verdadeiro` / `falso` / `nulo` |
| lógica | `e` / `ou` / `nao` |
| classe / instância | `classe` / `novo` |
| objeto atual | `este` |
| herança / pai | `herda` / `super` |
| módulo | `usar` |
| exportação / alias | `exportar` / `como` |

Formas históricas como `escrever()`, `retornar`, `quebrar`, `continuar`, `importar`, `herdar`, `&&`, `||` e `!` não pertencem à superfície canônica atual.

## Funções e controle de fluxo

```leviata
funcao soma(a, b) {
    retorne a + b
}

para n de 1 ate 10 {
    se n == 3 { continue }
    se n == 8 { pare }
    escreva(n)
}
```

`e` e `ou` possuem short-circuit real.

## Listas

```leviata
var nomes = ["Ana", "Bia"]
adicionar(nomes, "Caio")
inserir(nomes, 1, "Duda")
remover(nomes, "Bia")

para nome em nomes {
    escreva(nome)
}
```

## Mapas

```leviata
var pessoa = {
    nome: "Alex",
    idade: 29,
    endereco: {
        cidade: "Salvador"
    }
}

escreva(pessoa.nome)
escreva(pessoa["idade"])

pessoa.idade = 30
pessoa["idade"] += 1
pessoa["online"] = verdadeiro
```

Operações gerais:

```leviata
tamanho(pessoa)
contem(pessoa, "idade")
chaves(pessoa)
valores(pessoa)
remover(pessoa, "online")
limpar(pessoa)
```

`mapa.nome` e `mapa["nome"]` acessam a mesma entrada.

## Iteração de coleções — 3.10

A mesma escrita serve para listas e mapas.

Lista, apenas valor:

```leviata
para fruta em frutas {
    escreva(fruta)
}
```

Lista, índice e valor:

```leviata
para indice, fruta em frutas {
    escreva(indice, fruta)
}
```

Mapa, chave e valor:

```leviata
para chave, valor em pessoa {
    escreva(chave, valor)
}
```

A coleção é avaliada uma única vez. A forma dupla trabalha sobre um **snapshot** criado no início do laço; alterações posteriores na lista ou no mapa não mudam os pares que já serão percorridos.

`pare` e `continue` funcionam normalmente.

A função `pares()` expõe o mesmo mecanismo diretamente:

```leviata
pares(["a", "b"])       /* [[0, "a"], [1, "b"]] */
pares({ a: 10, b: 20 })  /* [["a", 10], ["b", 20]] em ordem interna */
```

Assim, Leviata não precisa de palavras separadas como `enumerar`, `itens`, `indices` ou `entradas` apenas para percorrer coleções.

## Conversões e tipos

```leviata
txt(123)
int("42")
dec("3.14")
bool(1)
tipo("Leviata")
```

Tipos canônicos incluem `txt`, `int`, `dec`, `bool`, `lista`, `mapa`, `imagem`, `som`, `arquivo`, `classe` e nomes de classes do usuário.

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

Não é necessário escrever `mat.raiz(25)`.

## Classes, construtores e herança

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

`iniciar(...)` é o construtor. `novo Classe(args...)` sempre devolve a instância.

## Módulos isolados

```leviata
/* biblioteca.pt */
var fator = 2

exportar funcao dobro(valor) {
    retorne valor * fator
}
```

```leviata
usar "biblioteca.pt" como util
escreva(util.dobro(21))
```

Com alias, símbolos privados não vazam. Dois módulos podem usar os mesmos nomes internos sem colisão. Cada caminho normalizado executa uma vez e ciclos são detectados.

## Objetos nativos

```leviata
var foto = imagem("jogador.png")
foto.desenhe(100, 80)

var tiro = som("tiro.wav")
tiro.volume(0.5)
tiro.toque()

var dados = arquivo("dados.txt")
dados.escreva("Olá")
escreva(dados.leia())
```

## APIs especializadas

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

Outros domínios incluem `camera`, `colisao`, `cena`, `mapa`, `entidade` e `audio`.

## Arquitetura

```text
fonte .pt
  ↓
lexer
  ↓
parser / AST
  ↓
normalizações de sintaxe
  ├─ construtores
  ├─ super
  ├─ literais de mapa
  └─ para a, b em colecao
  ↓
linker de módulos
  ├─ símbolos privados
  ├─ exports
  └─ aliases
  ↓
compilador de bytecode
  ↓
VM + GC
  ↓
stdlib / FFI / 2D / World
```

A implementação interna pode reutilizar helpers e nomes históricos sem expô-los na linguagem pública.

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

## Exemplos principais

```text
exemplos/hello.pt
exemplos/listas.pt
exemplos/mapas.pt
exemplos/mapas_iteracao.pt
exemplos/colecoes_iteracao.pt
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

O CI valida build, exemplos, núcleo, listas, mapas, iteração de coleções, gameplay, recursos, classes, construtores, `super`, módulos isolados, casos negativos, AddressSanitizer e UndefinedBehaviorSanitizer.

```bash
make
./leviata testes/nucleo.pt
./leviata testes/mapas.pt
./leviata testes/mapas_iteracao.pt
./leviata testes/colecoes_iteracao.pt

make debug
./leviata_debug testes/colecoes_iteracao.pt
```
