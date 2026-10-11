/* ============================================================================
 * LEVIATA 3.6 - CAMADA DE SINTAXE PARA CONSTRUTORES E MODULOS
 *
 * Normaliza antes do parser base:
 *   novo Pessoa(...)             -> __leviata_novo("Pessoa", ...)
 *   novo util.Caixa(...)         -> __leviata_novo("util.Caixa", ...)
 *   exportar funcao dobro(...)   -> __leviata_exportar("dobro"); funcao dobro(...)
 *   usar "util.pt" como util     -> usar "util.pt"; __leviata_alias_import("util.pt", "util")
 *
 * Strings e comentários são preservados sem transformações internas.
 * ============================================================================ */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define leviata_parse leviata_parse_base
#include "leviata_parser.c"
#undef leviata_parse

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} V36Buffer;

static int v36_garante(V36Buffer *b, size_t extra) {
    if (b->len + extra + 1 <= b->cap) return 1;
    size_t nova = b->cap ? b->cap : 256;
    while (nova < b->len + extra + 1) nova *= 2;
    char *p = (char*)realloc(b->data, nova);
    if (!p) return 0;
    b->data = p;
    b->cap = nova;
    return 1;
}

static int v36_char(V36Buffer *b, char c) {
    if (!v36_garante(b, 1)) return 0;
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
    return 1;
}

static int v36_txt(V36Buffer *b, const char *s, size_t n) {
    if (!v36_garante(b, n)) return 0;
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
    return 1;
}

