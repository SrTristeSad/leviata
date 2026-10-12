# Leviata — Especificação Canônica e Registro Histórico

> Documento normativo da escrita pública do Leviata. Define como a linguagem deve ser escrita e entendida; não é uma apostila.

**Criado:** 11 de outubro de 2026  
**Última revisão:** 11 de outubro de 2026  
**Versão implementada:** Leviata 3.10

---

# 1. Identidade

Leviata é uma linguagem própria em português brasileiro. Não é Python traduzido, C traduzido, JavaScript traduzido nem Portugol ampliado.

Regra central:

> **Uma ideia, uma forma oficial.**

A linguagem deve ser curta sem ser criptográfica, previsível, consistente e capaz de crescer sem nomes gigantes. Palavras reservadas e APIs canônicas evitam acentos. Siglas são aceitas quando são reconhecíveis e reduzem ruído.

---

# 2. Vocabulário canônico

| Conceito | Paralelo comum | Leviata |
|---|---|---|
| saída | `print` | `escreva` |
| entrada | `input`, `read` | `leia` |
| variável | `var`, `let` | `var` |
| constante | `const` | `const` |
| condição | `if` | `se` |
| alternativa | `else` | `senao` |
| while | `while` | `enquanto` |
| for | `for` | `para` |
| in | `in` | `em` |
| faixa | `from ... to` | `de ... ate` |
| função | `function`, `def`, `fn` | `funcao` |
| retorno | `return` | `retorne` |
| break | `break` | `pare` |
| continue | `continue` | `continue` |
| true / false / null | `true / false / null` | `verdadeiro / falso / nulo` |
| lógica | `and / or / not` | `e / ou / nao` |
| classe | `class` | `classe` |
| nova instância | `new` | `novo` |
| objeto atual | `this`, `self` | `este` |
| herança | `extends`, `inherit` | `herda` |
| classe pai | `super` | `super` |
| módulo | `import`, `use` | `usar` |
| exportação | `export` | `exportar` |
| alias | `as` | `como` |

Formas históricas removidas:

```text
escrever  -> escreva
retornar  -> retorne
quebrar   -> pare
continuar -> continue
importar  -> usar
herdar    -> herda
```

`&&`, `||` e `!` não pertencem à lógica canônica. `!=` continua válido.

---

# 3. Escrita básica

Símbolos universais permanecem símbolos:

```text
+  -  *  /  %
=  ==  !=
<  >  <=  >=
```

Blocos usam `{}`. Ponto e vírgula não é obrigatório. Não existem palavras obrigatórias como `entao`, `faca`, `fimse` ou `fimenquanto`.

```leviata
se idade >= 18 {
    escreva("Adulto")
}
```

---

# 4. Tipos e conversões

Formas canônicas:

```text
str / String   -> txt
int            -> int
float / double -> dec
bool / Boolean -> bool
```

```leviata
var a = txt(123)
var b = int("42")
var c = dec("3.14")
var d = bool(1)
```

`tipo(valor)` usa nomes canônicos da linguagem.

---

# 5. Controle de fluxo

```leviata
se idade >= 18 e possui_documento {
    escreva("Entrada permitida")
} senao {
    escreva("Entrada negada")
}

para n de 1 ate 10 {
    escreva(n)
}
```

`ate` é inclusivo. `e` e `ou` possuem short-circuit. `pare` e `continue` fazem unwind correto das variáveis locais abandonadas.

---

# 6. Funções

```leviata
funcao soma(a, b) {
    retorne a + b
}
```

Função sem `retorne` explícito devolve `nulo`.

---

# 7. Coleções

## 7.1 Listas

```leviata
var nomes = ["Ana", "Bia"]
adicionar(nomes, "Caio")
inserir(nomes, 1, "Duda")
remover(nomes, "Bia")
```

Operações genéricas devem ser reutilizadas quando o primeiro argumento já deixa o tipo claro. Evitar APIs como `lista_adicionar`.

## 7.2 Mapas

```leviata
var pessoa = {
    nome: "Alex",
    idade: 29
}
```

