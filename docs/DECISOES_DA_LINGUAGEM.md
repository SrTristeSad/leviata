# Leviata — Especificação Canônica e Registro Histórico

> Documento normativo da escrita pública do Leviata.
>
> Este arquivo registra **como a linguagem deve ser escrita e entendida**, qual é o paralelo conceitual com linguagens em inglês e por que cada decisão foi tomada. Não é uma apostila de programação.

**Criado:** 10 de outubro de 2026  
**Última revisão:** 11 de outubro de 2026  
**Versão implementada:** Leviata 3.6  

---

# 1. Identidade

Leviata é uma linguagem própria escrita em português brasileiro. Não é Python traduzido, C traduzido, JavaScript traduzido nem Portugol ampliado.

Conceitos como condição, função, classe, módulo e herança são universais. O paralelo em inglês serve apenas para identificar o conceito correspondente.

Regra central:

> **Uma ideia, uma forma oficial.**

Exemplo:

```leviata
escreva("Ola")
```

A linguagem pública não deve manter simultaneamente `print`, `imprimir`, `escrever` e `escreva` para a mesma operação.

---

# 2. Princípios de escrita

A linguagem deve ser curta sem ser criptográfica, legível, previsível, consistente, fácil de memorizar e capaz de crescer sem nomes gigantes.

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

Palavras reservadas e APIs canônicas evitam acentos para facilitar teclado, tooling e portabilidade. Siglas são permitidas quando são reconhecíveis e economizam escrita sem prejudicar leitura.

---

# 3. Vocabulário canônico

| Conceito | Paralelo comum | Leviata |
|---|---|---|
| saída | `print`, `printf` | `escreva` |
| entrada | `input`, `read` | `leia` |
| variável | `var`, `let` | `var` |
| constante | `const` | `const` |
| condição | `if` | `se` |
| alternativa | `else` | `senao` |
| laço condicional | `while` | `enquanto` |
| laço | `for` | `para` |
| item em coleção | `in` | `em` |
| início de faixa | `from` | `de` |
| fim de faixa | `to` | `ate` |
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
retornar  -> retorne
quebrar   -> pare
continuar -> continue
importar  -> usar
herdar    -> herda
escrever  -> escreva
```

`&&`, `||` e `!` não pertencem à lógica canônica. `!=` continua sendo diferença.

---

# 4. Tipos e conversões

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

`tipo(valor)` usa nomes canônicos do Leviata.

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

`e` e `ou` possuem short-circuit real. `pare` e `continue` fazem unwind correto das variáveis locais abandonadas.

---

# 6. Funções, listas e texto

```leviata
funcao soma(a, b) {
    retorne a + b
}
```

Função sem `retorne` explícito devolve `nulo`.

Lista:

```leviata
var nomes = ["Ana", "Bia"]
adicionar(nomes, "Caio")
inserir(nomes, 1, "Duda")
remover(nomes, "Bia")
```

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

---

# 7. Matemática

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

Preferir `raiz(25)`, não `mat.raiz(25)`.

---

# 8. Função, objeto ou namespace

Regra oficial:

> **Função autoexplicativa fica global. Recurso com estado vira objeto. Namespace só é usado quando realmente evita ambiguidade ou organiza uma API ampla.**

```leviata
raiz(25)

var foto = imagem("player.png")
foto.desenhe(10, 20)

jogo.iniciar("Meu jogo", 960, 540, 60)
camera.seguir(x, y)
```

Isso substitui APIs históricas como `jogo_desenhar_retangulo` e `jogo_camera_seguir`.

---

# 9. Objetos nativos — 3.1

Recursos com estado usam objeto e métodos:

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

# 10. Classes e herança — 3.2

```leviata
classe Pessoa {
    var nome = "sem nome"

    funcao nomear(valor) {
        este.nome = valor
    }
}

classe Funcionario herda Pessoa {
    var cargo = "dev"
}
```

Decisões:

- `classe` declara um tipo do usuário;
- `novo` cria uma instância;
- `este` é o receptor implícito;
- métodos usam bytecode normal com `este` como primeiro argumento implícito;
- listas padrão são clonadas por instância;
- lookup normal de método percorre filho -> pai.

---

# 11. Construtores — 3.4

Forma canônica:

```leviata
classe Pessoa {
    var nome = ""

    funcao iniciar(nome) {
        este.nome = nome
    }
}

