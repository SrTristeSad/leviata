# Leviata — Decisões da Linguagem e Biblioteca Canônica

> Documento histórico de design da linguagem.
>
> Este arquivo registra **como o Leviata deve ser escrito e ensinado**, mesmo quando a implementação atual ainda usa nomes diferentes.
> Mudanças futuras de sintaxe, palavras-chave ou biblioteca devem atualizar este documento e registrar o motivo no histórico ao final.

**Primeira versão deste documento:** 10 de outubro de 2026  
**Status:** especificação de direção da linguagem  
**Objetivo:** tornar o Leviata uniforme, fácil de ler, fácil de ensinar e fácil de aprender em português brasileiro.

---

## 1. Por que esta padronização existe

O Leviata começou crescendo por funcionalidades. Isso fez surgir nomes corretos isoladamente, mas nem sempre uniformes entre si.

Exemplos do problema:

```text
escrever(...)
imprimir(...)
retornar
quebrar
jogo_desenhar_retangulo(...)
entidade_atualizar_tilemap(...)
```

Nenhum desses nomes é necessariamente errado. O problema é que, juntos, eles obrigam o aluno a decorar estilos diferentes.

A partir desta especificação, o Leviata passa a seguir uma ideia central:

> **O código deve poder ser explicado em voz alta por um professor de lógica de programação sem parecer uma tradução artificial do inglês.**

O Leviata não será apenas uma linguagem existente com palavras traduzidas. Ele deverá parecer natural para quem está aprendendo programação em português.

---

# 2. Princípios da linguagem

## 2.1 Uma ideia deve ter um nome oficial

A documentação não deve ensinar vários nomes para a mesma operação.

Errado como padrão didático:

```text
escrever("Oi")
imprimir("Oi")
print("Oi")
```

Mesmo que aliases existam por compatibilidade, o material oficial ensinará somente:

```text
escreva("Oi")
```

Isso reduz a quantidade de coisas que o aluno precisa memorizar.

---

## 2.2 Comandos devem soar como instruções

Comandos principais usam preferencialmente a forma verbal de uma instrução:

```text
escreva
leia
retorne
pare
continue
```

A intenção é que um professor possa dizer:

> Leia o nome. Se a idade for maior que 18, escreva uma mensagem. Caso contrário, escreva outra mensagem.

E o código se pareça com essa explicação.

---

## 2.3 Estruturas de controle usam português natural

```text
se
senao
enquanto
para
em
de
ate
```

Exemplo:

```text
para numero de 1 ate 10 {
    escreva(numero)
}
```

Ou:

```text
para fruta em frutas {
    escreva(fruta)
}
```

---

## 2.4 Símbolos universais continuam universais

O Leviata não deve transformar programação em uma frase enorme.

Continuamos usando:

```text
+
-
*
/
%
=
==
!=
>
<
>=
<=
```

Preferimos:

```text
se idade >= 18 {
```

em vez de inventar algo como:

```text
se idade for maior ou igual a 18 entao faca
```

O objetivo é facilitar o aprendizado sem dificultar a futura transição para outras linguagens.

---

## 2.5 Operadores lógicos são palavras

A forma canônica será:

```text
e
ou
nao
```

Exemplo:

```text
se idade >= 18 e tem_documento {
    escreva("Entrada permitida")
}
```

Símbolos como `&&`, `||` e `!` podem existir como compatibilidade, mas não serão a forma ensinada primeiro.

---

## 2.6 Código oficial sem acentos em identificadores da linguagem

A documentação pode escrever português normalmente, porém palavras-chave e nomes oficiais da API evitam caracteres acentuados.

Usamos:

```text
funcao
senao
nao
ate
comeca_com
```

em vez de:

```text
função
senão
não
até
começa_com
```

Motivos:

- evita problemas de teclado e codificação;
- facilita terminal, compiladores, ferramentas e arquivos;
- torna o código mais portátil;
- mantém um padrão único.

