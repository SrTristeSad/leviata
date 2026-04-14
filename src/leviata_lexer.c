/* ============================================================================
 * MOTOR LEVIATA - IMPLEMENTAÇÃO DO LEXER
 * ============================================================================ */
#include "leviata_lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---- Tabela de Palavras-chave ---- */
typedef struct { const char *word; TokenType type; } Keyword;

static const Keyword KEYWORDS[] = {
    {"var",         TOK_VAR},
    {"const",       TOK_CONST_KW},
    {"funcao",      TOK_FUNC},
    {"retornar",    TOK_RETORNAR},
    {"se",          TOK_SE},
    {"senao",       TOK_SENAO},
    {"enquanto",    TOK_ENQUANTO},
    {"para",        TOK_PARA},
    {"de",          TOK_DE},
    {"ate",         TOK_ATE},
    {"quebrar",     TOK_QUEBRAR},
    {"continuar",   TOK_CONTINUAR},
    {"importar",    TOK_IMPORTAR},
    {"classe",      TOK_CLASSE},
    {"novo",        TOK_NOVO},
    {"este",        TOK_ESTE},
    {"herdar",      TOK_HERDAR},
    {"verdadeiro",  TOK_TRUE},
    {"falso",       TOK_FALSE},
    {"nulo",        TOK_NULL},
    {"e",           TOK_AND},
    {"ou",          TOK_OR},
    {"nao",         TOK_NOT},
    {NULL, TOK_EOF}
};

static char lex_peek_char(Lexer *lex) {
    if (lex->pos >= lex->length) return '\0';
    return lex->source[lex->pos];
}

static char lex_peek_next(Lexer *lex) {
    if (lex->pos + 1 >= lex->length) return '\0';
    return lex->source[lex->pos + 1];
}

static char lex_advance(Lexer *lex) {
    char c = lex->source[lex->pos++];
    if (c == '\n') { lex->line++; lex->col = 0; }
    else { lex->col++; }
    return c;
}

static void lex_skip_whitespace(Lexer *lex) {
    for (;;) {
        char c = lex_peek_char(lex);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            lex_advance(lex);
        } else if (c == '/' && lex_peek_next(lex) == '/') {
            /* Comentário de linha */
            while (lex_peek_char(lex) != '\n' && lex->pos < lex->length)
                lex_advance(lex);
        } else if (c == '/' && lex_peek_next(lex) == '*') {
            /* Comentário de bloco */
            lex_advance(lex); lex_advance(lex);
            while (lex->pos < lex->length) {
                if (lex_peek_char(lex) == '*' && lex_peek_next(lex) == '/') {
                    lex_advance(lex); lex_advance(lex);
                    break;
                }
                lex_advance(lex);
            }
        } else {
            break;
        }
    }
}

static Token make_token(Lexer *lex, TokenType type, const char *lexeme) {
    Token t;
    t.type = type;
    t.line = lex->line;
    t.col  = lex->col;
    t.i_val = 0;
    t.f_val = 0.0;
    strncpy(t.lexeme, lexeme ? lexeme : "", LV_MAX_STRING - 1);
    t.lexeme[LV_MAX_STRING - 1] = '\0';
    return t;
}

static Token lex_read_number(Lexer *lex) {
    char buf[64];
    int len = 0;
    int is_float = 0;

    while (isdigit(lex_peek_char(lex)) && len < 63) {
        buf[len++] = lex_advance(lex);
    }
    if (lex_peek_char(lex) == '.' && isdigit(lex_peek_next(lex))) {
        is_float = 1;
        buf[len++] = lex_advance(lex); /* . */
        while (isdigit(lex_peek_char(lex)) && len < 63)
            buf[len++] = lex_advance(lex);
    }
    buf[len] = '\0';

    Token t = make_token(lex, is_float ? TOK_FLOAT_LIT : TOK_INT_LIT, buf);
    if (is_float) t.f_val = atof(buf);
    else          t.i_val = (int64_t)atoll(buf);
    return t;
}

