# Leviata — Especificação Canônica e Registro Histórico

> Documento normativo da escrita pública do Leviata. Define como a linguagem deve ser escrita e entendida; não é uma apostila.

**Criado:** 11 de outubro de 2026  
**Última revisão:** 11 de outubro de 2026  
**Versão implementada:** Leviata 3.9

---

# 1. Identidade

Leviata é uma linguagem própria em português brasileiro. Não é Python traduzido, C traduzido, JavaScript traduzido nem Portugol ampliado.

Conceitos universais são usados quando ajudam, mas a escrita pública segue uma regra central:

> **Uma ideia, uma forma oficial.**

A linguagem deve ser curta sem ser criptográfica, previsível, consistente e capaz de crescer sem nomes gigantes.

Palavras reservadas e APIs canônicas evitam acentos para facilitar teclado, tooling e portabilidade. Siglas são aceitas quando são reconhecíveis e realmente reduzem ruído.

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
| true | `true` | `verdadeiro` |
| false | `false` | `falso` |
| null | `null`, `nil`, `None` | `nulo` |
| AND | `and`, `&&` | `e` |
| OR | `or`, `||` | `ou` |
| NOT | `not`, `!` | `nao` |
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

`&&`, `||` e `!` não fazem parte da lógica canônica. `!=` continua sendo operador de diferença.

---

# 3. Símbolos e blocos

Símbolos universais permanecem símbolos:

```text
+  -  *  /  %
=  ==  !=
<  >  <=  >=
```

Blocos usam chaves:

```leviata
se idade >= 18 {
    escreva("Adulto")
}
```

Não existem `entao`, `faca`, `fimse`, `fimenquanto` ou equivalentes obrigatórios.

Ponto e vírgula não é obrigatório.

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

`tipo(valor)` usa os nomes canônicos da linguagem.

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

para fruta em frutas {
    escreva(fruta)
}
```

`ate` é inclusivo. `e` e `ou` possuem short-circuit real. `pare` e `continue` fazem unwind correto de variáveis locais abandonadas.

---

# 6. Funções

```leviata
funcao soma(a, b) {
    retorne a + b
}
```

Função sem `retorne` explícito devolve `nulo`.

---

# 7. Listas

```leviata
var nomes = ["Ana", "Bia"]
adicionar(nomes, "Caio")
inserir(nomes, 1, "Duda")
remover(nomes, "Bia")
```

Operações genéricas devem ser reutilizadas quando o primeiro argumento já deixa o tipo claro. Evitar nomes como `lista_adicionar` na superfície pública.

---

# 8. Mapas — 3.8

Mapa é a estrutura chave/valor da linguagem:

```leviata
var pessoa = {
    nome: "Alex",
    idade: 29,
    endereco: {
        cidade: "Salvador"
    }
}
```

Acesso equivalente:

```leviata
pessoa.nome
pessoa["nome"]
```

Atribuição:

```leviata
pessoa.idade = 30
pessoa["idade"] += 1
pessoa["online"] = verdadeiro
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

`mapa` como estrutura de dados não elimina o namespace especializado `mapa.carregar(...)` do motor de tilemap: o contexto distingue os dois conceitos.

---

# 9. Iteração chave/valor — 3.9

Forma canônica para percorrer mapas:

```leviata
para chave, valor em dados {
    escreva(chave, valor)
}
```

Decisões:

1. listas continuam usando uma variável:

```leviata
para item em lista {
    escreva(item)
}
```

2. mapas usam duas variáveis quando o programador quer chave e valor;
3. a expressão da coleção é avaliada uma única vez;
4. a iteração usa um snapshot dos pares existentes no início do laço;
5. mutações posteriores do mapa não alteram o conjunto de pares já selecionado;
6. `pare` e `continue` mantêm a mesma semântica dos outros laços;
7. `pares(mapa)` expõe explicitamente o snapshot como lista de pares `[chave, valor]` quando isso for útil fora do `para`.

Internamente, a forma dupla é normalizada para um foreach comum sobre `pares(...)`. Isso reaproveita o bytecode já estável de iteração e evita criar uma segunda implementação de laço.

---

# 10. Texto

Funções canônicas:

```text
maiusculas
minusculas
contem
comeca
termina
trecho
tamanho
```

---

# 11. Matemática

Matemática não recebe namespace quando o nome já identifica a operação:

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

Preferir:

```leviata
raiz(25)
```

não:

```text
mat.raiz(25)
```

---

# 12. Função, objeto ou namespace

Regra oficial:

> **Função autoexplicativa fica global. Recurso com estado vira objeto. Namespace só aparece quando realmente evita ambiguidade ou organiza uma API ampla.**

Exemplos:

```leviata
raiz(25)

var foto = imagem("player.png")
foto.desenhe(10, 20)

jogo.iniciar("Meu jogo", 960, 540, 60)
camera.seguir(x, y)
```