O aluno continua lendo a palavra normalmente em português.

---

# 3. Vocabulário fundamental oficial

Esta tabela define a forma que deve aparecer em aulas, documentação e exemplos.

| Conceito | Forma comum em inglês | Leviata canônico | Motivo |
|---|---|---|---|
| saída | `print` | `escreva` | É a palavra tradicionalmente usada no ensino de algoritmos em português e soa como uma instrução direta. |
| entrada | `input` | `leia` | Combina com `escreva` e é familiar em pseudocódigo e Portugol. |
| variável | `var`, `let` | `var` | É curto e funciona como abreviação natural de variável. |
| constante | `const` | `const` | Curto, reconhecível e próximo de constante. |
| condição | `if` | `se` | Tradução natural e imediata. |
| alternativa | `else` | `senao` | Permite ler a estrutura como português. |
| repetição condicional | `while` | `enquanto` | Corresponde exatamente à ideia ensinada em lógica. |
| repetição | `for` | `para` | Natural em frases como `para numero de 1 ate 10`. |
| coleção | `in` | `em` | Permite `para item em lista`. |
| início de faixa | `from` | `de` | Permite `de 1 ate 10`. |
| fim de faixa | `to` | `ate` | Completa a leitura natural da faixa. |
| função | `function`, `def` | `funcao` | Nome conhecido em português e sem abreviação obscura. |
| retorno | `return` | `retorne` | É uma instrução dada à função. |
| verdadeiro | `true` | `verdadeiro` | Valor lógico explícito em português. |
| falso | `false` | `falso` | Valor lógico explícito em português. |
| ausência de valor | `null`, `nil` | `nulo` | Termo conhecido e direto. |
| E lógico | `and`, `&&` | `e` | Lê-se naturalmente dentro da condição. |
| OU lógico | `or`, `||` | `ou` | Lê-se naturalmente dentro da condição. |
| negação | `not`, `!` | `nao` | Mais fácil de explicar para iniciantes. |
| interromper laço | `break` | `pare` | Explica diretamente o efeito da instrução. |
| próxima repetição | `continue` | `continue` | Forma imperativa coerente com `pare` e `retorne`. |
| importar módulo | `import`, `use` | `usar` | Para iniciantes, “usar um módulo” é mais direto que “importar”. |
| classe | `class` | `classe` | Termo padrão em orientação a objetos. |
| criar objeto | `new` | `novo` | Natural e curto. |
| objeto atual | `this`, `self` | `este` | Representa o próprio objeto sem importar uma palavra inglesa. |
| herança | `extends`, `inherit` | `herda` | Lê-se como `classe Cachorro herda Animal`. |

---

# 4. Primeiro programa oficial

O primeiro contato com o Leviata deve ser simples:

```text
escreva("Ola, mundo!")
```

Não é necessário declarar `programa`, `inicio`, `algoritmo` ou outra estrutura artificial apenas para executar uma linha.

---

# 5. Entrada e saída

## 5.1 `print` -> `escreva`

Forma oficial:

```text
escreva("Ola")
```

### Por que `escreva`

- é uma instrução;
- já é familiar em materiais brasileiros de lógica de programação;
- pode ser explicada sem introduzir o termo inglês `print`;
- funciona para texto, números e outros valores;
- evita ter `escrever`, `imprimir` e `print` como três nomes concorrentes.

`escrever` poderá existir temporariamente durante migração, mas **não será o nome canônico**.

`imprimir` não será usado como nome principal porque pode dar a impressão de impressão física em papel.

---

## 5.2 `input` -> `leia`

Forma oficial:

```text
var nome = leia("Qual e seu nome? ")
```

### Por que `leia`

O computador “lê” uma informação fornecida pelo usuário.

A dupla fica didaticamente clara:

```text
var nome = leia("Nome: ")
escreva("Ola,", nome)
```

---

# 6. Variáveis e constantes

Forma canônica:

```text
var nome = "Ana"
var idade = 18

const PI = 3.14159
```