static int v36_inicio_ident(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

static int v36_ident(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

static int v36_limite_palavra(const char *s, size_t total,
                              size_t i, size_t tam) {
    if (i > 0 && v36_ident(s[i - 1])) return 0;
    if (i + tam < total && v36_ident(s[i + tam])) return 0;
    return 1;
}

static size_t v36_pula_espaco(const char *s, size_t n, size_t i) {
    while (i < n && isspace((unsigned char)s[i])) i++;
    return i;
}

static int v36_palavra_em(const char *s, size_t n, size_t i,
                          const char *palavra) {
    size_t tam = strlen(palavra);
    return i + tam <= n && strncmp(s + i, palavra, tam) == 0 &&
           v36_limite_palavra(s, n, i, tam);
}

/* Retorna posição após a string literal ou 0 se não for string válida. */
static size_t v36_fim_string(const char *s, size_t n, size_t inicio) {
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

static int v36_exportar(V36Buffer *out, const char *src,
                        size_t n, size_t *io_i) {
    size_t i = *io_i;
    if (!v36_palavra_em(src, n, i, "exportar")) return 0;

    size_t j = v36_pula_espaco(src, n, i + 8);
    const char *tipos[] = {"funcao", "classe", "var", "const", NULL};
    size_t apos_tipo = 0;

    for (int t = 0; tipos[t]; t++) {
        size_t tam = strlen(tipos[t]);
        if (v36_palavra_em(src, n, j, tipos[t])) {
            apos_tipo = j + tam;
            break;
        }
    }
    if (!apos_tipo) return 0;

    size_t k = v36_pula_espaco(src, n, apos_tipo);
    if (k >= n || !v36_inicio_ident(src[k])) return 0;
    size_t nome_ini = k++;
    while (k < n && v36_ident(src[k])) k++;

    if (!v36_txt(out, "__leviata_exportar(\"", 20)) return -1;
    if (!v36_txt(out, src + nome_ini, k - nome_ini)) return -1;
    if (!v36_txt(out, "\"); ", 4)) return -1;

    /* Reprocessa a declaração a partir de funcao/classe/var/const. */
    *io_i = j;
    return 1;
}

static int v36_usar_alias(V36Buffer *out, const char *src,
                          size_t n, size_t *io_i) {
    size_t i = *io_i;
    if (!v36_palavra_em(src, n, i, "usar")) return 0;

    size_t path_ini = v36_pula_espaco(src, n, i + 4);
    size_t path_fim = v36_fim_string(src, n, path_ini);
    if (!path_fim) return 0;

    size_t j = v36_pula_espaco(src, n, path_fim);
    if (!v36_palavra_em(src, n, j, "como")) return 0;

    size_t alias_ini = v36_pula_espaco(src, n, j + 4);
    if (alias_ini >= n || !v36_inicio_ident(src[alias_ini])) return 0;
    size_t alias_fim = alias_ini + 1;
    while (alias_fim < n && v36_ident(src[alias_fim])) alias_fim++;

    /* Mantém o usar original sem o `como alias`. */
    if (!v36_txt(out, src + i, path_fim - i)) return -1;
    if (!v36_txt(out, "; __leviata_alias_import(", 25)) return -1;
    if (!v36_txt(out, src + path_ini, path_fim - path_ini)) return -1;
    if (!v36_txt(out, ", \"", 3)) return -1;
    if (!v36_txt(out, src + alias_ini, alias_fim - alias_ini)) return -1;
    if (!v36_txt(out, "\");", 3)) return -1;

    size_t fim = alias_fim;
    while (fim < n && (src[fim] == ' ' || src[fim] == '\t' || src[fim] == '\r'))
        fim++;
    if (fim < n && src[fim] == ';') fim++;
    *io_i = fim;
    return 1;
}

static int v36_novo(V36Buffer *out, const char *src,
                    size_t n, size_t *io_i) {
    size_t i = *io_i;
    if (!v36_palavra_em(src, n, i, "novo")) return 0;

    size_t ws1_ini = i + 4;
    size_t j = v36_pula_espaco(src, n, ws1_ini);
    if (j >= n || !v36_inicio_ident(src[j])) return 0;

    size_t nome_ini = j;
    j++;
    while (j < n && v36_ident(src[j])) j++;

    /* Permite classe qualificada: modulo.Classe.Subclasse. */
    while (j < n && src[j] == '.') {
        size_t ponto = j++;
        if (j >= n || !v36_inicio_ident(src[j])) {
            j = ponto;
            break;
        }
        j++;
        while (j < n && v36_ident(src[j])) j++;
    }
    size_t nome_fim = j;

    size_t ws2_ini = j;
    j = v36_pula_espaco(src, n, j);
    if (j >= n || src[j] != '(') return 0;

    if (!v36_txt(out, "__leviata_novo(", 15)) return -1;
    if (!v36_txt(out, src + ws1_ini, nome_ini - ws1_ini)) return -1;
    if (!v36_char(out, '"')) return -1;
    if (!v36_txt(out, src + nome_ini, nome_fim - nome_ini)) return -1;
    if (!v36_char(out, '"')) return -1;
    if (!v36_txt(out, src + ws2_ini, j - ws2_ini)) return -1;

    size_t arg_ini = j + 1;
    size_t q = v36_pula_espaco(src, n, arg_ini);
    if (q < n && src[q] == ')') {
        if (!v36_txt(out, src + arg_ini, q - arg_ini)) return -1;
        if (!v36_char(out, ')')) return -1;
        *io_i = q + 1;
    } else {
        if (!v36_char(out, ',')) return -1;
        *io_i = arg_ini;
    }
    return 1;
}

static char *v36_normaliza(const char *src) {
    if (!src) return NULL;

    V36Buffer out = {0};
    size_t n = strlen(src);
    size_t i = 0;
    enum { NORMAL, ASPAS_DUPLAS, ASPAS_SIMPLES,
           COMENT_LINHA, COMENT_BLOCO } estado = NORMAL;

    while (i < n) {
        char c = src[i];

        if (estado == ASPAS_DUPLAS || estado == ASPAS_SIMPLES) {
            if (!v36_char(&out, c)) goto falha;
            if (c == '\\' && i + 1 < n) {
                if (!v36_char(&out, src[i + 1])) goto falha;
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
            if (!v36_char(&out, c)) goto falha;
            i++;
            if (c == '\n') estado = NORMAL;
            continue;
        }

        if (estado == COMENT_BLOCO) {
            if (c == '*' && i + 1 < n && src[i + 1] == '/') {
                if (!v36_txt(&out, src + i, 2)) goto falha;
                i += 2;
                estado = NORMAL;
            } else {
                if (!v36_char(&out, c)) goto falha;
                i++;
            }
            continue;
        }

        if (c == '"') {
            estado = ASPAS_DUPLAS;
            if (!v36_char(&out, c)) goto falha;
            i++;
            continue;
        }
        if (c == '\'') {
            estado = ASPAS_SIMPLES;
            if (!v36_char(&out, c)) goto falha;
            i++;
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '/') {
            estado = COMENT_LINHA;
            if (!v36_txt(&out, src + i, 2)) goto falha;
            i += 2;
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '*') {
            estado = COMENT_BLOCO;
            if (!v36_txt(&out, src + i, 2)) goto falha;
            i += 2;
            continue;
        }

        int r = v36_exportar(&out, src, n, &i);
        if (r < 0) goto falha;
        if (r > 0) continue;

        r = v36_usar_alias(&out, src, n, &i);
        if (r < 0) goto falha;
        if (r > 0) continue;

        r = v36_novo(&out, src, n, &i);
        if (r < 0) goto falha;
        if (r > 0) continue;

        if (!v36_char(&out, c)) goto falha;
        i++;
    }

    if (!out.data) out.data = (char*)calloc(1, 1);
    return out.data;

falha:
    free(out.data);
    return NULL;
}

AstNode *leviata_parse(const char *source, int *had_error) {
    char *normalizado = v36_normaliza(source);
    if (!normalizado) {
        if (had_error) *had_error = 1;
        fprintf(stderr, "[LEVIATA][Parser] Sem memoria ao preparar sintaxe 3.6\n");
        return NULL;
    }

    AstNode *ast = leviata_parse_base(normalizado, had_error);
    free(normalizado);
    return ast;
}
