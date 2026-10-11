# Leviata — Especificação Canônica e Registro Histórico

> Documento normativo da escrita pública do Leviata e registro histórico das decisões de linguagem.
>
> Quando sintaxe, palavra-chave ou API pública mudar, este arquivo deve registrar **o que mudou, o paralelo conceitual e por que mudou**.

**Criado:** 10 de outubro de 2026  
**Última revisão:** 11 de outubro de 2026  
**Versão implementada:** Leviata 3.0  

---

# 1. Identidade

Leviata não é Python em português, C em português, JavaScript em português nem Portugol ampliado.

Ele é uma linguagem própria escrita em português brasileiro. Conceitos como variável, condição, função e laço naturalmente existem em muitas linguagens; a coluna em inglês deste documento serve apenas como **paralelo semântico**.

Regra principal:

> **Uma ideia, uma forma oficial.**

O usuário não deve aprender três nomes para a mesma operação.

Exemplo:

```leviata
escreva("Ola")
```

A forma pública não é simultaneamente `print`, `imprimir`, `escrever` e `escreva`. No Leviata 3, é `escreva`.

---

# 2. Regras de escrita

A linguagem deve ser:

- curta;
- legível;
- previsível;
- consistente;
- simples para começar;
- capaz de crescer sem obrigar o programador a decorar nomes gigantes.

Não traduzimos símbolos universais desnecessariamente:

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

Por isso não precisamos de `entao`, `faca`, `fimse`, `fimenquanto` etc.

Palavras reservadas e nomes públicos evitam acentos para facilitar teclado, ferramentas e portabilidade:

```text
funcao
senao
nao
ate
```

Strings podem usar português normal:

```leviata
escreva("Você está programando em português.")
```

---

# 3. Vocabulário canônico

| Conceito | Inglês comum | Leviata 3 | Motivo |
|---|---|---|---|
| saída | `print`, `printf` | `escreva` | comando direto e tradicional em algoritmos em português |
| entrada | `input`, `read` | `leia` | forma complementar a `escreva` |
| variável | `var`, `let` | `var` | abreviação curta e clara |
| constante | `const` | `const` | curta e reconhecível |
| condição | `if` | `se` | equivalente natural |
| alternativa | `else` | `senao` | equivalente natural |
| laço condicional | `while` | `enquanto` | descreve diretamente a repetição |
| laço | `for` | `para` | permite sintaxe própria em português |
| item em coleção | `in` | `em` | `para item em lista` |
| início de faixa | `from` | `de` | `para n de 1 ate 10` |
| fim de faixa | `to` | `ate` | completa a faixa |
| função | `function`, `def`, `fn` | `funcao` | termo técnico em português |
| retorno | `return` | `retorne` | comando imperativo |
| interromper laço | `break` | `pare` | descreve exatamente o efeito |
| próxima iteração | `continue` | `continue` | curto e compreensível em português |
| verdadeiro | `true` | `verdadeiro` | explícito |
| falso | `false` | `falso` | explícito |
| ausência de valor | `null`, `nil`, `None` | `nulo` | um valor nulo único |
| E lógico | `and`, `&&` | `e` | leitura natural |
| OU lógico | `or`, `||` | `ou` | leitura natural |
| negação | `not`, `!` | `nao` | leitura natural |

Forma antiga -> forma canônica:

```text
retornar   -> retorne
quebrar    -> pare
continuar  -> continue
importar   -> usar       (reservado para módulos futuros)
herdar     -> herda      (reservado para OO futura)
```

`&&`, `||` e `!` não são a escrita lógica canônica. `!=` continua existindo como operador de diferença.

---

# 4. Entrada e saída

## `print` -> `escreva`

```leviata
escreva("Ola")
escreva("Idade:", idade)
```

Motivos:

- curto;
- natural como instrução;
- conhecido em ensino de algoritmos em português;
- não sugere necessariamente impressão física como `imprimir`;
- elimina aliases concorrentes.

## `input/read` -> `leia`

```leviata
var nome = leia("Nome: ")
```

Entrada não converte tipo automaticamente:

```leviata
var idade = int(leia("Idade: "))
```

---

# 5. Tipos e conversões

Siglas são permitidas quando são curtas, reconhecíveis e usadas com frequência.

| Conceito | Inglês comum | Leviata |
|---|---|---|
| texto | `str`, `String` | `txt` |
| inteiro | `int` | `int` |
| decimal | `float`, `double` | `dec` |
| booleano | `bool`, `Boolean` | `bool` |

```leviata
var a = txt(123)
var b = int("42")
var c = dec("3.14")
var d = bool(1)
```

