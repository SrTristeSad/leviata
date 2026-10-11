/* ============================================================================
 * LEVIATA 3.4 - CAMADA DE PARSER PARA CONSTRUTORES
 *
 * Mantém o parser 3.3 estável e normaliza:
 *     novo Pessoa("Alex", 20)
 * para uma chamada interna que a camada 3.4 do compilador transforma em
 * construtor real. Strings e comentários não são alterados.
 * ============================================================================ */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define leviata_parse leviata_parse_v33
#include "leviata_parser.c"
#undef leviata_parse

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} V34Buffer;

static int v34_garante(V34Buffer *b, size_t extra) {
    if (b->len + extra + 1 <= b->cap) return 1;
    size_t nova = b->cap ? b->cap : 256;
    while (nova < b->len + extra + 1) nova *= 2;
    char *p = (char*)realloc(b->data, nova);
    if (!p) return 0;
    b->data = p;
    b->cap = nova;
    return 1;
}

static int v34_char(V34Buffer *b, char c) {
    if (!v34_garante(b, 1)) return 0;
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
    return 1;
}

static int v34_txt(V34Buffer *b, const char *s, size_t n) {
    if (!v34_garante(b, n)) return 0;
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
    return 1;
}

static int v34_inicio_ident(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

static int v34_ident(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

static int v34_limite_palavra(const char *s, size_t i, size_t n) {
    if (i > 0 && v34_ident(s[i - 1])) return 0;
    if (i + n < strlen(s) && v34_ident(s[i + n])) return 0;
    return 1;
}

static char *v34_normaliza_novo(const char *src) {
    if (!src) return NULL;

    V34Buffer out = {0};
    size_t n = strlen(src);
    size_t i = 0;

    enum { NORMAL, ASPAS_DUPLAS, ASPAS_SIMPLES, COMENT_LINHA, COMENT_BLOCO } estado = NORMAL;

    while (i < n) {
        char c = src[i];

        if (estado == ASPAS_DUPLAS || estado == ASPAS_SIMPLES) {
            if (!v34_char(&out, c)) goto falha;
            if (c == '\\' && i + 1 < n) {
                if (!v34_char(&out, src[i + 1])) goto falha;
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
            if (!v34_char(&out, c)) goto falha;
            i++;
            if (c == '\n') estado = NORMAL;
            continue;
        }

        if (estado == COMENT_BLOCO) {
            if (!v34_char(&out, c)) goto falha;
            if (c == '*' && i < n && src[i] == '/') {
                if (!v34_char(&out, src[i])) goto falha;
                i++;
                estado = NORMAL;
            }
            continue;
        }

        if (c == '"') {
            estado = ASPAS_DUPLAS;
            if (!v34_char(&out, c)) goto falha;
            i++;
            continue;
        }
        if (c == '\'') {
            estado = ASPAS_SIMPLES;
            if (!v34_char(&out, c)) goto falha;
            i++;
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '/') {
            estado = COMENT_LINHA;
            if (!v34_txt(&out, src + i, 2)) goto falha;
            i += 2;
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '*') {
            estado = COMENT_BLOCO;
            if (!v34_txt(&out, src + i, 2)) goto falha;
            i += 2;
            continue;
        }

        if (i + 4 <= n && strncmp(src + i, "novo", 4) == 0 &&
            v34_limite_palavra(src, i, 4)) {
            size_t ws1_ini = i + 4;
            size_t j = ws1_ini;
            while (j < n && isspace((unsigned char)src[j])) j++;

            if (j < n && v34_inicio_ident(src[j])) {
                size_t nome_ini = j;
                j++;
                while (j < n && v34_ident(src[j])) j++;
                size_t nome_fim = j;

                size_t ws2_ini = j;
                while (j < n && isspace((unsigned char)src[j])) j++;

                if (j < n && src[j] == '(') {
                    if (!v34_txt(&out, "__leviata_novo(", 15)) goto falha;
                    if (!v34_txt(&out, src + ws1_ini, nome_ini - ws1_ini)) goto falha;
                    if (!v34_char(&out, '"')) goto falha;
                    if (!v34_txt(&out, src + nome_ini, nome_fim - nome_ini)) goto falha;
                    if (!v34_char(&out, '"')) goto falha;
                    if (!v34_txt(&out, src + ws2_ini, j - ws2_ini)) goto falha;

                    size_t arg_ini = j + 1;
                    size_t q = arg_ini;
                    while (q < n && isspace((unsigned char)src[q])) q++;

                    if (q < n && src[q] == ')') {
                        if (!v34_txt(&out, src + arg_ini, q - arg_ini)) goto falha;
                        if (!v34_char(&out, ')')) goto falha;
                        i = q + 1;
                    } else {
                        if (!v34_char(&out, ',')) goto falha;
                        i = arg_ini;
                    }
                    continue;
                }
            }
        }

        if (!v34_char(&out, c)) goto falha;
        i++;
    }

    if (!out.data) {
        out.data = (char*)calloc(1, 1);
    }
    return out.data;

falha:
    free(out.data);
    return NULL;
}

AstNode *leviata_parse(const char *source, int *had_error) {
    char *normalizado = v34_normaliza_novo(source);
    if (!normalizado) {
        if (had_error) *had_error = 1;
        fprintf(stderr, "[LEVIATA][Parser] Sem memória ao preparar construtor\n");
        return NULL;
    }

    AstNode *ast = leviata_parse_v33(normalizado, had_error);
    free(normalizado);
    return ast;
}
