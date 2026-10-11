# Leviata — Especificação Canônica da Linguagem

> Documento histórico e normativo da sintaxe do Leviata.
>
> Este arquivo define **como o Leviata deve entender código** e qual será a forma oficial da linguagem após a refatoração. Ele não descreve necessariamente o comportamento da implementação atual.
>
> Toda mudança futura de palavra-chave, operador, sintaxe ou nome da biblioteca base deve ser registrada aqui com seu equivalente conceitual comum em linguagens de programação e o motivo da decisão.

**Criado:** 10 de outubro de 2026  
**Estado:** especificação para refatoração  
**Idioma da linguagem:** português brasileiro, sem acentos nas palavras reservadas  

---

# 1. Objetivo

Leviata não é Python traduzido, JavaScript traduzido, C traduzido nem Portugol moderno.

Leviata é uma linguagem própria, escrita em português, que usa conceitos universais de programação com uma sintaxe pequena, consistente e previsível.

A regra principal é:

> **Uma ideia, uma forma oficial.**

O programador não deve precisar memorizar vários nomes para a mesma coisa.

Exemplo: saída de texto possui uma única forma canônica:

```leviata
escreva("Ola")
```

Não serão formas oficiais concorrentes:

```text
print
printf
console.log
imprimir
escrever
```

Aliases antigos poderão existir apenas durante migração, mas não fazem parte da especificação final.

---

# 2. Convenções gerais

## 2.1 Palavras reservadas sem acentos

A linguagem usa palavras em português, mas palavras reservadas não possuem acentos.

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
- código portátil.

Isso não significa remover acentos de textos ou strings.

```leviata
escreva("Você está programando em português.")
```

---

## 2.2 Blocos usam `{ }`

Leviata usa chaves para delimitar blocos.

```leviata
se idade >= 18 {
    escreva("Adulto")
}
```

Não serão necessárias palavras extras como:

```text
entao
faca
fimse
fimenquanto
fimfuncao
```

As chaves já possuem essa função.

---

## 2.3 Símbolos matemáticos e relacionais permanecem universais

Leviata não traduz símbolos que já são universais em programação.

```text
+   soma
-   subtracao
*   multiplicacao
/   divisao
%   resto
=   atribuicao
==  igualdade
!=  diferente
<   menor
>   maior
<=  menor ou igual
>=  maior ou igual
```

Isso mantém o código compacto e facilita a transição para outras linguagens.

---

# 3. Vocabulário fundamental

Esta é a tabela principal de equivalência entre conceitos normalmente encontrados em linguagens escritas em inglês e o Leviata.

| Conceito | Forma comum em inglês | Leviata | Motivo |
|---|---|---|---|
| saída | `print` | `escreva` | comando direto, reconhecível em português e sem ambiguidade |
| entrada | `input`, `read` | `leia` | forma complementar a `escreva` |
| variável | `var`, `let` | `var` | curto e imediatamente associado a variável |
| constante | `const` | `const` | curto, conhecido e derivado de constante |
| condição | `if` | `se` | equivalente natural em português |
| alternativa | `else` | `senao` | equivalente natural em português |
| repetição condicional | `while` | `enquanto` | representa diretamente a condição de repetição |
| repetição por faixa | `for` | `para` | permite uma construção própria do Leviata |
| membro de coleção | `in` | `em` | usado em `para item em lista` |
| início de faixa | `from` | `de` | usado em `para n de 1 ate 10` |
| fim de faixa | `to` | `ate` | completa a faixa numérica |
| função | `function`, `def`, `fn` | `funcao` | explícito e independente de outra linguagem |
| retorno | `return` | `retorne` | comando direto |
| interromper laço | `break` | `pare` | descreve o efeito real da instrução |
| próxima iteração | `continue` | `continue` | comando direto em português e conhecido |
| verdadeiro | `true` | `verdadeiro` | valor lógico explícito |
| falso | `false` | `falso` | valor lógico explícito |
| valor ausente | `null`, `nil`, `none` | `nulo` | um único valor nulo oficial |
| E lógico | `and`, `&&` | `e` | leitura natural |
| OU lógico | `or`, `||` | `ou` | leitura natural |
| negação lógica | `not`, `!` | `nao` | leitura natural |
| módulo | `import`, `use` | `usar` | indica dependência sem copiar sintaxe de uma linguagem específica |
| classe | `class` | `classe` | termo técnico já usado em português |
| instância nova | `new` | `novo` | curto e natural |
| objeto atual | `this`, `self` | `este` | referência ao próprio objeto |
| herança | `extends`, `inherits` | `herda` | forma direta: `classe Filho herda Pai` |

