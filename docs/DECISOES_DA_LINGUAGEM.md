# Leviata — Especificação Canônica da Linguagem

> Documento histórico e normativo da sintaxe e da biblioteca base do Leviata.
>
> Este arquivo define **como o Leviata deve entender e escrever código** após a refatoração. Ele não descreve necessariamente a implementação atual.
>
> Toda mudança futura de palavra-chave, operador, tipo, sintaxe ou nome canônico deve ser registrada aqui com seu equivalente conceitual comum e o motivo da decisão.

**Criado:** 10 de outubro de 2026  
**Estado:** especificação para refatoração  
**Idioma:** português brasileiro  

---

# 1. Identidade

Leviata não é Python traduzido, JavaScript traduzido, C traduzido nem Portugol moderno.

Leviata é uma linguagem própria, escrita em português, que usa conceitos universais de programação com uma sintaxe pequena, consistente e previsível.

A coluna ou referência em inglês existe somente para indicar o **paralelo conceitual** de algo conhecido em outras linguagens.

A regra principal é:

> **Uma ideia, uma forma oficial.**

O programador não deve decorar vários nomes para a mesma operação.

Exemplo:

```leviata
escreva("Ola")
```

É a forma canônica de saída. Não existirão como formas oficiais concorrentes:

```text
print
printf
console.log
imprimir
escrever
```

Aliases antigos podem existir temporariamente para migração, mas não pertencem à linguagem final.

---

# 2. Filosofia de escrita

O Leviata deve ser:

- curto sem virar código criptográfico;
- legível sem transformar cada linha em uma frase enorme;
- consistente;
- fácil de lembrar;
- capaz de crescer para projetos grandes;
- simples para iniciantes sem limitar usuários avançados.

Não devemos adicionar palavras apenas por organização estética.

Preferir:

```leviata
raiz(25)
```

em vez de:

```text
mat.raiz(25)
```

quando `raiz` já deixa claro o que a operação faz.

---

# 3. Regra de nomes

Existem três formas preferenciais de expor funcionalidade.

## 3.1 Função direta

Usar quando o nome sozinho já é claro e não cria ambiguidade relevante.

```leviata
raiz(25)
seno(angulo)
max(10, 20)
tamanho(lista)
aleatorio()
```

Não criar prefixo ou namespace sem necessidade.

---

## 3.2 Objeto + método

Usar quando existe um recurso ou estado que continuará sendo manipulado.

```leviata
var foto = imagem("jogador.png")
foto.desenhe(100, 80)

var tiro = som("tiro.wav")
tiro.toque()

var dados = arquivo("dados.txt")
dados.escreva("Ola")
```

A operação pertence naturalmente ao objeto.

Isso evita APIs como:

```text
img_carregar
img_desenhar
som_carregar
som_tocar
arquivo_escrever
```

---

## 3.3 Namespace ou módulo

Usar somente quando um domínio realmente possuir muitas operações globais relacionadas e o prefixo evitar ambiguidades reais.

Namespace **não é a regra padrão**.

Não devemos criar automaticamente:

```text
mat.
img.
arq.
som.
```

apenas para agrupar funções.

---

# 4. Siglas e abreviações

Siglas são permitidas quando forem:

1. fáceis de reconhecer;
2. muito usadas;
3. menores que o nome completo;
4. pouco ambíguas.

Formas canônicas:

```text
txt   texto
int   inteiro
dec   decimal
bool  booleano
var   variavel
const constante
```

Não abreviar palavras que já são pequenas e claras.

Continuam completas:

```text
se
senao
para
enquanto
funcao
retorne
escreva
leia
pare
```

Evitar abreviações obscuras como:

```text
fnc
escr
l
pq
rt
```

---

# 5. Palavras reservadas sem acentos

Palavras reservadas e nomes canônicos da API evitam acentos.

```text
funcao
senao
nao
ate
```

Motivos:

- digitação simples em qualquer teclado;
- menos problemas de codificação;
- melhor compatibilidade com ferramentas;
- identificadores previsíveis;
- portabilidade.

Strings continuam usando português normal:

```leviata
escreva("Você está programando em português.")
```

---

# 6. Blocos e símbolos

Blocos usam `{ }`.

```leviata
se idade >= 18 {
    escreva("Adulto")
}
```

Não são necessárias palavras como:

```text
entao
faca
fimse
fimenquanto
fimfuncao
```

Símbolos matemáticos e relacionais permanecem universais:

```text
+  -  *  /  %
=  ==  !=
<  >  <=  >=
```

---

# 7. Vocabulário fundamental