### Por que `var`

`variavel` seria mais explícito, porém aparece muitas vezes no código e tornaria programas básicos desnecessariamente longos.

`var` é uma abreviação curta de variável e já é comum no vocabulário de programação.

### Por que `const`

Pelo mesmo motivo: é curto, visualmente diferente de `var` e representa uma constante sem acrescentar ruído.

---

# 7. Condições

Forma oficial:

```text
se idade >= 18 {
    escreva("Maior de idade")
} senao {
    escreva("Menor de idade")
}
```

Para várias condições:

```text
se nota >= 7 {
    escreva("Aprovado")
} senao se nota >= 5 {
    escreva("Recuperacao")
} senao {
    escreva("Reprovado")
}
```

Não é necessário adicionar `entao`, pois `{ }` já indica onde o bloco começa e termina.

Isso reduz palavras sem perder clareza.

---

# 8. Repetições

## 8.1 Enquanto

```text
var numero = 1

enquanto numero <= 5 {
    escreva(numero)
    numero = numero + 1
}
```

---

## 8.2 Faixa numérica

```text
para numero de 1 ate 10 {
    escreva(numero)
}
```

Essa construção é escolhida porque pode ser lida literalmente:

> Para número de 1 até 10.

---

## 8.3 Percorrer coleção

A linguagem deverá possuir a forma:

```text
var frutas = ["maca", "uva", "banana"]

para fruta em frutas {
    escreva(fruta)
}
```

Isso evita obrigar o iniciante a aprender índices antes de aprender a percorrer uma lista.

---

## 8.4 Parar e continuar

```text
para numero de 1 ate 100 {
    se numero == 10 {
        pare
    }

    se numero == 5 {
        continue
    }

    escreva(numero)
}
```

### Mudança planejada

```text
quebrar -> pare
continuar -> continue
```

`quebrar` é uma tradução literal de `break`, mas não descreve tão bem a intenção para um aluno.

`pare` significa exatamente o que acontece com o laço.

---

# 9. Funções

Forma oficial:

```text
funcao soma(a, b) {
    retorne a + b
}

var resultado = soma(10, 5)
escreva(resultado)
```

### Mudança planejada

```text
retornar -> retorne
```

`retorne` segue a regra de comandos como instruções.

---

# 10. Valores fundamentais

```text
verdadeiro
falso
nulo
```

Exemplo:

```text
var conectado = verdadeiro
var erro = nulo

se conectado e erro == nulo {
    escreva("Tudo certo")
}
```

---

# 11. Conversão de tipos

A API atual possui nomes como:

```text
para_texto(valor)
para_inteiro(valor)
para_decimal(valor)
```

A forma canônica futura será mais curta:

```text
texto(valor)
inteiro(valor)
decimal(valor)
```

Exemplo:

```text
var idade_texto = leia("Idade: ")
var idade = inteiro(idade_texto)
```

### Motivo

O tipo desejado já explica a conversão.

`inteiro(valor)` significa diretamente “transforme/interprete este valor como inteiro”.

Isso também reduz funções excessivamente longas.

---

# 12. Texto

Biblioteca canônica básica:

```text
tamanho(valor)
maiusculas(texto)
minusculas(texto)
contem(texto, procura)
comeca_com(texto, inicio)
termina_com(texto, fim)
trecho(texto, inicio, fim)
```

### Mudança planejada

```text
fatiar -> trecho
começa_com -> comeca_com
```

`fatiar` é jargão de programação derivado de “slice”. Para um aluno, “trecho de um texto” comunica melhor o resultado.

A forma sem acento segue a regra de portabilidade da linguagem.

---

# 13. Listas

Criação:

```text
var nomes = ["Ana", "Joao", "Maria"]
```

Operações canônicas:

```text
tamanho(nomes)
adicionar(nomes, "Carlos")
remover(nomes, "Ana")
inserir(nomes, 1, "Pedro")
contem(nomes, "Maria")
limpar(nomes)
```