---

# 4. Declarações

## 4.1 Variável

Equivalentes conceituais:

```text
JavaScript: let idade = 20
Go:         var idade = 20
Leviata:    var idade = 20
```

Forma oficial:

```leviata
var idade = 20
```

`var` é parte da sintaxe do Leviata. Não significa que Leviata copie JavaScript ou Go.

---

## 4.2 Constante

Equivalentes conceituais:

```text
JavaScript: const limite = 10
Leviata:    const limite = 10
```

Forma oficial:

```leviata
const limite = 10
```

Uma constante não pode receber novo valor depois de criada.

---

# 5. Valores literais

## 5.1 Inteiro

```leviata
10
-20
5000
```

## 5.2 Decimal

```leviata
3.14
0.5
-12.75
```

## 5.3 Texto

```leviata
"Ola"
"Leviata"
```

## 5.4 Booleano

```leviata
verdadeiro
falso
```

Equivalentes:

```text
true  = verdadeiro
false = falso
```

## 5.5 Nulo

```leviata
nulo
```

Equivalentes conceituais:

```text
null
nil
None
```

Leviata possui somente `nulo`.

## 5.6 Lista

```leviata
[10, 20, 30]
["Ana", "Bia", "Caio"]
[]
```

---

# 6. Condicionais

Equivalente conceitual:

```text
if -> se
else -> senao
```

Forma Leviata:

```leviata
se idade >= 18 {
    escreva("Adulto")
} senao {
    escreva("Menor")
}
```

Condição encadeada:

```leviata
se nota >= 7 {
    escreva("Aprovado")
} senao se nota >= 5 {
    escreva("Recuperacao")
} senao {
    escreva("Reprovado")
}
```

`entao` não faz parte da sintaxe. A chave `{` já inicia o bloco.

---

# 7. Operadores lógicos

Forma oficial:

```text
and / && -> e
or  / || -> ou
not / !  -> nao
```

Exemplos de expressão válida:

```leviata
idade >= 18 e possui_documento
ativo ou administrador
nao bloqueado
```

A forma canônica da linguagem é textual: `e`, `ou`, `nao`.

Símbolos `&&`, `||` e `!` não são necessários na especificação final.

---

# 8. Repetição

## 8.1 Enquanto

Equivalente:

```text
while -> enquanto
```

Sintaxe:

```leviata
enquanto conectado {
    atualizar()
}
```

---

## 8.2 Para uma faixa numérica

Leviata possui sintaxe própria:

```leviata
para n de 1 ate 10 {
    escreva(n)
}
```

Equivalência conceitual aproximada:

```text
Python: for n in range(1, 11)
C:      for (n = 1; n <= 10; n++)
Leviata: para n de 1 ate 10
```

O Leviata não copia a sintaxe de nenhuma dessas linguagens.

`ate` é inclusivo por padrão: `1 ate 10` inclui o valor `10`.

---

## 8.3 Para cada item

```leviata
para fruta em frutas {
    escreva(fruta)
}
```

Equivalentes conceituais:

```text
for item in collection
foreach item in collection
```

No Leviata a forma é sempre:

```text
para <item> em <colecao>
```

---

## 8.4 Interromper

