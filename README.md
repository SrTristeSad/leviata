# Leviata

Leviata é uma linguagem de programação própria, escrita em português brasileiro e implementada em C. Possui lexer, parser, AST, compilador para bytecode, máquina virtual, garbage collector, biblioteca padrão, listas, mapas, objetos nativos, classes, construtores, herança, módulos isolados e uma camada 2D/World opcional baseada em Raylib.

A versão atual é **Leviata 3.8**.

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
| repetição condicional | `while` | `enquanto` |
| repetição | `for` | `para` |
| item em coleção | `in` | `em` |
| faixa | `from ... to` | `de ... ate` |
| função | `function`, `def`, `fn` | `funcao` |
| retorno | `return` | `retorne` |
| interromper laço | `break` | `pare` |
| próxima iteração | `continue` | `continue` |
| verdadeiro | `true` | `verdadeiro` |
| falso | `false` | `falso` |
| nulo | `null`, `nil`, `None` | `nulo` |
| E lógico | `and`, `&&` | `e` |
| OU lógico | `or`, `||` | `ou` |
| negação | `not`, `!` | `nao` |
| mapa/dicionário | `map`, `dict`, object literal | `{ chave: valor }` |
| classe | `class` | `classe` |
| nova instância | `new` | `novo` |
| objeto atual | `this`, `self` | `este` |
| herança | `extends`, `inherit` | `herda` |
| classe pai | `super` | `super` |
| módulo | `import`, `use` | `usar` |
| exportação | `export` | `exportar` |
| alias de módulo | `as` | `como` |

As formas antigas como `retornar`, `quebrar`, `continuar`, `importar`, `herdar`, `escrever()` e os operadores `&&`/`||` não pertencem à superfície canônica do Leviata 3.

## Controle de fluxo

```leviata
para n de 1 ate 10 {
    escreva(n)
}

var frutas = ["maçã", "uva", "banana"]
para fruta em frutas {
    escreva(fruta)
}

para n de 1 ate 100 {
    se n == 3 { continue }
    se n == 10 { pare }
    escreva(n)
}
```

`e` e `ou` possuem short-circuit real.

## Funções

```leviata
funcao soma(a, b) {
    retorne a + b
}

escreva(soma(5, 3))
```

## Classes e construtores

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
escreva(ana.saudacao())
escreva(tipo(ana))
```

Regras principais:

- `iniciar(...)` é chamado automaticamente por `novo Classe(...)`;
- a aridade precisa corresponder;
- o retorno de `iniciar` é ignorado;
- `novo` sempre devolve a instância;
- classe sem `iniciar` usa `novo Classe()`;
- listas usadas como valor padrão são clonadas por instância.

## Herança e `super`

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
```

`super.metodo(...)` começa a busca na classe pai da classe onde o método foi declarado. `super.iniciar(...)` chama explicitamente o construtor do pai.

## Módulos — 3.7

### Módulo com API explícita

```leviata
/* biblioteca.pt */
exportar const nome = "Ferramentas"

var fator = 2
var segredo = 123

exportar funcao dobro(valor) {
    retorne valor * fator
}

exportar classe Caixa {
    var valor = 0

    funcao iniciar(valor) {
        este.valor = valor
    }

    funcao dobro() {
        retorne este.valor * fator
    }
}
```

O importador pode dar um alias ao módulo:

```leviata
usar "biblioteca.pt" como util

escreva(util.nome)
escreva(util.dobro(21))

var caixa = novo util.Caixa(42)
escreva(caixa.dobro())
```

No Leviata 3.7+, o módulo é **isolado de verdade no nível da linguagem**:

```leviata
usar "biblioteca.pt" como util

util.dobro(5)  /* válido */
dobro(5)       /* erro: o alias não vaza o símbolo */
segredo        /* erro: símbolo privado do módulo */
```

Uma função ou método exportado continua enxergando os símbolos privados do arquivo onde foi declarado. Dois módulos podem ter variáveis, funções e classes com os mesmos nomes sem colisão.

Exemplo:

```leviata
usar "a.pt" como a
usar "b.pt" como b

escreva(a.valor())
escreva(b.valor())

var caixa_a = novo a.Caixa()
var caixa_b = novo b.Caixa()
```

Também funciona módulo importando outro módulo:

```leviata
/* compositor.pt */
usar "base.pt" como base

exportar funcao mensagem() {
    retorne base.texto() + "!"
}
```

### Import sem alias

A forma simples continua válida:

```leviata
usar "util.pt"
escreva(dobro(5))
```

Nesse caso os símbolos públicos do módulo são ligados diretamente ao escopo do importador.

### Compatibilidade com módulos antigos

Arquivos antigos que não usam `exportar` continuam funcionando: quando importados, suas declarações de topo são consideradas públicas. Internamente, porém, o módulo já recebe símbolos próprios e não depende mais de nomes globais compartilhados.

### Regras de módulos