Também existe:

```leviata
tipo(valor)
```

Retornos públicos de `tipo` usam os nomes canônicos: `txt`, `int`, `dec`, `bool`, `lista`, `nulo` etc.

Substituições:

```text
para_texto   -> txt
para_inteiro -> int
para_decimal -> dec
```

---

# 6. Condições e lógica

```leviata
se idade >= 18 e possui_documento {
    escreva("Entrada permitida")
} senao {
    escreva("Entrada negada")
}
```

Negação:

```leviata
se nao bloqueado {
    escreva("Pode continuar")
}
```

`e` e `ou` possuem short-circuit real: o operando direito só é avaliado quando necessário.

---

# 7. Repetições

## Enquanto

```leviata
enquanto conectado {
    atualizar()
}
```

## Faixa

```leviata
para n de 1 ate 10 {
    escreva(n)
}
```

`ate` é inclusivo: o exemplo inclui `10`.

## Coleção

```leviata
var frutas = ["maca", "uva", "banana"]

para fruta em frutas {
    escreva(fruta)
}
```

Essa forma evita obrigar o iniciante a usar índices para simplesmente percorrer uma lista.

## Pare e continue

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

No Leviata 3 ambos geram controle de fluxo real no bytecode e respeitam a limpeza de variáveis locais dos blocos abandonados.

---

# 8. Funções

```leviata
funcao soma(a, b) {
    retorne a + b
}
```

Paralelo:

```text
function / def / fn -> funcao
return              -> retorne
```

Função sem `retorne` explícito retorna `nulo`.

---

# 9. Listas

Criação literal:

```leviata
var nomes = ["Ana", "Bia"]
```

Criação vazia:

```leviata
var nomes = lista()
```

API canônica:

```text
adicionar(lista, valor)
inserir(lista, indice, valor)
remover(lista, valor)
limpar(lista)
contem(lista, valor)
tamanho(lista)
```

O prefixo `lista_` foi removido porque o primeiro argumento já informa o domínio.

```text
lista_adicionar -> adicionar
lista_nova      -> lista
```

---

# 10. Texto

API base:

```text
maiusculas(txt)
minusculas(txt)
contem(txt, trecho)
comeca(txt, inicio)
termina(txt, fim)
trecho(txt, inicio, fim)
tamanho(txt)
```

Mudanças:

```text
comeca_com -> comeca
termina_com -> termina
fatiar -> trecho
```

`contem` é intencionalmente genérico: funciona tanto para texto quanto para lista.

---

# 11. Matemática

Matemática não recebe um prefixo `mat.` quando o nome da operação já explica sua finalidade.

Preferir:

```leviata
raiz(25)
```

Não:

```text
mat.raiz(25)
```

API canônica:

```text
raiz(x)
potencia(base, expoente)
abs(x)
arredondar(x)
teto(x)
piso(x)
seno(x)
cos(x)
tan(x)
log(x)
max(a, b)
min(a, b)
aleatorio()
aleatorio_int(min, max)
pi
```

Abreviações escolhidas por serem convenções reconhecíveis:

```text
absoluto          -> abs
cosseno           -> cos
tangente          -> tan
logaritmo         -> log
maximo            -> max
minimo            -> min
aleatorio_inteiro -> aleatorio_int
```

`pi` é um valor, não uma função:

```leviata
escreva(pi)
```

---

# 12. Regra para APIs maiores

Não criar nomes globais enormes apenas para evitar colisão.

Forma antiga:

```text
jogo_desenhar_retangulo(...)
jogo_camera_seguir(...)
entidade_atualizar_tilemap(...)
```

Forma canônica:

```leviata
desenho.retangulo(...)
camera.seguir(...)
entidade.atualizar_mapa(...)
```

O ponto serve para organizar APIs especializadas sem contaminar o vocabulário básico.

O compilador do Leviata 3 resolve nomes qualificados como uma unidade da API pública.

---

# 13. Entrada de jogos

Códigos mágicos de tecla não fazem parte da API pública recomendada.

Antigo:

```text
jogo_tecla(65)
```

Canônico:

```leviata
teclado.pressionada("A")
teclado.apertada("espaco")
teclado.pressionada("esquerda")
```

Isso reduz memorização sem impedir que a implementação interna continue usando códigos de tecla.

---

# 14. Domínios especializados implementados no 3.0

```text
jogo.*
desenho.*
teclado.*
mouse.*
audio.*
camera.*
colisao.*
animacao.*
cena.*
mapa.*
entidade.*
arquivo.*
```

Exemplos:

```leviata
jogo.iniciar("Jogo", 960, 540, 60)
desenho.retangulo(10, 10, 80, 80, 255, 255, 255)
camera.seguir(100, 100)
entidade.criar("player", 0, 0, 32, 48)
arquivo.leia("dados.txt")
```

`arquivo.*` é um caso onde o domínio evita ambiguidade real com `leia`/`escreva` do console.

---

# 15. Ideia de recursos como objetos — não implementada no 3.0

Durante o desenho da linguagem foi considerada uma API futura como:

```text
var foto = imagem("jogador.png")
foto.desenhe(10, 20)

var efeito = som("efeito.wav")
efeito.toque()

var dados = arquivo("dados.txt")
dados.leia()
```

A ideia continua válida para estudo, mas **não é sintaxe/API prometida pelo Leviata 3.0**.

Ela exige semântica real de propriedades, métodos e recursos com estado. Não será fingida apenas com aliases.

Se esse modelo for implementado, este documento deve registrar a mudança antes de substituir a API pública existente.

---

# 16. Palavras reservadas para recursos futuros

O lexer do Leviata 3 reserva:

```text
usar
classe
novo
este
herda
```

Paralelos conceituais:

```text
import/use      -> usar
class           -> classe
new             -> novo
this/self       -> este
extends/inherit -> herda
```

Essas palavras definem a direção de nomenclatura, mas módulos e orientação a objetos completos ainda não fazem parte da implementação funcional do 3.0.

Não documentar exemplos de módulos/classes como se já fossem executáveis.

---

# 17. Separação entre implementação e superfície pública

A implementação C pode manter nomes internos históricos para evitar reescrever código estável sem necessidade.

Exemplo interno:

```text
jogo_desenhar_retangulo
```

Superfície `.pt`:

```leviata
desenho.retangulo(...)
```

A camada `leviata_canon` registra os nomes públicos e remove aliases antigos da tabela global da VM.

Portanto, nome interno não é especificação da linguagem.

---

# 18. Política de mudanças futuras

Toda mudança pública deve responder:

1. Qual conceito representa?
2. Qual é o paralelo comum em linguagens escritas em inglês?
3. Qual será a única forma canônica no Leviata?
4. Por que ela é melhor para o Leviata?
5. A implementação já suporta essa forma ou é apenas proposta?
6. Há impacto de compatibilidade com código existente?

Não aceitar uma decisão apenas porque “Python faz assim” ou “C faz assim”.

---

# 19. Registro histórico

## 10 de outubro de 2026 — Documento inicial

O documento nasceu para organizar nomes que haviam crescido sem uma regra única no protótipo.

Problemas identificados:

```text
escrever / imprimir
retornar
quebrar
jogo_desenhar_...
entidade_...
```

Foi definida a regra **uma ideia, uma forma oficial**.

## 10 de outubro de 2026 — De guia de ensino para especificação

O arquivo deixou de ser uma apostila de “como ensinar programação” e passou a ser a referência para **como o Leviata entende e escreve código**.

Também foi registrado que a coluna em inglês é apenas um paralelo conceitual; Leviata não é uma tradução de outra linguagem.

## 10 de outubro de 2026 — Siglas curtas

Foi decidido permitir siglas quando o significado permanece óbvio e o ganho de escrita é real.

Principal decisão:

```text
texto -> txt
inteiro -> int
decimal -> dec
booleano -> bool
```

Palavras já claras como `se`, `para`, `escreva` e `leia` não seriam abreviadas artificialmente.

## 10 de outubro de 2026 — Matemática sem `mat.`

Foi rejeitada a ideia de exigir:

```text
mat.raiz(25)
```

porque `raiz(25)` já identifica a operação sem ambiguidade.

## 10 de outubro de 2026 — Recursos como objetos considerados

Foi considerada a ideia de `imagem(...)`, `som(...)` e `arquivo(...)` retornarem recursos com métodos. A proposta foi preservada como direção possível, mas não deveria ser documentada como recurso pronto antes de existir semântica real de objetos/métodos.

## 11 de outubro de 2026 — Leviata 3.0 implementa a escrita canônica

A especificação deixou de ser apenas planejamento e passou para o código.

Implementado:

```text
escreva
leia
retorne
pare
continue
para ... em ...
txt / int / dec / bool
tipo
API curta de listas
matemática canônica
nomes qualificados para APIs especializadas
teclas por nome
```

O compilador passou a gerar bytecode real para `pare`, `continue` e `para item em lista`.

A camada pública `leviata_canon` passou a esconder os aliases históricos do runtime.

Todos os exemplos e testes oficiais foram migrados para a escrita do Leviata 3.