static Token lex_read_string(Lexer *lex) {
    char delim = lex_advance(lex); /* consume ' ou " */
    char buf[LV_MAX_STRING];
    int  len = 0;

    while (lex->pos < lex->length) {
        char c = lex_peek_char(lex);
        if (c == delim) { lex_advance(lex); break; }
        if (c == '\\') {
            lex_advance(lex);
            char esc = lex_advance(lex);
            switch (esc) {
                case 'n':  buf[len++] = '\n'; break;
                case 't':  buf[len++] = '\t'; break;
                case '\\': buf[len++] = '\\'; break;
                case '"':  buf[len++] = '"'; break;
                case '\'': buf[len++] = '\''; break;
                default:   buf[len++] = '\\'; buf[len++] = esc; break;
            }
        } else {
            buf[len++] = lex_advance(lex);
        }
        if (len >= LV_MAX_STRING - 1) break;
    }
    buf[len] = '\0';
    return make_token(lex, TOK_STR_LIT, buf);
}

static Token lex_read_ident(Lexer *lex) {
    char buf[LV_MAX_IDENTIFIER];
    int  len = 0;

    while ((isalnum(lex_peek_char(lex)) || lex_peek_char(lex) == '_') && len < LV_MAX_IDENTIFIER - 1) {
        buf[len++] = lex_advance(lex);
    }
    buf[len] = '\0';

    /* Verifica se é palavra-chave */
    for (int i = 0; KEYWORDS[i].word != NULL; i++) {
        if (strcmp(buf, KEYWORDS[i].word) == 0)
            return make_token(lex, KEYWORDS[i].type, buf);
    }

    return make_token(lex, TOK_IDENT, buf);
}

void lexer_init(Lexer *lex, const char *source) {
    lex->source    = source;
    lex->length    = strlen(source);
    lex->pos       = 0;
    lex->line      = 1;
    lex->col       = 0;
    lex->has_peek  = 0;
}

Token lexer_next(Lexer *lex) {
    if (lex->has_peek) {
        lex->has_peek = 0;
        return lex->peek;
    }

    lex_skip_whitespace(lex);

    if (lex->pos >= lex->length)
        return make_token(lex, TOK_EOF, "");

    char c = lex_peek_char(lex);

    /* Número */
    if (isdigit(c)) return lex_read_number(lex);

    /* String */
    if (c == '"' || c == '\'') return lex_read_string(lex);

    /* Identificador ou palavra-chave */
    if (isalpha(c) || c == '_') return lex_read_ident(lex);

    /* Operadores e símbolos */
    lex_advance(lex);
    char next = lex_peek_char(lex);

    switch (c) {
        case '+':
            if (next == '+') { lex_advance(lex); return make_token(lex, TOK_PLUSPLUS, "++"); }
            if (next == '=') { lex_advance(lex); return make_token(lex, TOK_PLUS_EQ, "+="); }
            return make_token(lex, TOK_PLUS, "+");
        case '-':
            if (next == '-') { lex_advance(lex); return make_token(lex, TOK_MINUSMINUS, "--"); }
            if (next == '=') { lex_advance(lex); return make_token(lex, TOK_MINUS_EQ, "-="); }
            return make_token(lex, TOK_MINUS, "-");
        case '*':
            if (next == '=') { lex_advance(lex); return make_token(lex, TOK_STAR_EQ, "*="); }
            return make_token(lex, TOK_STAR, "*");
        case '/':
            if (next == '=') { lex_advance(lex); return make_token(lex, TOK_SLASH_EQ, "/="); }
            return make_token(lex, TOK_SLASH, "/");
        case '%': return make_token(lex, TOK_PERCENT, "%");
        case '^': return make_token(lex, TOK_CARET, "^");
        case '=':
            if (next == '=') { lex_advance(lex); return make_token(lex, TOK_EQ, "=="); }
            return make_token(lex, TOK_ASSIGN, "=");
        case '!':
            if (next == '=') { lex_advance(lex); return make_token(lex, TOK_NEQ, "!="); }
            return make_token(lex, TOK_NOT, "!");
        case '<':
            if (next == '=') { lex_advance(lex); return make_token(lex, TOK_LTE, "<="); }
            return make_token(lex, TOK_LT, "<");
        case '>':
            if (next == '=') { lex_advance(lex); return make_token(lex, TOK_GTE, ">="); }
            return make_token(lex, TOK_GT, ">");
        case '&':
            if (next == '&') { lex_advance(lex); return make_token(lex, TOK_AND, "&&"); }
            break;
        case '|':
            if (next == '|') { lex_advance(lex); return make_token(lex, TOK_OR, "||"); }
            break;
        case '(': return make_token(lex, TOK_LPAREN, "(");
        case ')': return make_token(lex, TOK_RPAREN, ")");
        case '{': return make_token(lex, TOK_LBRACE, "{");
        case '}': return make_token(lex, TOK_RBRACE, "}");
        case '[': return make_token(lex, TOK_LBRACKET, "[");
        case ']': return make_token(lex, TOK_RBRACKET, "]");
        case ',': return make_token(lex, TOK_COMMA, ",");
        case ';': return make_token(lex, TOK_SEMICOLON, ";");
        case ':': return make_token(lex, TOK_COLON, ":");
        case '.': return make_token(lex, TOK_DOT, ".");
        default: break;
    }

    char err[4] = {c, '\0'};
    return make_token(lex, TOK_ERROR, err);
}

