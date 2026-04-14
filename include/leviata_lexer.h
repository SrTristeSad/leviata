/* ============================================================================
 * MOTOR LEVIATA - LEXER (TOKENIZADOR)
 * ============================================================================ */
#ifndef LEVIATA_LEXER_H
#define LEVIATA_LEXER_H

#include "leviata_types.h"

/* ---- Tipos de Token ---- */
typedef enum {
    /* Literais */
    TOK_INT_LIT,    /* 42 */
    TOK_FLOAT_LIT,  /* 3.14 */
    TOK_STR_LIT,    /* "texto" */
    TOK_TRUE,       /* verdadeiro */
    TOK_FALSE,      /* falso */
    TOK_NULL,       /* nulo */

    /* Identificadores */
    TOK_IDENT,      /* nome de variavel, funcao etc */

    /* Palavras-chave */
    TOK_VAR,        /* var */
    TOK_CONST_KW,   /* const */
    TOK_FUNC,       /* funcao */
    TOK_RETORNAR,   /* retornar */
    TOK_SE,         /* se */
    TOK_SENAO,      /* senao */
    TOK_ENQUANTO,   /* enquanto */
    TOK_PARA,       /* para */
    TOK_DE,         /* de */
    TOK_ATE,        /* ate */
    TOK_QUEBRAR,    /* quebrar */
    TOK_CONTINUAR,  /* continuar */
    TOK_IMPORTAR,   /* importar */
    TOK_CLASSE,     /* classe */
    TOK_NOVO,       /* novo */
    TOK_ESTE,       /* este */
    TOK_HERDAR,     /* herdar */

    /* Operadores aritméticos */
    TOK_PLUS,       /* + */
    TOK_MINUS,      /* - */
    TOK_STAR,       /* * */
    TOK_SLASH,      /* / */
    TOK_PERCENT,    /* % */
    TOK_CARET,      /* ^ (potência) */

    /* Operadores de comparação */
    TOK_EQ,         /* == */
    TOK_NEQ,        /* != */
    TOK_LT,         /* < */
    TOK_GT,         /* > */
    TOK_LTE,        /* <= */
    TOK_GTE,        /* >= */

    /* Operadores lógicos */
    TOK_AND,        /* e / && */
    TOK_OR,         /* ou / || */
    TOK_NOT,        /* nao / ! */

    /* Operadores de atribuição */
    TOK_ASSIGN,     /* = */
    TOK_PLUS_EQ,    /* += */
    TOK_MINUS_EQ,   /* -= */
    TOK_STAR_EQ,    /* *= */
    TOK_SLASH_EQ,   /* /= */

    /* Incremento/Decremento */
    TOK_PLUSPLUS,   /* ++ */
    TOK_MINUSMINUS, /* -- */

    /* Delimitadores */
    TOK_LPAREN,     /* ( */
    TOK_RPAREN,     /* ) */
    TOK_LBRACE,     /* { */
    TOK_RBRACE,     /* } */
    TOK_LBRACKET,   /* [ */
    TOK_RBRACKET,   /* ] */
    TOK_COMMA,      /* , */
    TOK_SEMICOLON,  /* ; */
    TOK_COLON,      /* : */
    TOK_DOT,        /* . */

    /* Controle */
    TOK_EOF,
    TOK_ERROR
} TokenType;

/* ---- Estrutura do Token ---- */
typedef struct {
    TokenType type;
    char      lexeme[LV_MAX_STRING];
    int       line;
    int       col;

    /* Valores pré-parseados */
    int64_t   i_val;
    double    f_val;
} Token;

/* ---- Estado do Lexer ---- */
typedef struct {
    const char *source;
    size_t      length;
    size_t      pos;    /* posição atual */
    int         line;
    int         col;
    Token       current;
    Token       peek;
    int         has_peek;
} Lexer;

/* ---- API do Lexer ---- */
void  lexer_init(Lexer *lex, const char *source);
Token lexer_next(Lexer *lex);
Token lexer_peek_token(Lexer *lex);
const char *token_type_name(TokenType type);

#endif /* LEVIATA_LEXER_H */