A biblioteca deve evitar prefixos redundantes quando o primeiro argumento deixa claro o objeto da operação.

Preferimos:

```text
adicionar(lista, item)
```

em vez de:

```text
lista_adicionar(lista, item)
```

quando não houver ambiguidade.

---

# 14. Matemática

Os nomes matemáticos atuais já seguem bem o objetivo do Leviata e formam a base canônica:

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
pi()
```

Aleatoriedade:

```text
aleatorio()
aleatorio_inteiro(minimo, maximo)
```

Esses nomes devem permanecer explícitos. Abreviações matemáticas obscuras não são desejáveis na biblioteca básica.

---

# 15. Arquivos

A família de arquivos permanece explícita porque operações genéricas como `ler()` já significam entrada do usuário.

```text
ler_arquivo(caminho)
escrever_arquivo(caminho, conteudo)
adicionar_arquivo(caminho, conteudo)
```

Aqui o sufixo `_arquivo` evita ambiguidade e melhora a leitura.

---

# 16. Módulos e nomes longos

APIs maiores não devem virar nomes gigantes separados por `_`.

Exemplo atual:

```text
jogo_desenhar_retangulo(...)
jogo_camera_configurar(...)
entidade_atualizar_tilemap(...)
```

Direção canônica futura:

```text
desenho.retangulo(...)
camera.configurar(...)
entidade.atualizar_mapa(...)
```

### Por que módulos com ponto

O ponto permite separar mentalmente:

```text
modulo.acao
```

Assim o aluno aprende que:

```text
camera.seguir(...)
```

significa “execute a ação `seguir` do módulo `camera`”.

Isso é mais simples de memorizar e deixa a API crescer sem gerar nomes enormes.

---

# 17. API de jogo — direção canônica

## 17.1 Jogo

```text
jogo.iniciar(titulo, largura, altura, fps)
jogo.fechar()
jogo.deve_fechar()
jogo.delta()
jogo.fps()
```

---

## 17.2 Quadro de desenho

O termo oficial em português será **quadro**, evitando exigir que o aluno conheça `frame` no início.

```text
jogo.iniciar_quadro()
jogo.finalizar_quadro()
```

---

## 17.3 Desenho

```text
desenho.limpar(r, g, b, a)
desenho.retangulo(x, y, largura, altura, r, g, b, a)
desenho.circulo(x, y, raio, r, g, b, a)
desenho.texto(texto, x, y, tamanho, r, g, b, a)
desenho.sprite(caminho, x, y, largura, altura)
desenho.sprite_regiao(...)
desenho.animacao(...)
desenho.tilemap(...)
```

A palavra `desenho` agrupa tudo que coloca algo visual na tela.

---

# 18. Teclado

Códigos numéricos de tecla são ruins para ensino.

Não queremos que um iniciante precise decorar:

```text
65
68
32
257
```

A direção desejada é permitir nomes:

```text
teclado.pressionada("A")
teclado.apertada("espaco")
teclado.solta("enter")
```

Exemplo:

```text
se teclado.pressionada("A") {
    x = x - 100 * jogo.delta()
}
```

Os códigos numéricos podem continuar existindo internamente ou para uso avançado.

---

# 19. Mouse

Forma canônica:

```text
mouse.x()
mouse.y()
mouse.pressionado("esquerdo")
mouse.apertado("esquerdo")
```

O vocabulário deve acompanhar o usado no teclado sempre que os conceitos forem equivalentes.

---

# 20. Áudio

```text
audio.iniciar()
audio.tocar_som(caminho)
audio.tocar_musica(caminho)
audio.volume(valor)
```

Agrupar tudo sob `audio` evita nomes globais extensos e facilita descobrir funções relacionadas.

---

# 21. Câmera

```text
camera.configurar(alvo_x, alvo_y, zoom, rotacao)
camera.seguir(x, y)
camera.limites(min_x, min_y, max_x, max_y)
camera.limpar_limites()
camera.iniciar()
camera.finalizar()
```

Comparação:

```text
jogo_camera_limites_limpar()
```

fica:

```text
camera.limpar_limites()
```

A segunda forma mostra primeiro **o objeto do assunto** e depois **a ação**.

---

# 22. Cenas

```text
cena.mudar(nome)
cena.atual()
cena.anterior()
cena.e(nome)
cena.versao()
```

Exemplo:

```text
se cena.e("menu") {
    escreva("Estamos no menu")
}
```

---

# 23. Mapas

Forma canônica:

```text
var mapa = mapa.carregar("mundo.csv")

