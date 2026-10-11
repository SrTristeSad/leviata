/* ============================================================================
 * LEVIATA 3.9 - ITERACAO CHAVE, VALOR EM MAPAS
 *
 * A sintaxe publica:
 *   para chave, valor em dados { ... }
 *
 * e normalizada antes do parser 3.8 para a forma ja suportada:
 *   para __leviata_par_N em pares(dados) {
 *       var chave = __leviata_par_N[0]
 *       var valor = __leviata_par_N[1]
 *       ...
 *   }
 *
 * A colecao e avaliada uma unica vez pelo foreach existente. `pares()` produz
 * um snapshot chave/valor do mapa no inicio da iteracao.
 * ============================================================================ */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "leviata_parser_v38.c"

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} V39Buffer;

static int v39_garante(V39Buffer *b, size_t extra) {
    if (b->len + extra + 1 <= b->cap) return 1;
    size_t nova = b->cap ? b->cap : 256;
    while (nova < b->len + extra + 1) nova *= 2;
    char *p = (char*)realloc(b->data, nova);
    if (!p) return 0;
    b->data = p;
    b->cap = nova;
    return 1;
}

static int v39_txt(V39Buffer *b, const char *s, size_t n) {
    if (!v39_garante(b, n)) return 0;
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
    return 1;
}

static int v39_char(V39Buffer *b, char c) {
    return v39_txt(b, &c, 1);
}