- `usar` e `exportar` só aparecem no escopo principal;
- caminhos relativos são resolvidos a partir do arquivo importador;
- imports aninhados usam o próprio módulo como base;
- cada caminho normalizado executa uma única vez por runtime;
- caminhos equivalentes compartilham cache;
- ciclos `A -> B -> A` são detectados;
- `exportar` aceita `var`, `const`, `funcao` e `classe`;
- com alias, apenas `alias.simbolo` é publicado no importador;
- símbolos não exportados permanecem privados;
- `novo alias.Classe(...)` funciona para classes exportadas;
- o mesmo módulo pode receber mais de um alias sem ser executado novamente.

## Tipos e conversões

```leviata
txt(123)
int("42")
dec("3.14")
bool(1)
tipo("Leviata")
```

`tipo()` retorna nomes canônicos como `txt`, `int`, `dec`, `bool`, `lista`, `mapa`, `imagem`, `som`, `arquivo`, `classe` e, para uma instância, o nome humano de sua classe.

## Listas

```leviata
var nomes = lista()
adicionar(nomes, "Ana")
adicionar(nomes, "Bia")
inserir(nomes, 1, "Caio")
remover(nomes, "Bia")
limpar(nomes)
```

## Mapas — 3.8

Mapas usam chaves de texto e não exigem uma família nova de funções para operações que já existem em coleções.

```leviata
var pessoa = {
    nome: "Alex",
    idade: 29,
    endereco: {
        cidade: "Salvador",
        uf: "BA"
    }
}

escreva(pessoa.nome)
escreva(pessoa["idade"])
escreva(pessoa.endereco.cidade)
```

Acesso por ponto e por colchetes operam sobre o mesmo dado:

```leviata
pessoa.idade = 30
pessoa["idade"] += 2
pessoa["online"] = verdadeiro
```

As operações gerais reutilizam nomes já conhecidos:

```leviata
escreva(tamanho(pessoa))
escreva(contem(pessoa, "online"))

var nomes = chaves(pessoa)
var dados = valores(pessoa)

remover(pessoa, "online")
limpar(pessoa)
```

Também existe o construtor funcional `mapa()`; por exemplo, `{}` e `mapa()` criam mapas vazios.

Regras:

- chaves são textos;
- `objeto.chave` é a forma curta quando a chave é um identificador simples;
- `objeto["qualquer-chave"]` funciona para nomes que não cabem na forma por ponto;
- atribuição por ponto ou colchetes cria a chave quando necessário;
- `+=`, `-=`, `*=` e `/=` funcionam em valores indexados compatíveis;
- `bool(mapa)` é falso quando vazio e verdadeiro quando possui entradas;
- nomes iniciados por `@` ficam reservados ao runtime e não podem ser chaves públicas;
- a mesma sintaxe `lista[indice]` continua válida; a indexação agora é genérica no compilador.

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

## Objetos nativos

```leviata
var foto = imagem("jogador.png")
escreva(foto.caminho)
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

Outros domínios incluem `camera`, `colisao`, `cena`, `mapa`, `entidade` e `audio`. O namespace `mapa.*` desse motor representa mapas de cenário/tilemap; o tipo de coleção `mapa` é uma estrutura de chave/valor da linguagem.

## Arquitetura

```text
arquivo principal .pt
   ↓
normalização de sintaxe
   ├─ literais de mapa
   ├─ novo Classe(...)
   └─ exportar / alias
   ↓
Lexer / Parser / AST
   ↓
resolver módulos e linker
   ├─ símbolos privados por arquivo
   ├─ exports explícitos
   ├─ alias qualificados
   └─ imports aninhados
   ↓
preparação do compilador
   ├─ indexação genérica de listas/mapas
   ├─ construtores gerados por classe
   └─ super lexical
   ↓
Compilador de bytecode
   ↓
VM Leviata + GC
   ↓
Stdlib / recursos / classes / mapas / FFI / Motor 2D / World
   ↓
Raylib opcional
```

A VM ainda usa uma HashMap global como armazenamento físico, mas módulos são ligados a nomes internos inacessíveis ao código-fonte (`@m...`). Portanto, o compartilhamento físico da tabela não equivale a compartilhamento do namespace da linguagem.

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
exemplos/mapas.pt
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
./leviata testes/mapas.pt
./leviata testes/gameplay.pt
./leviata testes/recursos.pt
./leviata testes/classes.pt
./leviata testes/construtores.pt
./leviata testes/super.pt
./leviata testes/modulos_main.pt
./leviata testes/modulos_alias_main.pt
./leviata testes/modulos_isolamento_main.pt
```

Build com sanitizers:

```bash
make debug
./leviata_debug testes/nucleo.pt
./leviata_debug testes/mapas.pt
./leviata_debug testes/gameplay.pt
./leviata_debug testes/recursos.pt
./leviata_debug testes/classes.pt
./leviata_debug testes/construtores.pt
./leviata_debug testes/super.pt
./leviata_debug testes/modulos_main.pt
./leviata_debug testes/modulos_alias_main.pt
./leviata_debug testes/modulos_isolamento_main.pt
```

O CI executa build, exemplos, regressões do núcleo/mapas/gameplay/recursos/classes/construtores/herança/módulos, testes de isolamento e colisão, casos negativos, AddressSanitizer e UndefinedBehaviorSanitizer.