var largura = mapa.largura()
var altura = mapa.altura()
```

No futuro, quando existirem vários mapas carregados simultaneamente, a API deve preferir trabalhar com o próprio valor retornado pelo carregamento, evitando estado global implícito.

Exemplo desejável:

```text
var mundo = mapa.carregar("mundo.csv")

escreva(mapa.largura(mundo))
escreva(mapa.altura(mundo))
```

ou, se o sistema de objetos permitir:

```text
escreva(mundo.largura)
escreva(mundo.altura)
```

A escolha entre essas duas formas será registrada quando o modelo de módulos/objetos estiver definido.

---

# 24. Entidades

API atual muito prefixada:

```text
entidade_criar
entidade_destruir
entidade_posicionar
entidade_velocidade
entidade_fisica
```

Forma canônica planejada:

```text
entidade.criar(nome, x, y, largura, altura)
entidade.destruir(id)
entidade.posicionar(id, x, y)
entidade.velocidade(id, vx, vy)
entidade.fisica(id, gravidade, arrasto)
entidade.colisor(id, ativo)
entidade.atualizar(id, delta)
entidade.atualizar_mapa(id, delta, mapa)
entidade.desenhar(id, r, g, b, a)
```

Consultas:

```text
entidade.x(id)
entidade.y(id)
entidade.vx(id)
entidade.vy(id)
entidade.no_chao(id)
entidade.nome(id)
entidade.quantidade()
entidade.tem_componente(id, componente)
```

---

# 25. Colisão

A colisão deve formar seu próprio módulo quando a API for reorganizada:

```text
colisao.retangulos(...)
colisao.circulos(...)
colisao.ponto_retangulo(...)
colisao.tilemap(...)
```

Isso é preferível a nomes como:

```text
jogo_colisao_retangulos
jogo_ponto_no_retangulo
```

porque colisão é um assunto independente e facilmente reconhecível.

---

# 26. Classes e objetos

Forma desejada:

```text
classe Animal {
    funcao falar() {
        escreva("Som")
    }
}

classe Cachorro herda Animal {
    funcao falar() {
        escreva("Au au")
    }
}

var cachorro = novo Cachorro()
```

Dentro de métodos, o objeto atual será representado por:

```text
este
```

Exemplo:

```text
este.nome
```

---

# 27. Convenção de nomes

## Palavras-chave

Uma palavra quando possível:

```text
se
para
funcao
retorne
```

## Funções globais simples

Verbo ou substantivo claro:

```text
escreva
leia
tamanho
inteiro
aleatorio
```

## Funções compostas

Usar `_` quando continuarem globais:

```text
ler_arquivo
aleatorio_inteiro
comeca_com
```

## APIs grandes

Preferir módulos:

```text
camera.seguir
jogo.iniciar
audio.volume
entidade.criar
```

em vez de:

```text
jogo_camera_seguir
jogo_audio_volume
entidade_criar
```

---

# 28. Predicados: funções que respondem verdadeiro ou falso

O nome deve parecer uma pergunta.

Boas formas:

```text
jogo.deve_fechar()
entidade.no_chao(id)
entidade.tem_componente(id, "fisica")
cena.e("menu")
texto.contem(...)
```

Evitar nomes vagos como:

```text
verifica(...)
testa(...)
check(...)
```

O nome deve indicar **o que está sendo perguntado**.

---

# 29. O que não será traduzido desnecessariamente

O Leviata é em português, mas não precisa inventar substitutos para conceitos universais que já são simples visualmente.

Permanecem símbolos como:

```text
()
{}
[]
.
,
+
-
*
/
=
==
>=
<=
```

Também não queremos frases excessivamente longas apenas para parecer português.

O código deve continuar sendo código.

---

# 30. Programa didático completo

Exemplo do estilo que a linguagem deve buscar:

```text
funcao media(notas) {
    var total = 0

    para nota em notas {
        total = total + nota
    }

    retorne total / tamanho(notas)
}

