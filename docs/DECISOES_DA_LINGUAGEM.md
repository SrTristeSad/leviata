# Leviata — Especificação Canônica e Registro Histórico

> Documento normativo da escrita pública do Leviata.
>
> Mudanças de sintaxe, palavras-chave e APIs públicas devem ser registradas aqui com o paralelo conceitual e o motivo da decisão.

**Criado:** 10 de outubro de 2026  
**Última revisão:** 11 de outubro de 2026  
**Versão implementada:** Leviata 3.2  

---

# 1. Identidade

Leviata é uma linguagem própria escrita em português brasileiro. Não é Python traduzido, C traduzido, JavaScript traduzido nem Portugol ampliado.

Conceitos como variável, condição, função, classe e laço são universais. Referências a palavras de outras linguagens neste documento servem apenas como paralelo semântico.

Regra principal:

> **Uma ideia, uma forma oficial.**

Não devem existir várias formas públicas concorrentes para a mesma operação.

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

Por isso não existem palavras redundantes como `entao`, `faca`, `fimse` ou `fimfuncao`.

Palavras reservadas e APIs canônicas evitam acentos:

```text
funcao
senao
nao
ate
```

Strings continuam usando português normal.

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

Formas antigas removidas:

```text
retornar  -> retorne
quebrar   -> pare
continuar -> continue
importar  -> usar
herdar    -> herda
```

`&&`, `||` e `!` não são operadores lógicos canônicos. `!=` continua sendo o operador de diferença.

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

Conversões frequentes usam formas curtas e reconhecíveis:

```text
txt  texto
int  inteiro
dec  decimal
bool booleano
```

```leviata
var a = txt(123)
var b = int("42")
var c = dec("3.14")
var d = bool(1)
```

`tipo(valor)` usa nomes canônicos. Em 3.2 ele também reconhece recursos, classes e instâncias.

---

# 5. Condições e lógica

```leviata
se idade >= 18 e possui_documento {
    escreva("Entrada permitida")
} senao {
    escreva("Entrada negada")
}
```

```leviata
se nao bloqueado {
    escreva("Pode continuar")
}
```

`e` e `ou` possuem short-circuit real: o lado direito só é avaliado quando necessário.

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
para n de 1 ate 100 {
    se n == 3 {
        continue
    }

    se n == 10 {
        pare
    }
}
```

`pare` e `continue` geram saltos reais no bytecode e fazem unwind das variáveis locais dos blocos abandonados.

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

escreva(tamanho(nomes))
escreva(contem(nomes, "Ana"))
```

O prefixo histórico `lista_` não faz parte da API pública.

---

# 9. Texto

API canônica:

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

`contem` é genérico e também funciona com listas.

---

# 10. Matemática

Operações autoexplicativas ficam no escopo global. Não existe obrigação de escrever `mat.`.

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

`pi` é valor, não função.

---

# 11. APIs especializadas

APIs grandes usam nomes por domínio:

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

O compilador resolve uma cadeia qualificada conhecida como uma única chave da API pública.

---

# 12. Entrada de jogos

Códigos mágicos de tecla não fazem parte da interface recomendada.

```leviata
teclado.pressionada("A")
teclado.apertada("espaco")
teclado.pressionada("esquerda")
```

A implementação C pode continuar usando códigos numéricos internamente.

---

# 13. Objetos nativos — implementado no 3.1

A proposta de recursos como objetos deixou de ser apenas ideia no Leviata 3.1.

Imagem:

```leviata
var foto = imagem("jogador.png")
foto.desenhe(10, 20)
escreva(foto.caminho)
escreva(foto.largura)
escreva(foto.altura)
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

A forma histórica `arquivo.leia("dados.txt")` foi retirada da superfície pública. `arquivo(...)` é um construtor de recurso.

O compilador distingue:

- cadeia estática conhecida: `camera.seguir`;
- membro de um valor: `foto.desenhe`, `dados.caminho`.

Os handlers internos usam nomes iniciados por `@`, que não podem ser escritos como identificadores da linguagem.

---

# 14. Classes — implementado no 3.2

Forma canônica:

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
escreva(pessoa.saudacao())
```

Decisões:

- `classe` declara um tipo definido pelo usuário;
- `novo Nome()` cria uma instância;
- `este` é o receptor implícito do método;
- campos podem ser lidos e alterados por ponto;
- métodos são funções de bytecode normais com `este` como primeiro argumento implícito;
- classes são declaradas no escopo principal no 3.2;
- construtores com argumentos ainda não existem no 3.2.

Campos com lista padrão são clonados por instância para impedir compartilhamento acidental:

```leviata
classe Caixa {
    var itens = []
}
```