Acesso equivalente:

```leviata
pessoa.nome
pessoa["nome"]
```

Operações:

```text
mapa(...)
chaves(mapa)
valores(mapa)
tamanho(mapa)
contem(mapa, chave)
remover(mapa, chave)
limpar(mapa)
```

Chaves públicas são textos. Chaves iniciadas por `@` são reservadas ao runtime.

`mapa` como estrutura de dados não elimina o namespace especializado `mapa.carregar(...)` do motor de tilemap; o contexto distingue os dois conceitos.

---

# 8. Iteração de coleções — 3.10

A forma simples continua sendo a forma principal quando só o valor interessa:

```leviata
para item em lista {
    escreva(item)
}
```

A forma dupla é única para coleções que fornecem pares:

```leviata
para indice, item em lista {
    escreva(indice, item)
}

para chave, valor em mapa {
    escreva(chave, valor)
}
```

Semântica oficial:

1. a expressão da coleção é avaliada uma única vez;
2. lista produz pares `[indice, valor]`;
3. mapa produz pares `[chave, valor]`;
4. a forma dupla usa snapshot criado no início do laço;
5. mutações posteriores na coleção original não alteram os pares já selecionados;
6. `pare` e `continue` mantêm a mesma semântica dos demais laços;
7. `pares(colecao)` expõe diretamente a lista de pares usada pela forma dupla.

Exemplos:

```leviata
pares(["a", "b"])
/* [[0, "a"], [1, "b"]] */

pares({ a: 10, b: 20 })
/* [["a", 10], ["b", 20]] em ordem interna */
```

Decisão de design: **não criar palavras separadas como `enumerar`, `indices`, `entradas` ou `itens_do_mapa` quando a mesma forma `para a, b em colecao` já expressa o conceito.**

Internamente, a forma dupla é normalizada para foreach sobre `pares(...)`, reaproveitando o bytecode estável de iteração.

---

# 9. Texto e matemática

Texto:

```text
maiusculas
minusculas
contem
comeca
termina
trecho
tamanho
```

Matemática fica global quando o nome já identifica a operação:

```text
raiz
potencia
abs
arredondar
teto
piso
seno
cos
tan
log
max
min
aleatorio
aleatorio_int
pi
```

Preferir `raiz(25)`, não `mat.raiz(25)`.

---

# 10. Função, objeto ou namespace

Regra oficial:

> **Função autoexplicativa fica global. Recurso com estado vira objeto. Namespace só aparece quando realmente evita ambiguidade ou organiza uma API ampla.**

```leviata
raiz(25)

var foto = imagem("player.png")
foto.desenhe(10, 20)

jogo.iniciar("Meu jogo", 960, 540, 60)
camera.seguir(x, y)
```

Evitar APIs públicas como `jogo_desenhar_retangulo` e `jogo_camera_seguir`.

---

# 11. Objetos nativos

```leviata
var foto = imagem("jogador.png")
foto.desenhe(10, 20)

var efeito = som("efeito.wav")
efeito.volume(0.5)
efeito.toque()

var dados = arquivo("dados.txt")
dados.escreva("Ola")
escreva(dados.leia())
```

---

