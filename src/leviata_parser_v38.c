/* ============================================================================
 * LEVIATA 3.8 - LITERAIS DE MAPA
 *
 * Normaliza antes do parser 3.6:
 *   { nome: "Alex", idade: 29 }
 *      -> mapa("nome", "Alex", "idade", 29)
 *
 * Chaves podem ser identificadores ou textos. A deteccao exige `chave:` como
 * primeiro elemento, portanto blocos normais `{ ... }` continuam intactos.
 * ============================================================================ */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define leviata_parse leviata_parse_v36
#include "leviata_parser_v36.c"
#undef leviata_parse

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} V38Buffer;

static int v38_garante(V38Buffer *b, size_t extra) {
    if (b->len + extra + 1 <= b->cap) return 1;
    size_t nova = b->cap ? b->cap : 256;
    while (nova < b->len + extra + 1) nova *= 2;
    char *p = (char*)realloc(b->data, nova);
    if (!p) return 0;
    b->data = p;
    b->cap = nova;
    return 1;
}

static int v38_char(V38Buffer *b, char c) {
    if (!v38_garante(b, 1)) return 0;
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
    return 1;
}

static int v38_txt(V38Buffer *b, const char *s, size_t n) {
    if (!v38_garante(b, n)) return 0;
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
    return 1;
}

