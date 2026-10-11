# Leviata — Especificação Canônica e Registro Histórico

> Documento normativo da escrita pública do Leviata.
>
> Qualquer mudança futura de sintaxe, palavra-chave ou API pública deve registrar aqui o paralelo conceitual, a forma oficial escolhida e o motivo da decisão.

**Criado:** 10 de outubro de 2026  
**Última revisão:** 11 de outubro de 2026  
**Versão implementada:** Leviata 3.4  

---

# 1. Identidade

Leviata é uma linguagem própria escrita em português brasileiro. Não é Python traduzido, C traduzido, JavaScript traduzido nem Portugol ampliado.

Conceitos universais podem ter paralelo em outras linguagens, mas a escrita pública do Leviata é definida por esta especificação.

Regra principal:

> **Uma ideia, uma forma oficial.**

Exemplo:

```leviata
escreva("Ola")
```

A saída pública não possui ao mesmo tempo `print`, `imprimir`, `escrever` e `escreva`. A forma oficial é `escreva`.

---

# 2. Regras de escrita

A linguagem deve ser:

- curta sem ser criptográfica;
- legível;
- previsível;
- consistente;
- simples para começar;
- capaz de crescer sem nomes gigantes;
- independente da sintaxe de uma linguagem estrangeira específica.

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

Não são necessários `entao`, `faca`, `fimse`, `fimenquanto` e equivalentes.

Palavras reservadas evitam acentos para facilitar teclado, ferramentas e portabilidade.

---

# 3. Vocabulário fundamental

| Conceito | Paralelo comum | Leviata |
|---|---|---|
| saída | `print` | `escreva` |
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
| módulo | `import`, `use` | `usar` |

Formas históricas removidas:

```text
retornar  -> retorne
quebrar   -> pare
continuar -> continue
importar  -> usar
herdar    -> herda
```

`&&`, `||` e `!` não pertencem à lógica canônica. `!=` continua sendo diferença.

---

# 4. Entrada, saída e conversões

```leviata
var nome = leia("Nome: ")
var idade = int(leia("Idade: "))
escreva(nome, idade)
```

Conversões frequentes usam nomes curtos:

```text
txt  texto
int  inteiro
dec  decimal
bool booleano
```

Também existe:

```leviata
tipo(valor)
```

`tipo()` retorna nomes canônicos e reconhece recursos, classes e instâncias.

---

# 5. Controle de fluxo

Condição:

```leviata
se idade >= 18 e possui_documento {
    escreva("Entrada permitida")
} senao {
    escreva("Entrada negada")
}
```

Faixa inclusiva:

```leviata
para n de 1 ate 10 {
    escreva(n)
}
```

Coleção:

```leviata
para fruta em frutas {
    escreva(fruta)
}
```

Controle de laço:

```leviata
se ignorar { continue }
se terminou { pare }
```

`e` e `ou` possuem short-circuit real. `pare` e `continue` respeitam a limpeza de variáveis locais dos blocos abandonados.

---

# 6. Funções

```leviata
funcao soma(a, b) {
    retorne a + b
}
```

Função sem `retorne` explícito devolve `nulo`.

---

# 7. Listas e texto

```leviata
var nomes = ["Ana", "Bia"]
var vazia = lista()

adicionar(nomes, "Caio")
inserir(nomes, 1, "Duda")
remover(nomes, "Bia")
limpar(vazia)
```

Texto:

```text
maiusculas(txt)
minusculas(txt)
contem(txt, trecho)
comeca(txt, inicio)
termina(txt, fim)
trecho(txt, inicio, fim)
tamanho(txt)
```

Mudanças históricas:

```text
lista_adicionar -> adicionar
lista_nova      -> lista
comeca_com      -> comeca
termina_com     -> termina
fatiar          -> trecho
```

---

# 8. Matemática

Matemática não recebe `mat.` quando o nome já explica a operação.

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

Não:

```text
mat.raiz(25)
```

`pi` é valor, não função.

---

# 9. APIs especializadas

Quando um recurso não possui receptor natural, APIs maiores podem usar domínio:

```leviata
jogo.iniciar(...)
desenho.retangulo(...)
camera.seguir(...)
entidade.atualizar(...)
```

Isso substitui nomes históricos como:

```text
jogo_desenhar_retangulo
jogo_camera_seguir
entidade_atualizar_tilemap
```