# 12. Classes, construtores e herança

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
}
```

Decisões:

- `este` é o receptor implícito;
- campos pertencem à instância;
- `iniciar(...)` é chamado automaticamente por `novo Classe(args...)`;
- o retorno de `iniciar` é ignorado;
- `novo` sempre devolve a instância;
- `super.metodo(...)` começa a busca na classe pai lexical;
- construtores pais não são chamados automaticamente em cascata.

---

# 13. Módulos

```leviata
exportar funcao dobro(x) {
    retorne x * 2
}
```

```leviata
usar "util.pt" como util
escreva(util.dobro(5))
```

Regras atuais:

- `usar` e `exportar` aparecem no escopo principal;
- caminho relativo é resolvido a partir do importador;
- cada caminho normalizado executa uma vez;
- ciclos são detectados;
- `exportar` aceita `var`, `const`, `funcao` e `classe`;
- com alias, somente `alias.simbolo` fica visível;
- privados não vazam;
- funções e métodos exportados continuam enxergando privados do próprio módulo;
- dois módulos podem usar os mesmos nomes sem colisão;
- `novo alias.Classe(...)` funciona;
- módulos antigos sem `exportar` tratam declarações de topo como públicas por compatibilidade.

O isolamento usa linkedição para símbolos internos `@m...`, não uma VM separada para cada arquivo.

---

# 14. O ponto (`.`)

Usos coerentes:

```leviata
camera.seguir(x, y)      /* API estática */
foto.desenhe(x, y)       /* método */
pessoa.nome              /* campo */
util.dobro(5)            /* export de módulo */
novo util.Caixa(10)      /* classe exportada */
```

---

# 15. Modelo interno

Detalhes internos não são sintaxe pública. Podem existir nomes como:

```text
@classe.criar
@obj.metodo
@indice.obter
@m2:4
__leviata_par_0
jogo_desenhar_retangulo
```

Eles podem mudar sem alterar código Leviata válido.

---

# 16. Política de mudanças futuras

Toda mudança pública deve responder:

1. Qual conceito representa?
2. Qual é o paralelo comum?
3. Qual será a única forma canônica?
4. Por que essa forma é melhor para o Leviata?
5. A implementação já suporta a forma?
6. Qual é o impacto de compatibilidade?

Não aceitar uma decisão apenas porque outra linguagem faz assim.

---

# 17. Registro histórico

## 10 de outubro de 2026 — vocabulário inicial

Foram definidos `escreva`, `leia`, `se`, `senao`, `enquanto`, `para`, `funcao`, `retorne`, `pare`, conversões curtas e matemática sem namespace redundante.

## 11 de outubro de 2026 — Leviata 3.0

A escrita canônica passou ao lexer, parser, compilador, biblioteca, exemplos e testes. Entraram `para ... em ...`, lógica `e/ou/nao`, APIs qualificadas e teclas por nome.

## Leviata 3.1

`imagem`, `som` e `arquivo` passaram a ser objetos com campos e métodos.

## Leviata 3.2

Entraram `classe`, `novo`, `este`, `herda`, campos, métodos e override.

## Leviata 3.3

`usar` passou a carregar módulos reais com caminho relativo, cache, import aninhado e detecção de ciclo.

## Leviata 3.4

`iniciar(...)` tornou-se construtor com argumentos usados por `novo Classe(...)`.

## Leviata 3.5

Entrou `super.metodo(...)` com semântica lexical de classe pai.

## Leviata 3.6

Entraram `exportar` e `usar ... como ...`.

## Leviata 3.7

Módulos ganharam isolamento real de nomes por linkedição de símbolos internos.

## Leviata 3.8

Mapas viraram estrutura pública com literal `{ chave: valor }`, acesso por ponto e índice e operações genéricas.

## Leviata 3.9

Mapas ganharam iteração direta de chave/valor e `pares(mapa)`, com snapshot e avaliação única da coleção.

## Leviata 3.10

A mesma forma dupla foi generalizada para listas e mapas:

```leviata
para indice, valor em lista { ... }
para chave, valor em mapa { ... }
```

`pares(lista)` devolve `[indice, valor]` e `pares(mapa)` devolve `[chave, valor]`. A decisão evitou acrescentar palavras novas para um conceito que já cabia na sintaxe existente.

---

# 18. Estado canônico atual

A referência implementada é **Leviata 3.10**.

Recursos públicos consolidados:

```text
escreva / leia
se / senao / enquanto / para
para item em lista
para indice, item em lista
para chave, valor em mapa
funcao / retorne
pare / continue
verdadeiro / falso / nulo
e / ou / nao
lista / mapa / pares
classe / novo / este / herda / super
iniciar como construtor
usar / exportar / como
txt / int / dec / bool
objetos de recurso
APIs qualificadas
módulos isolados
cache de módulos
detecção de ciclos
```

Qualquer mudança futura deve atualizar este documento para ser considerada parte da escrita canônica do Leviata.
