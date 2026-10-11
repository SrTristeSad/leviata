# Leviata

Leviata é uma linguagem de programação própria, escrita em português brasileiro e implementada em C. Possui lexer, parser, AST, compilador para bytecode, máquina virtual, garbage collector, biblioteca padrão, objetos nativos, classes, módulos e uma camada 2D/World opcional baseada em Raylib.

A versão atual é **Leviata 3.3**.

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
| módulo | `import`, `use` | `usar` |

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

## Classes — 3.2+

```leviata
classe Pessoa {
    var nome = "sem nome"
    var tags = []

    funcao nomear(valor) {
        este.nome = valor
    }

    funcao saudacao() {
        retorne "Oi " + este.nome
    }
}

var ana = novo Pessoa()
ana.nomear("Ana")

escreva(ana.nome)
escreva(ana.saudacao())
escreva(tipo(ana))   /* Pessoa */
```

`este` é o objeto que recebeu a chamada. Campos são independentes por instância. Listas usadas como valor padrão são clonadas para impedir compartilhamento acidental entre objetos.

### Herança

```leviata
classe Animal {
    var nome = "sem nome"

    funcao nomear(valor) {
        este.nome = valor
    }

    funcao falar() {
        retorne este.nome + " fez um som"
    }
}

classe Gato herda Animal {
    var vidas = 7

    funcao falar() {
        retorne este.nome + " disse miau"
    }
}

var gato = novo Gato()
gato.nomear("Baltazar")
escreva(gato.falar())
```

A classe filha herda campos e métodos e pode sobrescrever métodos do pai.

Na versão atual, `novo Classe()` não recebe argumentos. Classes são declaradas no escopo principal.

## Módulos — 3.3

Arquivos Leviata podem carregar outros arquivos com `usar`:

```leviata
usar "modulos/util.pt"

escreva(saudacao_modulo("Leviata"))
```

Um módulo pode exportar naturalmente funções, classes e variáveis globais porque o 3.3 usa **namespace global compartilhado**:

```leviata
/* modulos/util.pt */
funcao dobro(valor) {
    retorne valor * 2
}

classe Caixa {
    var valor = 10
}
```

Depois:

```leviata
usar "modulos/util.pt"

escreva(dobro(5))
var caixa = novo Caixa()
```

Regras do `usar` em 3.3:

- só pode aparecer no escopo principal;
- o caminho é relativo ao arquivo que contém o `usar`;
- imports aninhados também resolvem seus caminhos relativamente ao próprio módulo;
- cada arquivo é executado somente uma vez por processo;
- importar o mesmo arquivo por caminhos equivalentes não o executa novamente;
- ciclos como `A -> B -> A` são detectados e geram erro;
- os módulos são carregados antes da compilação do arquivo importador;
- a extensão deve ser informada explicitamente, por exemplo `"util.pt"`;
- 3.3 ainda não possui `exportar`, alias nem namespace isolado por módulo.

Exemplo oficial:

```bash
./leviata exemplos/modulos.pt
```

A decisão de começar com namespace compartilhado é deliberada: é melhor ter um sistema pequeno e correto do que fingir isolamento renomeando símbolos sem corrigir referências internas. Uma revisão futura pode introduzir exportação/alias de forma completa.

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

Recursos são objetos reais e usam a mesma sintaxe de membro das classes.

### Imagem

```leviata
var foto = imagem("jogador.png")

escreva(foto.caminho)
escreva(foto.largura)
escreva(foto.altura)

foto.desenhe(100, 80)
foto.desenhe(100, 80, 64, 64)
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
escreva(dados.caminho)
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
resolver `usar` recursivamente
   ↓
Lexer -> Parser / AST
   ↓
Compilador de bytecode
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

Módulos importados são executados antes do arquivo importador e compartilham a mesma VM e tabela global.

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
exemplos/modulos.pt
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
./leviata testes/modulos_main.pt
```

Build com sanitizers:

```bash
make debug
./leviata_debug testes/nucleo.pt
./leviata_debug testes/gameplay.pt
./leviata_debug testes/recursos.pt
./leviata_debug testes/classes.pt
./leviata_debug testes/modulos_main.pt
```

O CI executa build, exemplos, regressões do núcleo/gameplay/recursos/classes/módulos, testes negativos de ciclo/escopo e AddressSanitizer/UndefinedBehaviorSanitizer.