Isso substitui APIs históricas como `jogo_desenhar_retangulo` e `jogo_camera_seguir`.

---

# 13. Objetos nativos — 3.1

Recursos com estado são objetos:

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

# 14. Classes — 3.2

```leviata
classe Pessoa {
    var nome = "sem nome"

    funcao nomear(valor) {
        este.nome = valor
    }
}
```

`este` é o receptor implícito. Campos pertencem à instância. Listas usadas como padrão são clonadas por instância.

---

# 15. Construtores — 3.4

```leviata
classe Pessoa {
    var nome = ""

    funcao iniciar(nome) {
        este.nome = nome
    }
}

var alex = novo Pessoa("Alex")
```

`iniciar(...)` é chamado automaticamente por `novo Classe(args...)`. A aridade deve corresponder. O retorno de `iniciar` é ignorado e `novo` sempre devolve a instância.

---

# 16. Herança e `super` — 3.5

```leviata
classe Funcionario herda Pessoa {
    var cargo = ""

    funcao iniciar(nome, cargo) {
        super.iniciar(nome)
        este.cargo = cargo
    }
}
```

`super.metodo(...)` começa a busca na classe pai lexical da classe onde o método atual foi declarado. Construtores pais não são chamados automaticamente em cascata.

---

# 17. Módulos — 3.3 a 3.7

Importação simples:

```leviata
usar "util.pt"
```

API explícita:

```leviata
exportar funcao dobro(x) {
    retorne x * 2
}
```

Alias:

```leviata
usar "util.pt" como util
escreva(util.dobro(5))
```

Regras atuais:

- `usar` e `exportar` só aparecem no escopo principal;
- caminho relativo é resolvido a partir do importador;
- cada caminho normalizado executa uma vez;
- ciclos são detectados;
- `exportar` aceita `var`, `const`, `funcao` e `classe`;
- com alias, apenas `alias.simbolo` fica visível;
- privados não vazam;
- funções e métodos exportados continuam enxergando privados do próprio módulo;
- dois módulos podem usar os mesmos nomes sem colisão;
- `novo alias.Classe(...)` funciona;
- módulos antigos sem `exportar` tratam declarações de topo como públicas por compatibilidade.

O isolamento é feito por linkedição para nomes internos `@m...`, não por uma VM separada para cada arquivo.

---

# 18. O ponto (`.`)

O ponto mantém usos coerentes:

```leviata
camera.seguir(x, y)      /* API estática */
foto.desenhe(x, y)       /* método */
pessoa.nome              /* campo */
util.dobro(5)            /* export de módulo */
novo util.Caixa(10)      /* classe exportada */
```

O compilador resolve nomes qualificados conhecidos antes do acesso dinâmico a membro.

---

# 19. Modelo interno

Detalhes internos não são sintaxe pública.

Podem existir nomes como:

```text
@classe.criar
@obj.metodo
@indice.obter
@m2:4
__leviata_par_0
jogo_desenhar_retangulo
```

Eles existem para implementar a linguagem e podem mudar sem alterar o código Leviata válido.

---

# 20. Política de mudanças futuras

Toda mudança pública deve responder:

1. Qual conceito representa?
2. Qual é o paralelo comum?
3. Qual será a única forma canônica?
4. Por que essa forma é melhor para o Leviata?
5. A implementação já suporta a forma?
6. Qual é o impacto de compatibilidade?

Não aceitar uma decisão apenas porque outra linguagem faz assim.

---

# 21. Registro histórico

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

Mapas viraram estrutura pública da linguagem com literal `{ chave: valor }`, acesso por ponto e índice, escrita, `chaves`, `valores` e integração com operações genéricas.

## Leviata 3.9

Mapas ganharam iteração direta de chave e valor:

```leviata
para chave, valor em mapa {
    escreva(chave, valor)
}
```

A coleção é avaliada uma vez e a iteração trabalha sobre snapshot de pares. A implementação reutiliza o foreach existente por meio de uma normalização para `pares(...)`, preservando `pare`, `continue` e o bytecode estável.

---

# 22. Estado canônico atual

A referência implementada é **Leviata 3.9**.

Recursos públicos consolidados:

```text
escreva / leia
se / senao / enquanto / para
para item em lista
para chave, valor em mapa
funcao / retorne
pare / continue
verdadeiro / falso / nulo
e / ou / nao
listas
mapas
chaves / valores / pares
classe / novo / este / herda / super
iniciar como construtor
usar / exportar / como
módulos isolados
txt / int / dec / bool
objetos de recurso
APIs qualificadas
cache de módulos
detecção de ciclos
```

Qualquer mudança futura deve atualizar este documento para ser considerada parte da escrita canônica do Leviata.