static int v38_ident_inicio(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

static int v38_ident_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

static size_t v38_fim_string(const char *s, size_t n, size_t inicio) {
    if (inicio >= n || (s[inicio] != '"' && s[inicio] != '\'')) return 0;
    char q = s[inicio];
    size_t i = inicio + 1;
    while (i < n) {
        if (s[i] == '\\' && i + 1 < n) {
            i += 2;
            continue;
        }
        if (s[i] == q) return i + 1;
        i++;
    }
    return 0;
}

/* Pula espacos e comentarios apenas para reconhecer a estrutura do mapa. */
static size_t v38_pula_trivia(const char *s, size_t n, size_t i) {
    for (;;) {
        while (i < n && isspace((unsigned char)s[i])) i++;
        if (i + 1 < n && s[i] == '/' && s[i + 1] == '/') {
            i += 2;
            while (i < n && s[i] != '\n') i++;
            continue;
        }
        if (i + 1 < n && s[i] == '/' && s[i + 1] == '*') {
            i += 2;
            while (i + 1 < n && !(s[i] == '*' && s[i + 1] == '/')) i++;
            if (i + 1 < n) i += 2;
            continue;
        }
        return i;
    }
}

static int v38_contexto_expressao(const char *s, size_t brace) {
    if (brace == 0) return 1;
    size_t i = brace;
    while (i > 0 && isspace((unsigned char)s[i - 1])) i--;
    if (i == 0) return 1;

    char c = s[i - 1];
    if (c == '=' || c == '(' || c == '[' || c == ',' || c == ':') return 1;

    if (v38_ident_char(c)) {
        size_t fim = i;
        size_t ini = i - 1;
        while (ini > 0 && v38_ident_char(s[ini - 1])) ini--;
        size_t tam = fim - ini;
        if (tam == 7 && strncmp(s + ini, "retorne", 7) == 0) return 1;
    }
    return 0;
}

/* Encontra a virgula do par ou a chave final, ignorando estruturas aninhadas. */
static size_t v38_fim_valor(const char *s, size_t n, size_t inicio, char *delim) {
    int par = 0, col = 0, cha = 0;
    enum { V_NORMAL, V_DQ, V_SQ, V_LINHA, V_BLOCO } estado = V_NORMAL;

    for (size_t i = inicio; i < n; i++) {
        char c = s[i];

        if (estado == V_DQ || estado == V_SQ) {
            if (c == '\\' && i + 1 < n) { i++; continue; }
            if ((estado == V_DQ && c == '"') ||
                (estado == V_SQ && c == '\'')) estado = V_NORMAL;
            continue;
        }
        if (estado == V_LINHA) {
            if (c == '\n') estado = V_NORMAL;
            continue;
        }
        if (estado == V_BLOCO) {
            if (c == '*' && i + 1 < n && s[i + 1] == '/') {
                i++;
                estado = V_NORMAL;
            }
            continue;
        }

        if (c == '"') { estado = V_DQ; continue; }
        if (c == '\'') { estado = V_SQ; continue; }
        if (c == '/' && i + 1 < n && s[i + 1] == '/') {
            i++;
            estado = V_LINHA;
            continue;
        }
        if (c == '/' && i + 1 < n && s[i + 1] == '*') {
            i++;
            estado = V_BLOCO;
            continue;
        }

        if (c == '(') { par++; continue; }
        if (c == ')') { if (par > 0) par--; continue; }
        if (c == '[') { col++; continue; }
        if (c == ']') { if (col > 0) col--; continue; }
        if (c == '{') { cha++; continue; }
        if (c == '}') {
            if (cha > 0) { cha--; continue; }
            if (par == 0 && col == 0) {
                *delim = '}';
                return i;
            }
            continue;
        }
        if (c == ',' && par == 0 && col == 0 && cha == 0) {
            *delim = ',';
            return i;
        }
    }
    return 0;
}

static char *v38_normaliza_mapas(const char *src);

static int v38_tenta_mapa(V38Buffer *out,
                          const char *src,
                          size_t n,
                          size_t *io_i) {
    size_t inicio = *io_i;
    if (inicio >= n || src[inicio] != '{') return 0;

    size_t p = v38_pula_trivia(src, n, inicio + 1);
    if (p >= n) return 0;

    if (src[p] == '}') {
        if (!v38_contexto_expressao(src, inicio)) return 0;
        if (!v38_txt(out, "mapa()", 6)) return -1;
        *io_i = p + 1;
        return 1;
    }

    /* Um mapa nao vazio obrigatoriamente comeca por chave seguida de ':'. */
    V38Buffer tmp = {0};
    if (!v38_txt(&tmp, "mapa(", 5)) goto sem_memoria;
    int pares = 0;

    for (;;) {
        p = v38_pula_trivia(src, n, p);
        if (p >= n) goto nao_e_mapa;

        size_t chave_ini = p;
        size_t chave_fim = 0;
        int chave_string = 0;

        if (src[p] == '"' || src[p] == '\'') {
            chave_fim = v38_fim_string(src, n, p);
            if (!chave_fim) goto nao_e_mapa;
            chave_string = 1;
        } else if (v38_ident_inicio(src[p])) {
            p++;
            while (p < n && v38_ident_char(src[p])) p++;
            chave_fim = p;
        } else {
            goto nao_e_mapa;
        }

        p = v38_pula_trivia(src, n, chave_fim);
        if (p >= n || src[p] != ':') goto nao_e_mapa;

        size_t valor_ini = p + 1;
        char delimitador = 0;
        size_t valor_fim = v38_fim_valor(src, n, valor_ini, &delimitador);
        if (!valor_fim || !delimitador) goto nao_e_mapa;

        size_t valor_real_ini = v38_pula_trivia(src, valor_fim, valor_ini);
        if (valor_real_ini >= valor_fim) goto nao_e_mapa;

        if (pares > 0 && !v38_txt(&tmp, ", ", 2)) goto sem_memoria;

        if (chave_string) {
            if (!v38_txt(&tmp, src + chave_ini, chave_fim - chave_ini))
                goto sem_memoria;
        } else {
            if (!v38_char(&tmp, '"')) goto sem_memoria;
            if (!v38_txt(&tmp, src + chave_ini, chave_fim - chave_ini))
                goto sem_memoria;
            if (!v38_char(&tmp, '"')) goto sem_memoria;
        }
        if (!v38_txt(&tmp, ", ", 2)) goto sem_memoria;

        char *valor = (char*)malloc(valor_fim - valor_ini + 1);
        if (!valor) goto sem_memoria;
        memcpy(valor, src + valor_ini, valor_fim - valor_ini);
        valor[valor_fim - valor_ini] = '\0';

        char *normalizado = v38_normaliza_mapas(valor);
        free(valor);
        if (!normalizado) goto sem_memoria;
        int ok = v38_txt(&tmp, normalizado, strlen(normalizado));
        free(normalizado);
        if (!ok) goto sem_memoria;
        pares++;

        if (delimitador == '}') {
            if (!v38_char(&tmp, ')')) goto sem_memoria;
            if (!v38_txt(out, tmp.data, tmp.len)) goto sem_memoria;
            free(tmp.data);
            *io_i = valor_fim + 1;
            return 1;
        }

        p = v38_pula_trivia(src, n, valor_fim + 1);
        if (p < n && src[p] == '}') {
            /* Virgula final e valida. */
            if (!v38_char(&tmp, ')')) goto sem_memoria;
            if (!v38_txt(out, tmp.data, tmp.len)) goto sem_memoria;
            free(tmp.data);
            *io_i = p + 1;
            return 1;
        }
    }

nao_e_mapa:
    free(tmp.data);
    return 0;
sem_memoria:
    free(tmp.data);
    return -1;
}

static char *v38_normaliza_mapas(const char *src) {
    if (!src) return NULL;

    V38Buffer out = {0};
    size_t n = strlen(src);
    size_t i = 0;
    enum { NORMAL, ASPAS_DUPLAS, ASPAS_SIMPLES,
           COMENT_LINHA, COMENT_BLOCO } estado = NORMAL;

    while (i < n) {
        char c = src[i];

        if (estado == ASPAS_DUPLAS || estado == ASPAS_SIMPLES) {
            if (!v38_char(&out, c)) goto falha;
            if (c == '\\' && i + 1 < n) {
                if (!v38_char(&out, src[i + 1])) goto falha;
                i += 2;
                continue;
            }
            if ((estado == ASPAS_DUPLAS && c == '"') ||
                (estado == ASPAS_SIMPLES && c == '\''))
                estado = NORMAL;
            i++;
            continue;
        }

        if (estado == COMENT_LINHA) {
            if (!v38_char(&out, c)) goto falha;
            i++;
            if (c == '\n') estado = NORMAL;
            continue;
        }

        if (estado == COMENT_BLOCO) {
            if (c == '*' && i + 1 < n && src[i + 1] == '/') {
                if (!v38_txt(&out, src + i, 2)) goto falha;
                i += 2;
                estado = NORMAL;
            } else {
                if (!v38_char(&out, c)) goto falha;
                i++;
            }
            continue;
        }

        if (c == '"') {
            estado = ASPAS_DUPLAS;
            if (!v38_char(&out, c)) goto falha;
            i++;
            continue;
        }
        if (c == '\'') {
            estado = ASPAS_SIMPLES;
            if (!v38_char(&out, c)) goto falha;
            i++;
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '/') {
            estado = COMENT_LINHA;
            if (!v38_txt(&out, src + i, 2)) goto falha;
            i += 2;
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '*') {
            estado = COMENT_BLOCO;
            if (!v38_txt(&out, src + i, 2)) goto falha;
            i += 2;
            continue;
        }

        if (c == '{') {
            int r = v38_tenta_mapa(&out, src, n, &i);
            if (r < 0) goto falha;
            if (r > 0) continue;
        }

        if (!v38_char(&out, c)) goto falha;
        i++;
    }

    if (!out.data) out.data = (char*)calloc(1, 1);
    return out.data;

falha:
    free(out.data);
    return NULL;
}

AstNode *leviata_parse(const char *source, int *had_error) {
    char *normalizado = v38_normaliza_mapas(source);
    if (!normalizado) {
        if (had_error) *had_error = 1;
        fprintf(stderr, "[LEVIATA][Parser] Sem memoria ao preparar mapas 3.8\n");
        return NULL;
    }

    AstNode *ast = leviata_parse_v36(normalizado, had_error);
    free(normalizado);
    return ast;
}