| Conceito | Inglês comum | Leviata | Motivo |
|---|---|---|---|
| saída | `print` | `escreva` | comando direto e conhecido em algoritmos em português |
| entrada | `input`, `read` | `leia` | complementar a `escreva` |
| variável | `var`, `let` | `var` | abreviação curta e clara |
| constante | `const` | `const` | curta e clara |
| condição | `if` | `se` | equivalente natural |
| alternativa | `else` | `senao` | equivalente natural |
| repetição condicional | `while` | `enquanto` | descreve a condição de repetição |
| repetição | `for` | `para` | sintaxe própria do Leviata |
| item de coleção | `in` | `em` | `para item em lista` |
| início de faixa | `from` | `de` | `para n de 1 ate 10` |
| fim de faixa | `to` | `ate` | completa a faixa |
| função | `function`, `def`, `fn` | `funcao` | termo técnico em português |
| retorno | `return` | `retorne` | comando direto |
| interromper laço | `break` | `pare` | descreve o efeito real |
| próxima iteração | `continue` | `continue` | forma imperativa compreensível |
| verdadeiro | `true` | `verdadeiro` | explícito |
| falso | `false` | `falso` | explícito |
| valor ausente | `null`, `nil`, `None` | `nulo` | um único valor nulo |
| E lógico | `and`, `&&` | `e` | leitura natural |
| OU lógico | `or`, `||` | `ou` | leitura natural |
| negação | `not`, `!` | `nao` | leitura natural |
| dependência | `import`, `use` | `usar` | palavra própria do Leviata |
| classe | `class` | `classe` | termo técnico em português |
| novo objeto | `new` | `novo` | curto |
| objeto atual | `this`, `self` | `este` | referência ao próprio objeto |
| herança | `extends`, `inherit` | `herda` | leitura direta |

---

# 8. Declarações e valores

Variável:

```leviata
var nome = "Ana"
var idade = 20
```

Constante:

```leviata
const limite = 10
```

Valores básicos:

```leviata
10
3.14
"Leviata"
verdadeiro
falso
nulo
[1, 2, 3]
```

---

# 9. Entrada e saída

## `print` -> `escreva`

```leviata
escreva("Ola")
escreva("Idade:", idade)
```

Decisão:

```text
print     -> escreva
printf    -> escreva
imprimir  -> escreva
escrever  -> escreva
```

Somente `escreva` é canônico.

## `input/read` -> `leia`

```leviata
var nome = leia("Nome: ")
```

`leia` retorna a entrada recebida. Conversão é feita explicitamente quando necessária.

```leviata
var idade = int(leia("Idade: "))
```

---

# 10. Tipos e conversões curtas

A forma canônica usa siglas reconhecíveis.

| Conceito | Inglês comum | Leviata |
|---|---|---|
| texto | `str`, `String` | `txt` |
| inteiro | `int` | `int` |
| decimal | `float`, `double` | `dec` |
| booleano | `bool`, `Boolean` | `bool` |

Conversões:

```leviata
var a = txt(123)
var b = int("42")
var c = dec("3.14")
var d = bool(1)
```

Substituições planejadas:

```text
para_texto    -> txt
texto         -> txt
para_inteiro  -> int
inteiro       -> int
para_decimal  -> dec
decimal       -> dec
logico        -> bool
```

O nome curto é escolhido porque conversões aparecem com frequência e o significado permanece claro.

---

# 11. Condicionais

```leviata
se idade >= 18 {
    escreva("Adulto")
} senao {
    escreva("Menor")
}
```

Encadeamento:

```leviata
se nota >= 7 {
    escreva("Aprovado")
} senao se nota >= 5 {
    escreva("Recuperacao")
} senao {
    escreva("Reprovado")
}
```

`entao` não existe. `{` já inicia o bloco.

---

# 12. Operadores lógicos

Forma canônica:

```text
and / && -> e
or  / || -> ou
not / !  -> nao
```

```leviata
se idade >= 18 e possui_documento {
    escreva("Entrada permitida")
}
```

A sintaxe oficial usa `e`, `ou` e `nao`.

---

# 13. Repetição

Enquanto:

```leviata
enquanto conectado {
    atualizar()
}
```

Faixa numérica:

```leviata
para n de 1 ate 10 {
    escreva(n)
}
```

`ate` é inclusivo por padrão.

Coleção:

```leviata
para fruta em frutas {
    escreva(fruta)
}
```

Interrupção:

```leviata
se terminou {
    pare
}
```

Próxima iteração:

```leviata
se ignorar {
    continue
}
```

---

# 14. Funções

```leviata
funcao soma(a, b) {
    retorne a + b
}

var resultado = soma(5, 3)
```