var alex = novo Pessoa("Alex")
```

Paralelo:

```text
constructor / __init__ -> iniciar
new Class(args...)     -> novo Classe(args...)
```

Semântica:

1. `novo Classe(args...)` cria a instância;
2. `iniciar(...)`, se existir, é chamado automaticamente;
3. a aridade deve corresponder;
4. o retorno de `iniciar` é ignorado;
5. `novo` sempre devolve a instância;
6. classe sem `iniciar` usa `novo Classe()`;
7. `novo` é expressão e pode aparecer aninhado.

---

# 12. `super` — 3.5

`super` representa a implementação da classe pai **a partir da classe onde o método atual foi declarado**.

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
```

Decisões:

- `super.metodo(...)` só é válido dentro de método de classe com pai;
- a busca começa no pai lexical da classe atual;
- isso evita recursão no override atual;
- `super.iniciar(...)` é a chamada explícita ao construtor do pai;
- construtores não são chamados automaticamente em cascata;
- `super.campo` não existe no 3.5; campos herdados continuam acessíveis por `este.campo`.

`super` foi mantido porque é curto e tecnicamente reconhecível. Uma tradução longa reduziria a simplicidade sem melhorar a precisão.

---

# 13. Módulos básicos — 3.3

```leviata
usar "modulos/util.pt"
```

Semântica:

1. `usar` só aparece no escopo principal;
2. caminho é relativo ao importador;
3. imports aninhados usam o módulo atual como base;
4. cada caminho normalizado executa uma vez;
5. caminhos equivalentes compartilham cache;
6. ciclos são detectados;
7. módulo executa antes da compilação do importador;
8. extensão do arquivo é explícita;
9. namespace global é compartilhado.

---

# 14. Exportação e alias — 3.6

Um módulo declara sua API pública com `exportar`:

```leviata
exportar const nome = "Ferramentas"

exportar funcao dobro(valor) {
    retorne valor * 2
}

exportar classe Caixa {
    var valor = 0

    funcao iniciar(valor) {
        este.valor = valor
    }
}

var segredo = 123
```

Formas aceitas:

```text
exportar var nome = ...
exportar const nome = ...
exportar funcao nome(...) { ... }
exportar classe Nome { ... }
```

Importação com alias:

```leviata
usar "biblioteca.pt" como util

escreva(util.nome)
escreva(util.dobro(5))
var caixa = novo util.Caixa(10)
```

Decisões:

- o alias expõe somente símbolos marcados com `exportar`;
- símbolo não exportado não recebe `alias.simbolo`;
- classe exportada recebe também `novo alias.Classe(...)`;
- o mesmo módulo pode receber mais de um alias sem executar novamente;
- `usar` e `exportar` ficam no escopo principal.

## Limite intencional do 3.6

`usar ... como ...` é uma **visão qualificada da API exportada**, não um namespace isolado completo.

Por compatibilidade com a VM e os bytecodes da série 3.x, símbolos originais do módulo continuam na tabela global compartilhada:

```leviata
usar "biblioteca.pt" como util

util.dobro(5)  /* API pública explícita */
dobro(5)       /* ainda existe por compatibilidade 3.x */
```

Isolamento real exigirá ambiente/tabela de globais por módulo e referências internas ligadas a esse ambiente. Não será simulado escondendo nomes superficialmente.

---

# 15. O ponto (`.`)

O ponto possui usos consistentes:

```leviata
camera.seguir(x, y)      /* API estática */
foto.desenhe(x, y)       /* método de objeto */
pessoa.nome              /* campo */
util.dobro(5)            /* exportação qualificada */
novo util.Caixa(10)      /* classe exportada */
```

O compilador resolve nomes qualificados conhecidos antes do acesso dinâmico a membro.

---

# 16. Modelo interno

Recursos nativos usam `LV_TYPE_OBJECT`. Classes e instâncias usam `LV_TYPE_MAP`, rastreado pelo GC.

Chaves internas começam com `@`:

```text
@tipo
@nome
@pai
@classe
@campo:<nome>
@metodo:<nome>
```

`super` usa lookup específico começando em `@pai`.

Módulos mantêm:

- caminhos já carregados;
- pilha de caminhos em carregamento para detectar ciclos;
- registro de símbolos exportados por caminho normalizado.