```text
break -> pare
```

```leviata
se terminou {
    pare
}
```

---

## 8.5 Próxima iteração

```text
continue -> continue
```

```leviata
se ignorar {
    continue
}
```

---

# 9. Funções

Equivalentes:

```text
function / def / fn -> funcao
return              -> retorne
```

Forma Leviata:

```leviata
funcao soma(a, b) {
    retorne a + b
}
```

Chamada:

```leviata
var resultado = soma(5, 3)
```

Funções sem valor retornam `nulo` implicitamente.

---

# 10. Entrada e saída

## 10.1 `print` -> `escreva`

Forma oficial:

```leviata
escreva("Ola")
```

Motivo da escolha:

- é curto;
- funciona como comando;
- não sugere impressora física tanto quanto `imprimir`;
- é familiar no ensino brasileiro de algoritmos;
- substitui a confusão entre `escrever`, `imprimir` e outros aliases.

Forma antiga ou externa:

```text
print     -> escreva
printf    -> escreva
imprimir  -> escreva
escrever  -> escreva
```

Somente `escreva` é canônico.

---

## 10.2 `input` / `read` -> `leia`

Forma oficial:

```leviata
var nome = leia("Nome: ")
```

`leia` retorna o valor fornecido pela entrada padrão.

Conversão de tipo é explícita quando necessária.

```leviata
var idade = inteiro(leia("Idade: "))
```

---

# 11. Conversão de tipos

A forma canônica usa o nome do tipo desejado como função.

| Conceito | Formas comuns | Leviata |
|---|---|---|
| converter para texto | `str`, `String`, `toString` | `texto(valor)` |
| converter para inteiro | `int`, `parseInt` | `inteiro(valor)` |
| converter para decimal | `float`, `double`, `parseFloat` | `decimal(valor)` |
| converter para lógico | `bool`, `Boolean` | `logico(valor)` |

Formas antigas como:

```text
para_texto
para_inteiro
para_decimal
```

serão substituídas pelas formas curtas.

---

# 12. Biblioteca base

A biblioteca base deve possuir nomes pequenos e genéricos somente quando não houver ambiguidade importante.

## 12.1 Geral

```text
tamanho(valor)
tipo(valor)
```

Equivalentes conceituais:

```text
len / length -> tamanho
type / typeof -> tipo
```

---

## 12.2 Texto

Forma canônica:

```text
maiusculas(texto)
minusculas(texto)
contem(texto, valor)
comeca_com(texto, valor)
termina_com(texto, valor)
trecho(texto, inicio, fim)
```

Equivalentes conceituais:

| Inglês comum | Leviata |
|---|---|
| `upper`, `toUpperCase` | `maiusculas` |
| `lower`, `toLowerCase` | `minusculas` |
| `contains`, `includes` | `contem` |
| `startsWith` | `comeca_com` |
| `endsWith` | `termina_com` |
| `slice`, `substring` | `trecho` |

`trecho` é escolhido em vez de `fatiar` porque descreve o resultado sem importar o jargão inglês “slice”.

---

## 12.3 Listas

Forma canônica:

```text
adicionar(lista, valor)
remover(lista, valor)
inserir(lista, indice, valor)
limpar(lista)
tamanho(lista)
```

O prefixo `lista_` não deve existir quando o primeiro argumento já deixa claro que a operação atua sobre uma lista.

Portanto:

```text
lista_adicionar(lista, valor) -> adicionar(lista, valor)
lista_remover(lista, valor)   -> remover(lista, valor)
```

Isso reduz nomes sem remover significado.

Acesso por índice permanece universal:

```leviata
nomes[0]
```

---

## 12.4 Matemática

Forma canônica:

```text
raiz(x)
potencia(base, expoente)
absoluto(x)
arredondar(x)
teto(x)
piso(x)
seno(x)
cosseno(x)
tangente(x)
logaritmo(x)
maximo(a, b)
minimo(a, b)
aleatorio()
aleatorio_inteiro(minimo, maximo)
pi()
```

