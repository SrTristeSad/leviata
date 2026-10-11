# Leviata — Especificação Canônica e Registro Histórico

> Documento normativo da escrita pública do Leviata.
>
> Mudanças de sintaxe, palavras-chave e APIs públicas devem ser registradas aqui com o paralelo conceitual e o motivo da decisão.

**Criado:** 10 de outubro de 2026  
**Última revisão:** 11 de outubro de 2026  
**Versão implementada:** Leviata 3.3  

---

# 1. Identidade

Leviata é uma linguagem própria escrita em português brasileiro. Não é Python traduzido, C traduzido, JavaScript traduzido nem Portugol ampliado.

Conceitos como variável, condição, função, classe e módulo são universais. Referências a outras linguagens servem apenas como paralelo semântico.

Regra principal:

> **Uma ideia, uma forma oficial.**

```leviata
escreva("Ola")
```

é a forma oficial de saída. `print`, `imprimir`, `escrever` e `escreva` não coexistem como aliases públicos.

---

# 2. Regras de escrita

A linguagem deve ser:

- curta sem ser criptográfica;
- legível;
- previsível;
- consistente;
- simples para começar;
- capaz de crescer sem nomes globais gigantes.

Símbolos universais não são traduzidos:

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

Palavras reservadas e APIs canônicas evitam acentos. Strings continuam usando português normal.

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
| ausência de valor | `null`, `nil`, `None` | `nulo` |
| E lógico | `and`, `&&` | `e` |
| OU lógico | `or`, `||` | `ou` |
| negação | `not`, `!` | `nao` |
| classe | `class` | `classe` |
| nova instância | `new` | `novo` |
| objeto atual | `this`, `self` | `este` |
| herança | `extends`, `inherit` | `herda` |
| módulo/dependência | `import`, `use` | `usar` |

Formas históricas removidas:

```text
retornar  -> retorne
quebrar   -> pare
continuar -> continue
importar  -> usar
herdar    -> herda
```

`&&`, `||` e `!` não são operadores lógicos canônicos. `!=` continua sendo diferença.

---

# 4. Entrada, saída e conversões

```leviata
var nome = leia("Nome: ")
escreva("Ola", nome)
```

Entrada não converte tipo automaticamente:

```leviata
var idade = int(leia("Idade: "))
```

Conversões canônicas:

```text
txt  texto
int  inteiro
dec  decimal
bool booleano
```

`tipo(valor)` usa nomes canônicos e reconhece recursos, classes e instâncias.

---

# 5. Condições e lógica

```leviata
se idade >= 18 e possui_documento {
    escreva("Entrada permitida")
} senao {
    escreva("Entrada negada")
}
```

`e` e `ou` possuem short-circuit real.

---

# 6. Repetições

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

Controle:

```leviata
se ignorar { continue }
se terminou { pare }
```

`pare` e `continue` fazem unwind das variáveis locais dos blocos abandonados.

---

# 7. Funções

```leviata
funcao soma(a, b) {
    retorne a + b
}
```

Função sem `retorne` explícito devolve `nulo`.

---

# 8. Listas

```leviata
var nomes = ["Ana", "Bia"]
var vazia = lista()

adicionar(nomes, "Caio")
inserir(nomes, 1, "Duda")
remover(nomes, "Bia")
limpar(vazia)
```

O prefixo histórico `lista_` não faz parte da API pública.

---

# 9. Texto

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
comeca_com -> comeca
termina_com -> termina
fatiar     -> trecho
```

---

# 10. Matemática

Operações autoexplicativas ficam no escopo global:

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

`pi` é valor, não função.

---

# 11. APIs especializadas

APIs grandes usam domínio:

```leviata
jogo.iniciar(...)
desenho.retangulo(...)
camera.seguir(...)
entidade.atualizar(...)
```

Domínios implementados:

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
```

O compilador resolve uma cadeia qualificada conhecida como uma única chave pública.

---

# 12. Entrada de jogos

Códigos mágicos de tecla não fazem parte da interface recomendada:

```leviata
teclado.pressionada("A")
teclado.apertada("espaco")
teclado.pressionada("esquerda")
```

---

# 13. Objetos nativos — 3.1

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
dados.adicione("\nLinha")
escreva(dados.leia())
```

A forma histórica `arquivo.leia("dados.txt")` foi retirada da superfície pública.

---

# 14. Classes — 3.2

```leviata
classe Pessoa {
    var nome = "sem nome"

    funcao nomear(valor) {
        este.nome = valor
    }

    funcao saudacao() {
        retorne "Oi " + este.nome
    }
}