Códigos mágicos de teclado também não fazem parte da API recomendada:

```leviata
teclado.pressionada("A")
teclado.apertada("espaco")
teclado.pressionada("esquerda")
```

---

# 10. Objetos nativos — 3.1

Recursos com estado usam objeto e métodos.

Imagem:

```leviata
var foto = imagem("jogador.png")
foto.desenhe(10, 20)
escreva(foto.caminho)
```

Som:

```leviata
var efeito = som("efeito.wav")
efeito.volume(0.5)
efeito.toque()
```

Arquivo:

```leviata
var dados = arquivo("dados.txt")
dados.escreva("Ola")
escreva(dados.leia())
```

A antiga forma estática `arquivo.leia("dados.txt")` deixou de ser a forma pública principal.

Regra geral:

> Função autoexplicativa fica global. Recurso com estado vira objeto. Namespace só existe quando realmente evita ambiguidade.

---

# 11. Classes — 3.2

```leviata
classe Pessoa {
    var nome = "sem nome"

    funcao nomear(valor) {
        este.nome = valor
    }
}

var pessoa = novo Pessoa()
pessoa.nomear("Ana")
```

Decisões:

- `classe` declara um tipo do usuário;
- `novo Nome()` cria uma instância;
- `este` é o receptor implícito;
- campos são acessados por ponto;
- métodos são funções de bytecode normais com `este` implícito;
- listas usadas como valor padrão são clonadas por instância;
- classes ficam no escopo principal nesta fase da linguagem.

---

# 12. Herança — 3.2

```leviata
classe Gato herda Animal {
    var vidas = 7

    funcao falar() {
        retorne este.nome + " disse miau"
    }
}
```

A busca de membros percorre filho -> pai. A classe filha herda campos e métodos e pode sobrescrever métodos.

---

# 13. Construtores — 3.4

Forma canônica:

```leviata
classe Pessoa {
    var nome = ""
    var idade = 0

    funcao iniciar(nome, idade) {
        este.nome = nome
        este.idade = idade
    }
}

var alex = novo Pessoa("Alex", 29)
```

Paralelo conceitual:

```text
constructor / __init__ -> iniciar
new Class(args...)     -> novo Classe(args...)
```

## Por que `iniciar`?

- é verbo curto em português;
- descreve o papel sem copiar `constructor` ou `__init__`;
- combina com a ideia de preparar a instância recém-criada;
- evita criar uma palavra-chave nova só para construtor.

## Semântica oficial do 3.4

1. `novo Classe(args...)` cria uma nova instância;
2. se a classe declara `funcao iniciar(...)`, esse método é chamado automaticamente;
3. os argumentos de `novo` correspondem aos parâmetros de `iniciar`;
4. aridade incorreta gera erro;
5. qualquer valor retornado por `iniciar` é ignorado;
6. `novo` sempre devolve a instância criada;
7. classe sem `iniciar` possui construtor vazio e usa `novo Classe()`;
8. `novo` continua sendo expressão e pode aparecer aninhado;
9. strings e comentários contendo o texto `novo Classe(...)` não são reinterpretados;
10. construtores não são herdados no 3.4.

Exemplo aninhado:

```leviata
var caixa = novo Caixa(novo Pessoa("Bia", 25))
```

## Decisão sobre herança de construtores

No 3.4, uma classe filha que precisa receber argumentos declara seu próprio `iniciar(...)`.

```leviata
classe Funcionario herda Pessoa {
    var cargo = ""

    funcao iniciar(nome, idade, cargo) {
        este.nome = nome
        este.idade = idade
        este.cargo = cargo
    }
}
```

A escolha é deliberada. Herdar construtores automaticamente exigiria definir também chamada ao construtor pai, ordem de inicialização e uma forma equivalente a `super`. Isso deve ser decidido como um recurso completo, não improvisado.

## Implementação interna

A escrita pública continua sendo:

```leviata
novo Pessoa("Alex", 29)
```

Internamente, o front-end normaliza a construção e o compilador gera uma função construtora escondida para cada classe. Essa função:

1. cria a instância com os campos padrão;
2. chama `iniciar(...)` quando existe;
3. ignora o retorno de `iniciar`;
4. devolve a instância.

Essa estratégia reutiliza a VM e o sistema de métodos existentes sem criar uma segunda semântica de chamada.