A matemática usa os nomes já conhecidos em português. Não há motivo para criar traduções artificiais.

---

# 13. Arquivos

Arquivos pertencem à biblioteca padrão, não à gramática da linguagem.

Forma canônica proposta:

```text
arquivo.ler(caminho)
arquivo.escrever(caminho, conteudo)
arquivo.adicionar(caminho, conteudo)
```

Em vez de espalhar nomes globais como:

```text
ler_arquivo
escrever_arquivo
adicionar_arquivo
```

Motivo: operações especializadas devem ser agrupadas por domínio. Isso reduz colisões de nomes e facilita descoberta sem criar nomes gigantes.

---

# 14. Módulos

Equivalente conceitual:

```text
import / use -> usar
```

Forma Leviata:

```leviata
usar matematica
usar arquivo
```

Quando um módulo expõe um conjunto especializado de operações, o acesso preferencial é por ponto:

```leviata
arquivo.ler("dados.txt")
```

Essa regra é importante para impedir funções globais enormes e difíceis de memorizar.

---

# 15. Classes e objetos

O modelo canônico reservado para orientação a objetos é:

```text
class      -> classe
new        -> novo
this/self  -> este
extends    -> herda
```

Sintaxe:

```leviata
classe Pessoa {
    funcao falar() {
        escreva(este.nome)
    }
}
```

Herança:

```leviata
classe Aluno herda Pessoa {
}
```

Instância:

```leviata
var aluno = novo Aluno()
```

Esses termos pertencem ao Leviata; não pretendem reproduzir a sintaxe completa de Java, Python ou JavaScript.

---

# 16. Acesso a membros

O ponto `.` é o padrão para acessar membros de objetos, módulos e namespaces.

```leviata
pessoa.nome
arquivo.ler("dados.txt")
camera.seguir(x, y)
```

A regra é:

```text
<objeto-ou-modulo>.<membro>
```

Isso substitui APIs globais excessivamente prefixadas.

Exemplo de direção de refatoração:

```text
jogo_camera_seguir(x, y) -> camera.seguir(x, y)
jogo_desenhar_texto(...) -> desenho.texto(...)
entidade_criar(...)       -> entidade.criar(...)
```

Esses exemplos definem **a convenção de nomes**, não obrigam o núcleo da linguagem a conhecer jogos, câmera ou entidades.

---

# 17. Regra para bibliotecas especializadas

O núcleo do Leviata deve permanecer pequeno.

Recursos especializados são bibliotecas.

Exemplos de domínios:

```text
arquivo
rede
json
tempo
jogo
desenho
camera
audio
teclado
mouse
mapa
entidade
fisica
```

A regra para novas APIs é:

```text
modulo.acao(...)
```

Preferir:

```text
camera.seguir(x, y)
desenho.retangulo(...)
audio.tocar("som.wav")
```

Evitar:

```text
jogo_camera_seguir(...)
jogo_desenhar_retangulo(...)
jogo_tocar_som(...)
```

O objetivo não é deixar o código parecido com Python. O objetivo é possuir uma organização consistente própria.

---

# 18. Identificadores de entrada

APIs públicas não devem obrigar o programador a memorizar números mágicos quando existir um nome natural.

Exemplo:

```text
tecla 65 -> "A"
tecla 32 -> "espaco"
```

Uma biblioteca de entrada deve aceitar nomes legíveis:

```leviata
teclado.pressionada("A")
teclado.apertada("espaco")
```

Os códigos numéricos podem existir internamente, mas não são parte da interface canônica destinada ao programador.

---

# 19. Comentários

Comentários de uma linha:

```leviata
// comentario
```

Comentários de bloco:

```leviata
/*
   comentario
   de varias linhas
*/
```

Esses símbolos são amplamente reconhecidos e não precisam ser traduzidos.

---

# 20. Pontuação