var pessoa = novo Pessoa()
pessoa.nomear("Ana")
```

Decisões:

- `classe` declara tipo definido pelo usuário;
- `novo Nome()` cria instância;
- `este` é receptor implícito;
- campos podem ser lidos e alterados por ponto;
- métodos são funções de bytecode normais com `este` como primeiro argumento implícito;
- classes ficam no escopo principal no 3.2/3.3;
- `novo Classe()` ainda não recebe argumentos.

Listas padrão são clonadas por instância.

---

# 15. Herança — 3.2

```leviata
classe Gato herda Animal {
    var vidas = 7

    funcao falar() {
        retorne este.nome + " disse miau"
    }
}
```

A classe filha herda campos e métodos, pode adicionar membros e sobrescrever métodos. O lookup percorre filho -> pai.

---

# 16. Modelo interno de objetos

Recursos nativos usam `LV_TYPE_OBJECT` com metadados leves.

Classes e instâncias usam `LV_TYPE_MAP`, rastreado pelo GC.

Chaves internas começam com `@` e não colidem com identificadores do usuário:

```text
@tipo
@nome
@pai
@classe
@campo:<nome>
@metodo:<nome>
```

O despacho de campos e métodos é compartilhado entre recursos nativos e instâncias de usuário.

---

# 17. O ponto (`.`)

Se a cadeia completa existir como API estática:

```leviata
camera.seguir(10, 20)
```

ela é resolvida como nome qualificado.

Caso contrário:

```leviata
objeto.campo
objeto.metodo()
```

é resolvido como membro do valor à esquerda.

---

# 18. Módulos — implementado no 3.3

Forma canônica:

```leviata
usar "modulos/util.pt"
```

Regras implementadas:

1. `usar` só é válido no escopo principal;
2. o caminho é relativo ao arquivo importador;
3. módulos podem importar outros módulos com o mesmo critério;
4. cada caminho normalizado é executado uma única vez por runtime;
5. imports repetidos não repetem efeitos de topo do módulo;
6. ciclos de dependência são detectados e abortam com erro;
7. módulos são resolvidos e executados antes da compilação do arquivo importador;
8. funções, classes e variáveis do módulo entram no namespace global compartilhado;
9. a extensão do arquivo deve ser escrita explicitamente no 3.3.

Exemplo:

```leviata
/* util.pt */
funcao dobro(x) {
    retorne x * 2
}
```

```leviata
/* principal.pt */
usar "util.pt"
escreva(dobro(10))
```

## Por que namespace global compartilhado primeiro?

O runtime atual executa todos os módulos na mesma VM. Renomear apenas símbolos exportados para fingir namespace isolado quebraria referências internas dos bytecodes dos módulos.

Portanto o 3.3 escolhe uma semântica pequena e verdadeira em vez de uma abstração incompleta.

Ainda não existem:

```text
exportar
usar ... como ...
namespace isolado por arquivo
visibilidade privada de módulo
```

Esses recursos só devem entrar quando referências internas, cache e exportação puderem ser tratados de forma consistente.

---

# 19. Carregamento de módulos

O runtime mantém dois conjuntos lógicos:

- módulos já carregados;
- pilha de módulos atualmente sendo carregados.

A pilha permite detectar:

```text
A.pt -> B.pt -> A.pt
```

antes que haja recursão infinita.

Caminhos normalizam `/`, `\\`, `.` e `..` para reduzir imports duplicados equivalentes.

O arquivo importado é parseado, seus próprios imports são resolvidos, depois ele é compilado e executado. Só então o importador é compilado.

Isso permite que o importador use classes e funções do módulo normalmente.

---

# 20. Separação entre implementação e superfície pública

A implementação C pode manter nomes históricos para reaproveitar código estável.

Internamente:

```text
jogo_desenhar_retangulo
```

Publicamente:

```leviata
desenho.retangulo(...)
```

Nome interno não é especificação da linguagem.

---

# 21. Política de mudanças

Toda mudança pública deve responder:

1. Qual conceito representa?
2. Qual é o paralelo comum?
3. Qual será a única forma canônica?
4. Por que ela é melhor para o Leviata?
5. A implementação já suporta essa forma ou é proposta?
6. Qual o impacto de compatibilidade?

---

# 22. Registro histórico

## 10 de outubro de 2026 — Documento inicial

A especificação nasceu para organizar nomes que haviam crescido sem regra única no protótipo.

Foi definida a regra **uma ideia, uma forma oficial**.

## 10 de outubro de 2026 — Siglas curtas

```text
texto    -> txt
inteiro  -> int
decimal  -> dec
booleano -> bool
```

## 10 de outubro de 2026 — Matemática sem `mat.`

Foi rejeitada a obrigação de `mat.raiz(25)` porque `raiz(25)` já identifica a operação.

## 11 de outubro de 2026 — Leviata 3.0

A escrita canônica passou para lexer, parser, compilador, biblioteca, exemplos e testes.

Implementado:

```text
escreva / leia
retorne
pare / continue
para ... em ...
e / ou / nao
txt / int / dec / bool
API curta de listas
matemática canônica
APIs qualificadas
teclas por nome
```

## 11 de outubro de 2026 — Leviata 3.1

`imagem`, `som` e `arquivo` passaram a ser objetos reais com campos e métodos.

## 11 de outubro de 2026 — Leviata 3.2

Foram implementados:

```text
classe
novo
este
herda
campos de instância
métodos de usuário
sobrescrita
herança
```

A suíte passou a validar estado independente, listas padrão clonadas, métodos herdados e sanitizers.

## 11 de outubro de 2026 — Leviata 3.3

`usar` deixou de ser apenas palavra reservada e passou a carregar módulos reais por arquivo.

Foram adicionados:

```text
resolução relativa ao importador
imports aninhados
cache de execução única
normalização de caminho
detecção de ciclo
rejeição de importar dentro de função/bloco
regressões de função e classe importadas
```

O namespace global compartilhado foi assumido explicitamente como semântica do 3.3, deixando exportação/alias/isolamento para uma evolução posterior completa.