var nome = leia("Nome do aluno: ")
var notas = [8, 7, 9]
var resultado = media(notas)

escreva("Aluno:", nome)
escreva("Media:", resultado)

se resultado >= 7 {
    escreva("Aprovado")
} senao {
    escreva("Reprovado")
}
```

Esse exemplo contém vários dos princípios principais sem exigir conhecimento específico do funcionamento interno do compilador.

---

# 31. Exemplo de jogo no estilo futuro

```text
jogo.iniciar("Meu jogo", 960, 540, 60)

var x = 430
var y = 240

enquanto nao jogo.deve_fechar() {
    var delta = jogo.delta()

    se teclado.pressionada("A") {
        x = x - 200 * delta
    }

    se teclado.pressionada("D") {
        x = x + 200 * delta
    }

    jogo.iniciar_quadro()

    desenho.limpar(20, 24, 36)
    desenho.retangulo(x, y, 70, 70, 90, 180, 255)

    jogo.finalizar_quadro()
}

jogo.fechar()
```

A intenção é que mesmo um aluno que ainda não sabe Raylib, C ou detalhes de renderização consiga entender aproximadamente o que acontece.

---

# 32. Ordem recomendada para ensinar Leviata

A documentação de aprendizado deve apresentar recursos nesta ordem:

1. `escreva`
2. valores: texto e números
3. `var`
4. `leia`
5. operadores matemáticos
6. `se` e `senao`
7. `verdadeiro`, `falso`, `e`, `ou`, `nao`
8. `enquanto`
9. `para ... de ... ate`
10. listas
11. `para ... em ...`
12. funções
13. `retorne`
14. módulos e arquivos
15. classes e objetos
16. APIs especializadas, como jogos

A biblioteca padrão não deve obrigar o aluno a aprender conceitos avançados antes dessa progressão.

---

# 33. Regra para novas APIs

Antes de adicionar uma nova palavra-chave ou função à biblioteca oficial, responder:

1. Um iniciante entende aproximadamente o nome sem documentação?
2. Existe outra função do Leviata que já faz algo parecido?
3. O nome segue o mesmo padrão das APIs relacionadas?
4. Estamos criando um alias desnecessário?
5. O nome pode ser menor sem perder significado?
6. Ele pode ser explicado naturalmente em português?
7. É uma operação global ou pertence claramente a um módulo?

Se uma função pertence claramente a um assunto, preferir:

```text
modulo.acao()
```

---

# 34. Compatibilidade não define o futuro da linguagem

O código antigo pode precisar de aliases durante uma migração.

Exemplo possível:

```text
escrever -> escreva
imprimir -> escreva
retornar -> retorne
quebrar -> pare
```

Porém aliases de compatibilidade **não devem aparecer como alternativas equivalentes na documentação principal**.

A documentação deve sempre ensinar uma única forma oficial.

Quando a compatibilidade antiga puder ser removida com segurança, ela poderá ser descontinuada.

---

# 35. Resumo do vocabulário canônico

## Núcleo

```text
var
const

escreva
leia

se
senao

enquanto
para
em
de
ate

funcao
retorne

verdadeiro
falso
nulo

e
ou
nao

pare
continue

usar