O ponto e vírgula não é obrigatório no final de uma instrução.

Forma preferida:

```leviata
var nome = "Ana"
escreva(nome)
```

A quebra de linha normalmente encerra a instrução quando a expressão está completa.

Vírgula separa argumentos e elementos:

```leviata
soma(10, 20)
[1, 2, 3]
```

Parênteses agrupam expressões e chamadas:

```leviata
(2 + 3) * 4
soma(2, 3)
```

---

# 21. Forma canônica resumida

## Palavras reservadas principais

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

## Funções fundamentais

```text
escreva()
leia()

tamanho()
tipo()

texto()
inteiro()
decimal()
logico()
```

## Convenção para bibliotecas

```text
modulo.acao()
```

---

# 22. Exemplo de equivalência geral

O objetivo desta seção não é ensinar uma linguagem estrangeira. Ela existe para registrar a correspondência semântica usada pela implementação e pela documentação do Leviata.

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
laço                  for                para
membro               in                 em
função                function/def       funcao
retorno               return             retorne
parar laço            break              pare
próxima iteração      continue           continue
verdadeiro            true               verdadeiro
falso                 false              falso
nulo                  null/nil/None      nulo
E lógico              and/&&             e
OU lógico             or/||              ou
negação               not/!              nao
módulo                import/use         usar
classe                class              classe
novo objeto           new                novo
objeto atual          this/self          este
herança               extends/inherit    herda
```

---

# 23. O que não define o Leviata

Leviata não deve ser descrito como:

- “Python em português”;
- “C em português”;
- “JavaScript em português”;
- “Portugol com mais recursos”.

Essas linguagens podem possuir conceitos equivalentes porque programação compartilha conceitos universais.

A identidade do Leviata vem do conjunto de decisões:

- português brasileiro;
- sintaxe compacta;
- blocos com `{ }`;
- sem palavras redundantes como `entao` e `fimse`;
- operadores matemáticos universais;
- operadores lógicos legíveis (`e`, `ou`, `nao`);
- uma forma canônica por conceito;
- biblioteca principal pequena;
- recursos especializados organizados por `modulo.acao`;
- nomes curtos, previsíveis e sem prefixos repetitivos.

---

# 24. Política de mudanças futuras

Toda alteração de sintaxe ou API canônica deve responder quatro perguntas:

1. **Qual conceito isso representa?**
2. **Como esse conceito costuma aparecer em linguagens escritas em inglês?**
3. **Qual será a forma oficial no Leviata?**
4. **Por que essa forma é melhor para o Leviata?**

Uma mudança não deve ser aceita apenas porque “Python faz assim”, “C faz assim” ou “JavaScript faz assim”.

A decisão precisa fazer sentido dentro do próprio Leviata.

---

# 25. Registro histórico

## 10 de outubro de 2026 — Padronização inicial

Foi decidido interromper a expansão da API para primeiro uniformizar a linguagem.

Principais decisões:

```text
escrever / imprimir -> escreva
ler                  -> leia
retornar             -> retorne
quebrar              -> pare
importar              -> usar
para_texto            -> texto
para_inteiro          -> inteiro
para_decimal          -> decimal
fatiar                -> trecho
```

Também foi definida a regra de que APIs especializadas deixam de crescer por prefixos globais e passam a seguir:

```text
modulo.acao()
```

Exemplo:

```text
jogo_camera_seguir -> camera.seguir
jogo_desenhar_texto -> desenho.texto
entidade_criar -> entidade.criar
```

## 10 de outubro de 2026 — Documento redefinido como especificação

O documento deixou de ser tratado como guia de ensino e passou a ser a **referência canônica para a refatoração do lexer, parser, compilador, VM e biblioteca padrão do Leviata**.

A coluna em inglês existe apenas como paralelo semântico. Ela não define a sintaxe do Leviata e não implica que a linguagem seja tradução de Python ou de qualquer outra linguagem.