Equivalências:

```text
function / def / fn -> funcao
return              -> retorne
```

Funções sem retorno explícito retornam `nulo`.

---

# 15. Biblioteca base global

Funções pequenas e autoexplicativas podem ficar diretamente no escopo padrão.

Geral:

```text
tamanho(valor)
tipo(valor)
```

Texto:

```text
maiusculas(txt)
minusculas(txt)
contem(valor, procura)
comeca_com(txt, inicio)
termina_com(txt, fim)
trecho(txt, inicio, fim)
```

Listas:

```text
adicionar(lista, valor)
remover(lista, valor)
inserir(lista, indice, valor)
limpar(lista)
tamanho(lista)
```

O prefixo `lista_` é desnecessário quando os argumentos tornam a operação clara.

Acesso por índice:

```leviata
nomes[0]
```

---

# 16. Matemática

Matemática básica permanece global porque os nomes já são autoexplicativos.

Forma canônica:

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

Exemplos:

```leviata
var lado = raiz(25)
var maior = max(10, 30)
var angulo = seno(90)
```

Não usar:

```text
mat.raiz()
mat.seno()
mat.max()
```

porque `mat` acrescentaria algo que o nome da função já informa.

Alguns nomes foram encurtados porque são convenções matemáticas/programacionais muito reconhecíveis:

```text
absoluto          -> abs
cosseno           -> cos
tangente          -> tan
logaritmo         -> log
maximo            -> max
minimo            -> min
aleatorio_inteiro -> aleatorio_int
```

---

# 17. Recursos como objetos

Quando uma operação manipula continuamente um recurso, a API preferida é criar um objeto e operar nele.

## Imagem

```leviata
var jogador = imagem("jogador.png")

jogador.desenhe(100, 80)
escreva(jogador.largura)
escreva(jogador.altura)
```

Não criar um namespace `img` apenas por abreviação.

Evitar:

```text
img.carregar(...)
img.desenhar(...)
jogo_desenhar_sprite(...)
```

## Som

```leviata
var tiro = som("tiro.wav")

tiro.toque()
tiro.volume(0.5)
```

Evitar:

```text
som_carregar(...)
som_tocar(...)
som_definir_volume(...)
```

## Arquivo

```leviata
var dados = arquivo("dados.txt")

var conteudo = dados.leia()
dados.escreva("Ola")
dados.adicione("Nova linha")
```

Não usar `arq` apenas para economizar três letras quando `arquivo` aparece principalmente na criação do objeto.

Evitar:

```text
arq.ler(...)
ler_arquivo(...)
escrever_arquivo(...)
adicionar_arquivo(...)
```

---

# 18. Acesso a membros

O ponto `.` acessa propriedade ou comportamento pertencente a um objeto.

```leviata
pessoa.nome
foto.largura
foto.desenhe(x, y)
tiro.toque()
```

Regra:

```text
<objeto>.<membro>
```

O ponto não significa automaticamente “namespace”. Seu uso principal é expressar pertencimento.

---

# 19. Módulos e dependências

`usar` representa dependência externa ou biblioteca que não faça parte do ambiente base.

```leviata
usar rede
usar json
```

A forma exata de importação, resolução de pacote e visibilidade será definida separadamente.

Importante: importar um módulo não obriga toda API do Leviata a usar `modulo.acao()`.

---

# 20. Classes e objetos

Paralelos conceituais:

```text
class      -> classe
new        -> novo
this/self  -> este
extends    -> herda
```

```leviata
classe Pessoa {
    funcao falar() {
        escreva(este.nome)
    }
}

classe Aluno herda Pessoa {
}

var aluno = novo Aluno()
```

---

# 21. Comentários e pontuação

Linha:

```leviata
// comentario
```

Bloco:

```leviata
/* comentario */
```

Ponto e vírgula não é obrigatório no final da instrução.

```leviata
var nome = "Ana"
escreva(nome)
```

Vírgulas separam argumentos e elementos.

```leviata
soma(10, 20)
[1, 2, 3]
```

---

# 22. Forma canônica resumida

Palavras principais:

```text
var
const

se
senao

enquanto
para
em
de
ate

funcao
retorne
pare
continue

verdadeiro
falso
nulo

e
ou
nao

usar

classe
novo
este
herda
```

Entrada e saída:

```text
escreva()
leia()
```

Tipos/conversões:

```text
txt()
int()
dec()
bool()
```

Biblioteca base:

```text
tamanho()
tipo()
adicionar()
remover()
raiz()
abs()
seno()
cos()
tan()
log()
max()
min()
aleatorio()
```

Recursos:

```text
imagem("arquivo.png")
som("arquivo.wav")
arquivo("arquivo.txt")
```

Depois da criação, operações pertencem ao próprio recurso:

```text
foto.desenhe()
tiro.toque()
dados.leia()
```

---

# 23. Paralelo semântico resumido

```text
CONCEITO            INGLÊS COMUM       LEVIATA
-------------------------------------------------
saída               print              escreva
entrada             input/read         leia
variável            var/let            var
constante           const              const
condição             if                 se
alternativa          else               senao
laço condicional     while              enquanto
laço                 for                para
membro               in                 em
função               function/def/fn    funcao
retorno              return             retorne
parar laço           break              pare
próxima iteração     continue           continue
verdadeiro           true               verdadeiro
falso                false              falso
nulo                 null/nil/None      nulo
E lógico             and/&&             e
OU lógico            or/||              ou
negação              not/!              nao
texto/conversão      str/String         txt
inteiro/conversão    int                int
decimal/conversão    float/double       dec
booleano/conversão   bool/Boolean       bool
módulo               import/use         usar
classe               class              classe
novo objeto          new                novo
objeto atual         this/self          este
herança              extends/inherit    herda
```

---

# 24. O que não define o Leviata

Leviata não deve ser descrito como:

- Python em português;
- C em português;
- JavaScript em português;
- Portugol com mais recursos.

Sua identidade vem das próprias decisões:

- português brasileiro;
- sintaxe compacta;
- blocos com `{ }`;
- ausência de palavras redundantes como `entao` e `fimse`;
- símbolos matemáticos universais;
- operadores lógicos `e`, `ou`, `nao`;
- uma forma canônica por conceito;
- siglas somente quando forem realmente claras;
- funções globais quando o nome já explica a operação;
- objetos quando existe um recurso com estado;
- namespace somente quando realmente necessário;
- ausência de prefixos globais gigantes.

---

# 25. Política para novas APIs

Antes de criar qualquer nome novo, responder nesta ordem:

1. O nome da função sozinho já deixa a operação clara?
   - Sim: usar função direta.
2. A operação pertence a um recurso ou objeto específico?
   - Sim: usar método do objeto.
3. Existe ambiguidade real entre várias operações globais de um mesmo domínio?
   - Sim: considerar namespace/módulo.
4. A abreviação é realmente conhecida e mais clara que o nome completo?
   - Sim: pode usar sigla.
5. O novo nome duplica algo que já existe?
   - Sim: não criar outro nome.

Exemplos:

```text
raiz(25)                   correto
mat.raiz(25)               redundante

var foto = imagem("a.png") correto
foto.desenhe(10, 20)       correto
img.desenhar(foto, 10, 20) desnecessario

escreva("Oi")              correto
escrever("Oi")             alias antigo, nao canonico
imprimir("Oi")             nao canonico
```

---

# 26. Registro histórico

## 10 de outubro de 2026 — Padronização inicial

Foi decidido interromper a expansão da API para primeiro uniformizar a linguagem.

Decisões iniciais:

```text
escrever / imprimir -> escreva
ler                  -> leia
retornar             -> retorne
quebrar              -> pare
importar              -> usar
fatiar                -> trecho
```

O documento passou a ser a referência para a futura refatoração do lexer, parser, compilador, VM e biblioteca padrão.

## 10 de outubro de 2026 — Documento redefinido como especificação

O arquivo deixou de ser tratado como material de ensino e passou a definir **como o Leviata deve entender e escrever código**.

Os equivalentes em inglês passaram a ser registrados apenas como paralelo semântico, e não como modelo de sintaxe.

## 10 de outubro de 2026 — Siglas, matemática global e recursos como objetos

Foi decidido que abreviações podem fazer parte oficialmente da linguagem quando forem claras e úteis.

Conversões canônicas:

```text
txt
int
dec
bool
```

Foi removida a ideia de usar `mat.` para matemática. Funções autoexplicativas permanecem globais:

```text
raiz()
seno()
cos()
max()
min()
```

Também foi abandonada a regra universal `modulo.acao()`.

A regra atual é:

```text
função autoexplicativa -> global
recurso com estado      -> objeto.metodo()
namespace               -> somente se realmente necessário
```

Recursos como imagem, som e arquivo passam a ser pensados como objetos:

```leviata
var foto = imagem("a.png")
foto.desenhe(10, 20)

var efeito = som("efeito.wav")
efeito.toque()

var dados = arquivo("dados.txt")
dados.leia()
```

Essa decisão substitui propostas anteriores como `mat.raiz`, `img.carregar`, `arq.ler` e a obrigação geral de `modulo.acao()`.