classe
novo
este
herda
```

## Conversões e utilidades

```text
tamanho
texto
inteiro
decimal
```

## Texto

```text
maiusculas
minusculas
contem
comeca_com
termina_com
trecho
```

## Matemática

```text
raiz
potencia
absoluto
arredondar
teto
piso
seno
cosseno
tangente
logaritmo
aleatorio
aleatorio_inteiro
maximo
minimo
pi
```

## Arquivos

```text
ler_arquivo
escrever_arquivo
adicionar_arquivo
```

## Módulos de APIs maiores

```text
jogo.*
desenho.*
teclado.*
mouse.*
audio.*
camera.*
cena.*
mapa.*
entidade.*
colisao.*
```

---

# 36. Migrações principais planejadas

| Forma antiga/possível | Forma canônica | Razão principal |
|---|---|---|
| `print` | `escreva` | Português didático e já familiar em algoritmos. |
| `escrever` | `escreva` | Padronizar comandos como instruções. |
| `imprimir` | `escreva` | Evitar múltiplos nomes para saída. |
| `input` | `leia` | Par natural de `escreva`. |
| `retornar` | `retorne` | Forma de comando. |
| `quebrar` | `pare` | Explica diretamente o efeito no laço. |
| `continuar` | `continue` | Forma imperativa coerente. |
| `importar` | `usar` | Mais simples para um iniciante. |
| `herdar` | `herda` | Melhor leitura na declaração da classe. |
| `para_texto` | `texto` | Menor sem perder significado. |
| `para_inteiro` | `inteiro` | Menor sem perder significado. |
| `para_decimal` | `decimal` | Menor sem perder significado. |
| `fatiar` | `trecho` | Palavra mais natural para texto. |
| `começa_com` | `comeca_com` | API sem acentos. |
| `lista_adicionar` | `adicionar` | Prefixo redundante em operação simples. |
| `jogo_desenhar_retangulo` | `desenho.retangulo` | Organização modular e nome menor. |
| `jogo_camera_seguir` | `camera.seguir` | Organização por assunto. |
| `jogo_tocar_som` | `audio.tocar_som` | Organização por assunto. |
| `entidade_criar` | `entidade.criar` | API modular uniforme. |
| `mapa_carregar` | `mapa.carregar` | API modular uniforme. |
| códigos de tecla (`65`) | nomes (`"A"`) | Muito mais fácil de ensinar e lembrar. |

---

# 37. Histórico de decisões

## 2026-10-10 — Primeira padronização pedagógica

Foi decidido que o Leviata será reorganizado priorizando ensino de lógica de programação em português brasileiro.

Decisões principais:

- `print` passa a ser representado canonicamente por `escreva`;
- `input` passa a ser `leia`;
- a documentação terá apenas um nome oficial por conceito;
- comandos usam preferencialmente forma de instrução: `escreva`, `retorne`, `pare`;
- estruturas de controle usam português simples: `se`, `senao`, `enquanto`, `para`, `em`, `de`, `ate`;
- operadores matemáticos e comparadores continuam simbólicos;
- operadores lógicos são ensinados como `e`, `ou`, `nao`;
- palavras oficiais da linguagem evitam acentos no código;
- APIs grandes serão reorganizadas por módulos com ponto;
- códigos numéricos de teclado não serão a interface didática principal;
- compatibilidade com nomes antigos poderá existir durante a migração, mas não determinará a forma futura da linguagem;
- este documento passa a ser a referência histórica para justificar mudanças futuras de nomenclatura e sintaxe.

---

# 38. Regra de manutenção deste documento

Quando uma decisão de linguagem mudar, **não apagar silenciosamente a razão antiga**.

A alteração deve:

1. atualizar a seção correspondente;
2. adicionar uma entrada datada em **Histórico de decisões**;
3. explicar qual era a forma anterior;
4. registrar a nova forma;
5. explicar por que a mudança foi feita.

Assim, no futuro será possível entender não apenas **como** o Leviata funciona, mas **por que** cada decisão foi tomada.