---

# 14. Módulos — 3.3+

Forma canônica:

```leviata
usar "modulos/util.pt"
```

Regras atuais:

1. `usar` só é válido no escopo principal;
2. o caminho é relativo ao arquivo importador;
3. módulos podem importar outros módulos;
4. cada caminho normalizado executa uma única vez por runtime;
5. imports repetidos não repetem efeitos de topo;
6. ciclos são detectados;
7. o módulo é resolvido e executado antes da compilação do importador;
8. funções, classes e variáveis entram no namespace global compartilhado;
9. a extensão do arquivo é explícita.

Ainda não existem:

```text
exportar
usar ... como ...
namespace isolado por arquivo
visibilidade privada de módulo
```

A decisão de começar com namespace compartilhado evita fingir isolamento apenas renomeando símbolos sem corrigir referências internas.

Construtores 3.4 funcionam também para classes definidas em módulos, porque cada módulo passa pelo mesmo front-end e compilador.

---

# 15. Modelo interno de objetos

Recursos nativos usam `LV_TYPE_OBJECT` com metadados leves.

Classes e instâncias usam `LV_TYPE_MAP`, rastreado pelo GC.

Chaves internas começam com `@` e não colidem com identificadores normais:

```text
@tipo
@nome
@pai
@classe
@campo:<nome>
@metodo:<nome>
```

O despacho de campos e métodos é compartilhado entre recursos nativos e instâncias do usuário.

---

# 16. Separação entre implementação e superfície pública

Nomes C internos não definem a linguagem.

Internamente pode continuar existindo:

```text
jogo_desenhar_retangulo
```

Publicamente:

```leviata
desenho.retangulo(...)
```

Da mesma forma, nomes internos gerados para construtores não fazem parte da sintaxe que o programador precisa aprender.

---

# 17. Política de mudanças futuras

Toda mudança pública deve responder:

1. Qual conceito representa?
2. Qual é o paralelo comum em outras linguagens?
3. Qual será a única forma canônica no Leviata?
4. Por que essa forma é melhor para o Leviata?
5. A implementação já suporta essa forma ou é apenas proposta?
6. Qual o impacto de compatibilidade?

Não aceitar uma decisão apenas porque “Python faz assim” ou “C faz assim”.

---

# 18. Registro histórico

## 10 de outubro de 2026 — Documento inicial

O documento nasceu para organizar nomes que haviam crescido sem regra única no protótipo. Foi definida a regra **uma ideia, uma forma oficial**.

## 10 de outubro de 2026 — Siglas curtas

```text
texto    -> txt
inteiro  -> int
decimal  -> dec
booleano -> bool
```

Palavras já claras, como `se`, `para`, `escreva` e `leia`, não seriam abreviadas artificialmente.

## 10 de outubro de 2026 — Matemática sem `mat.`

Foi rejeitada a obrigação de `mat.raiz(25)` porque `raiz(25)` já identifica a operação.

## 11 de outubro de 2026 — Leviata 3.0

A escrita canônica passou para lexer, parser, compilador, biblioteca, exemplos e testes.

Entraram `escreva`, `leia`, `retorne`, `pare`, `continue`, `para ... em ...`, `e`, `ou`, `nao`, conversões curtas e APIs qualificadas.

## 11 de outubro de 2026 — Leviata 3.1

`imagem`, `som` e `arquivo` passaram a ser objetos reais com campos e métodos.

## 11 de outubro de 2026 — Leviata 3.2

Foram implementados `classe`, `novo`, `este`, `herda`, campos de instância, métodos, sobrescrita e herança.

## 11 de outubro de 2026 — Leviata 3.3

`usar "arquivo.pt"` passou a carregar módulos reais com caminho relativo, execução única, imports aninhados e detecção de ciclo.

## 11 de outubro de 2026 — Leviata 3.4

`novo Classe(args...)` passou a aceitar argumentos e chamar automaticamente `iniciar(...)`.

A implementação foi validada com:

- construtor com múltiplos argumentos;
- retorno de `iniciar` ignorado;
- `novo` aninhado;
- classe sem construtor explícito;
- classe filha com construtor próprio;
- erro de aridade;
- strings e comentários contendo `novo`;
- regressões antigas;
- AddressSanitizer e UndefinedBehaviorSanitizer.