Marcadores internos usados pelo front-end são consumidos antes da compilação e suas ASTs são liberadas integralmente.

---

# 17. Implementação versus superfície pública

Nome interno não é especificação da linguagem.

Internamente podem existir:

```text
jogo_desenhar_retangulo
@classe.super_metodo
__leviata_exportar
__leviata_alias_import
```

Publicamente:

```leviata
desenho.retangulo(...)
super.metodo(...)
exportar funcao nome(...) { ... }
usar "arquivo.pt" como modulo
```

---

# 18. Política de mudanças futuras

Toda mudança pública deve responder:

1. Qual conceito representa?
2. Qual é o paralelo comum?
3. Qual será a única forma canônica?
4. Por que essa forma é melhor para o Leviata?
5. A implementação já suporta essa forma ou é proposta?
6. Qual o impacto de compatibilidade?

Não aceitar uma decisão apenas porque outra linguagem faz assim.

---

# 19. Registro histórico

## 10 de outubro de 2026 — Documento inicial

A especificação nasceu para organizar um protótipo que havia acumulado múltiplos nomes e APIs longas sem regra única. Foi estabelecido **uma ideia, uma forma oficial**.

## 10 de outubro de 2026 — Vocabulário em português

```text
print  -> escreva
input  -> leia
if     -> se
else   -> senao
while  -> enquanto
for    -> para
return -> retorne
break  -> pare
```

## 10 de outubro de 2026 — Siglas curtas

```text
texto    -> txt
inteiro  -> int
decimal  -> dec
booleano -> bool
```

Siglas são aceitas quando reconhecíveis, sem transformar a linguagem em abreviações opacas.

## 10 de outubro de 2026 — Matemática sem `mat.`

`mat.raiz(25)` foi rejeitado porque `raiz(25)` já identifica a operação.

## 11 de outubro de 2026 — Leviata 3.0

A escrita canônica passou ao lexer, parser, compilador, biblioteca, exemplos e testes. Entraram `escreva/leia`, `retorne`, `pare/continue`, `para ... em ...`, `e/ou/nao`, conversões curtas, listas curtas, matemática global, APIs qualificadas e teclas por nome.

## 11 de outubro de 2026 — Leviata 3.1

`imagem`, `som` e `arquivo` passaram a ser objetos com campos e métodos.

## 11 de outubro de 2026 — Leviata 3.2

Foram implementados `classe`, `novo`, `este`, `herda`, campos de instância, métodos, sobrescrita e herança.

## 11 de outubro de 2026 — Leviata 3.3

`usar "arquivo.pt"` passou a carregar módulos reais com caminho relativo, cache de execução única, import aninhado, detecção de ciclo e namespace global compartilhado.

## 11 de outubro de 2026 — Leviata 3.4

Construtores ganharam argumentos com `funcao iniciar(...)` e `novo Classe(...)`.

## 11 de outubro de 2026 — Leviata 3.5

Foi implementado `super.metodo(...)` com semântica lexical de classe pai, incluindo `super.iniciar(...)`.

## 11 de outubro de 2026 — Leviata 3.6

Módulos ganharam API explícita:

```leviata
exportar funcao dobro(x) { retorne x * 2 }
usar "util.pt" como util
util.dobro(5)
novo util.Caixa(10)
```

O alias expõe apenas símbolos exportados, mas permanece uma visão qualificada sobre o namespace global compartilhado.

Durante a implementação, o LeakSanitizer encontrou ASTs de marcadores internos que eram tornadas inalcançáveis ao serem substituídas por `nulo`. O runtime foi corrigido para liberar a árvore consumida antes de inserir o nó substituto. ASan/UBSan voltaram a ficar verdes.

---

# 20. Estado canônico atual

A referência implementada é **Leviata 3.6**.

Recursos públicos consolidados:

```text
escreva / leia
se / senao / enquanto / para
funcao / retorne
pare / continue
verdadeiro / falso / nulo
e / ou / nao
classe / novo / este / herda / super
iniciar como construtor
usar
exportar
usar ... como ...
txt / int / dec / bool
objetos de recurso
APIs qualificadas
módulos com cache e ciclos detectados
```

Qualquer mudança futura deve atualizar este documento antes de ser considerada parte da escrita canônica do Leviata.