Duas instâncias de `Caixa` recebem listas diferentes.

---

# 15. Herança — implementada no 3.2

```leviata
classe Animal {
    var nome = "sem nome"

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
```

A classe filha:

- recebe campos do pai;
- encontra métodos herdados;
- pode adicionar novos campos e métodos;
- pode sobrescrever métodos do pai.

O lookup de método percorre filho -> pai.

`tipo(instancia)` retorna o nome concreto da classe, inclusive para classes filhas.

---

# 16. Modelo interno de objetos

Recursos nativos usam `LV_TYPE_OBJECT` com metadados leves dentro do objeto gerenciado pelo GC.

Classes e instâncias usam `LV_TYPE_MAP`, já rastreado pelo coletor.

Chaves internas começam com `@`:

```text
@tipo
@nome
@pai
@classe
@campo:<nome>
@metodo:<nome>
```

Elas não colidem com código do usuário porque `@` não inicia identificadores Leviata.

O despacho público de campos e métodos é compartilhado entre recursos nativos e instâncias definidas pelo usuário. Assim, o compilador não precisa codificar cada tipo de objeto.

---

# 17. O ponto (`.`)

O operador de ponto possui semântica contextual, mas previsível.

Se a cadeia completa existir como API estática registrada:

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

Isso permite manter APIs organizadas e, ao mesmo tempo, oferecer objetos reais.

---

# 18. Módulos — próximo recurso reservado

A palavra:

```text
usar
```

já é reservada como paralelo de `import`/`use`, mas módulos ainda não fazem parte da implementação funcional do 3.2.

A implementação futura deve definir explicitamente:

- como um arquivo é localizado;
- o que ele exporta;
- isolamento de nomes;
- ciclos de importação;
- execução única por módulo;
- relação entre módulo e nome qualificado.

Não documentar sintaxe de módulos como executável antes desse comportamento existir.

---

# 19. Separação entre implementação e superfície pública

A implementação C pode manter nomes históricos para reaproveitar código estável.

Internamente:

```text
jogo_desenhar_retangulo
```

Publicamente:

```leviata
desenho.retangulo(...)
```

As camadas canônicas removem aliases antigos da tabela global e expõem somente a escrita oficial.

Nome interno não é especificação da linguagem.

---

# 20. Política de mudanças

Toda mudança pública deve responder:

1. Qual conceito representa?
2. Qual é o paralelo comum em outras linguagens?
3. Qual será a única forma canônica no Leviata?
4. Por que ela é melhor para o Leviata?
5. A implementação já suporta essa forma ou é só proposta?
6. Qual o impacto de compatibilidade?

Não aceitar uma decisão apenas porque outra linguagem faz assim.

---

# 21. Registro histórico

## 10 de outubro de 2026 — Documento inicial

O documento nasceu para organizar nomes que cresceram sem regra única no protótipo.

Problemas identificados incluíam:

```text
escrever / imprimir
retornar
quebrar
jogo_desenhar_...
entidade_...
```

Foi definida a regra **uma ideia, uma forma oficial**.

## 10 de outubro de 2026 — Siglas curtas

Foram adotadas formas curtas quando reconhecíveis:

```text
texto    -> txt
inteiro  -> int
decimal  -> dec
booleano -> bool
```

Palavras já curtas como `se`, `para`, `escreva` e `leia` não seriam abreviadas artificialmente.

## 10 de outubro de 2026 — Matemática sem `mat.`

Foi rejeitada a obrigação de `mat.raiz(25)` porque `raiz(25)` já identifica a operação.

## 10 de outubro de 2026 — Recursos como objetos considerados

Foi registrada a ideia de `imagem(...)`, `som(...)` e `arquivo(...)` retornarem recursos com estado, mas decidiu-se não fingir esse comportamento usando aliases.

## 11 de outubro de 2026 — Leviata 3.0

A escrita canônica passou para lexer, parser, compilador, stdlib, exemplos e testes.

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

Recursos passaram a ser objetos reais:

```text
imagem(...)
som(...)
arquivo(...)
```

Foram implementados campos e métodos nativos, além de regressões específicas e testes com sanitizers.

## 11 de outubro de 2026 — Leviata 3.2

Foram implementados:

```text
classe
novo
este
herda
campos de instância
métodos de usuário
sobrescrita de métodos
herança de campos e métodos
```

Classes/instâncias passaram a usar mapas rastreados pelo GC. Métodos continuam sendo bytecode Leviata normal, com `este` como argumento implícito.

A suíte de regressão passou a validar estado independente entre instâncias, listas padrão clonadas, atribuição de campos, método chamando método, herança, override e sanitizers.