static int v39_inicio_ident(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

static int v39_ident(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

static size_t v39_espaco(const char *s, size_t n, size_t i) {
    while (i < n && isspace((unsigned char)s[i])) i++;
    return i;
}

static int v39_palavra(const char *s, size_t n, size_t i, const char *p) {
    size_t m = strlen(p);
    if (i + m > n || strncmp(s + i, p, m) != 0) return 0;
    if (i > 0 && v39_ident(s[i - 1])) return 0;
    if (i + m < n && v39_ident(s[i + m])) return 0;
    return 1;
}

static int v39_ident_intervalo(const char *s, size_t n, size_t *io,
                               size_t *ini, size_t *fim) {
    size_t i = v39_espaco(s, n, *io);
    if (i >= n || !v39_inicio_ident(s[i])) return 0;
    *ini = i++;
    while (i < n && v39_ident(s[i])) i++;
    *fim = i;
    *io = i;
    return 1;
}

/* Encontra a chave que inicia o bloco do `para`. A primeira chave pode ser um
 * literal de mapa quando a expressao da colecao comeca por `{`. */
static size_t v39_inicio_bloco(const char *s, size_t n, size_t inicio) {
    size_t primeiro = v39_espaco(s, n, inicio);
    int mapa_topo = primeiro < n && s[primeiro] == '{';
    int parenteses = 0;
    int colchetes = 0;
    int chaves_mapa = 0;
    enum { V39_NORMAL, V39_DQ, V39_SQ, V39_LINHA, V39_BLOCO } estado = V39_NORMAL;

    for (size_t i = inicio; i < n; i++) {
        char c = s[i];

        if (estado == V39_DQ || estado == V39_SQ) {
            if (c == '\\' && i + 1 < n) { i++; continue; }
            if ((estado == V39_DQ && c == '"') ||
                (estado == V39_SQ && c == '\'')) estado = V39_NORMAL;
            continue;
        }
        if (estado == V39_LINHA) {
            if (c == '\n') estado = V39_NORMAL;
            continue;
        }
        if (estado == V39_BLOCO) {
            if (c == '*' && i + 1 < n && s[i + 1] == '/') {
                i++;
                estado = V39_NORMAL;
            }
            continue;
        }

        if (c == '"') { estado = V39_DQ; continue; }
        if (c == '\'') { estado = V39_SQ; continue; }
        if (c == '/' && i + 1 < n && s[i + 1] == '/') {
            i++;
            estado = V39_LINHA;
            continue;
        }
        if (c == '/' && i + 1 < n && s[i + 1] == '*') {
            i++;
            estado = V39_BLOCO;
            continue;
        }

        if (c == '(') { parenteses++; continue; }
        if (c == ')' && parenteses > 0) { parenteses--; continue; }
        if (c == '[') { colchetes++; continue; }
        if (c == ']' && colchetes > 0) { colchetes--; continue; }

        if (parenteses == 0 && colchetes == 0) {
            if (c == '{') {
                if (chaves_mapa > 0 || (mapa_topo && i == primeiro)) {
                    chaves_mapa++;
                    continue;
                }
                if (chaves_mapa == 0) return i;
            }
            if (c == '}' && chaves_mapa > 0) {
                chaves_mapa--;
                continue;
            }
        }
    }
    return n;
}

/* 1 = transformou, 0 = nao era a forma dupla, -1 = erro de memoria. */
static int v39_tenta_para_duplo(V39Buffer *out, const char *s, size_t n,
                                size_t *io_i, int *contador) {
    size_t i = *io_i;
    if (!v39_palavra(s, n, i, "para")) return 0;

    size_t j = i + 4;
    size_t a0, a1, b0, b1;
    if (!v39_ident_intervalo(s, n, &j, &a0, &a1)) return 0;
    j = v39_espaco(s, n, j);
    if (j >= n || s[j] != ',') return 0;
    j++;
    if (!v39_ident_intervalo(s, n, &j, &b0, &b1)) return 0;
    j = v39_espaco(s, n, j);
    if (!v39_palavra(s, n, j, "em")) return 0;
    j += 2;

    size_t colecao_ini = v39_espaco(s, n, j);
    size_t bloco = v39_inicio_bloco(s, n, colecao_ini);
    if (bloco >= n || bloco <= colecao_ini) return 0;

    char oculto[96];
    snprintf(oculto, sizeof(oculto), "__leviata_par_%d", (*contador)++);

    if (!v39_txt(out, "para ", 5)) return -1;
    if (!v39_txt(out, oculto, strlen(oculto))) return -1;
    if (!v39_txt(out, " em pares(", 10)) return -1;
    if (!v39_txt(out, s + colecao_ini, bloco - colecao_ini)) return -1;
    if (!v39_txt(out, ") { var ", 8)) return -1;
    if (!v39_txt(out, s + a0, a1 - a0)) return -1;
    if (!v39_txt(out, " = ", 3)) return -1;
    if (!v39_txt(out, oculto, strlen(oculto))) return -1;
    if (!v39_txt(out, "[0]; var ", 9)) return -1;
    if (!v39_txt(out, s + b0, b1 - b0)) return -1;
    if (!v39_txt(out, " = ", 3)) return -1;
    if (!v39_txt(out, oculto, strlen(oculto))) return -1;
    if (!v39_txt(out, "[1]; ", 5)) return -1;

    *io_i = bloco + 1;
    return 1;
}

static char *v39_normaliza_iteracao(const char *src) {
    if (!src) return NULL;
    size_t n = strlen(src);
    V39Buffer out = {0};
    size_t i = 0;
    int contador = 0;
    enum { NORMAL, DQ, SQ, COM_LINHA, COM_BLOCO } estado = NORMAL;

    while (i < n) {
        char c = src[i];

        if (estado == DQ || estado == SQ) {
            if (!v39_char(&out, c)) goto falha;
            if (c == '\\' && i + 1 < n) {
                if (!v39_char(&out, src[i + 1])) goto falha;
                i += 2;
                continue;
            }
            if ((estado == DQ && c == '"') || (estado == SQ && c == '\''))
                estado = NORMAL;
            i++;
            continue;
        }
        if (estado == COM_LINHA) {
            if (!v39_char(&out, c)) goto falha;
            i++;
            if (c == '\n') estado = NORMAL;
            continue;
        }
        if (estado == COM_BLOCO) {
            if (c == '*' && i + 1 < n && src[i + 1] == '/') {
                if (!v39_txt(&out, src + i, 2)) goto falha;
                i += 2;
                estado = NORMAL;
            } else {
                if (!v39_char(&out, c)) goto falha;
                i++;
            }
            continue;
        }

        if (c == '"') { estado = DQ; if (!v39_char(&out, c)) goto falha; i++; continue; }
        if (c == '\'') { estado = SQ; if (!v39_char(&out, c)) goto falha; i++; continue; }
        if (c == '/' && i + 1 < n && src[i + 1] == '/') {
            estado = COM_LINHA;
            if (!v39_txt(&out, src + i, 2)) goto falha;
            i += 2;
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '*') {
            estado = COM_BLOCO;
            if (!v39_txt(&out, src + i, 2)) goto falha;
            i += 2;
            continue;
        }

        int r = v39_tenta_para_duplo(&out, src, n, &i, &contador);
        if (r < 0) goto falha;
        if (r > 0) continue;

        if (!v39_char(&out, c)) goto falha;
        i++;
    }

    if (!out.data) out.data = (char*)calloc(1, 1);
    return out.data;

falha:
    free(out.data);
    return NULL;
}

AstNode *leviata_parse_v39(const char *source, int *had_error) {
    char *normalizado = v39_normaliza_iteracao(source);
    if (!normalizado) {
        if (had_error) *had_error = 1;
        fprintf(stderr, "[LEVIATA][Parser] Sem memoria ao preparar iteracao 3.9\n");
        return NULL;
    }

    AstNode *ast = leviata_parse_v38(normalizado, had_error);
    free(normalizado);
    return ast;
}