Token lexer_peek_token(Lexer *lex) {
    if (!lex->has_peek) {
        lex->peek = lexer_next(lex);
        lex->has_peek = 1;
    }
    return lex->peek;
}

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOK_INT_LIT:    return "INTEIRO";
        case TOK_FLOAT_LIT:  return "DECIMAL";
        case TOK_STR_LIT:    return "TEXTO";
        case TOK_TRUE:       return "verdadeiro";
        case TOK_FALSE:      return "falso";
        case TOK_NULL:       return "nulo";
        case TOK_IDENT:      return "IDENTIFICADOR";
        case TOK_VAR:        return "var";
        case TOK_CONST_KW:   return "const";
        case TOK_FUNC:       return "funcao";
        case TOK_RETORNAR:   return "retornar";
        case TOK_SE:         return "se";
        case TOK_SENAO:      return "senao";
        case TOK_ENQUANTO:   return "enquanto";
        case TOK_PARA:       return "para";
        case TOK_DE:         return "de";
        case TOK_ATE:        return "ate";
        case TOK_QUEBRAR:    return "quebrar";
        case TOK_CONTINUAR:  return "continuar";
        case TOK_IMPORTAR:   return "importar";
        case TOK_PLUS:       return "+";
        case TOK_MINUS:      return "-";
        case TOK_STAR:       return "*";
        case TOK_SLASH:      return "/";
        case TOK_PERCENT:    return "%";
        case TOK_CARET:      return "^";
        case TOK_EQ:         return "==";
        case TOK_NEQ:        return "!=";
        case TOK_LT:         return "<";
        case TOK_GT:         return ">";
        case TOK_LTE:        return "<=";
        case TOK_GTE:        return ">=";
        case TOK_AND:        return "e";
        case TOK_OR:         return "ou";
        case TOK_NOT:        return "nao";
        case TOK_ASSIGN:     return "=";
        case TOK_LPAREN:     return "(";
        case TOK_RPAREN:     return ")";
        case TOK_LBRACE:     return "{";
        case TOK_RBRACE:     return "}";
        case TOK_LBRACKET:   return "[";
        case TOK_RBRACKET:   return "]";
        case TOK_COMMA:      return ",";
        case TOK_SEMICOLON:  return ";";
        case TOK_DOT:        return ".";
        case TOK_EOF:        return "EOF";
        case TOK_ERROR:      return "ERRO";
        default:             return "?";
    }
}
