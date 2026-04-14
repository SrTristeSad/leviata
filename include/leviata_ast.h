/* ============================================================================
 * MOTOR LEVIATA - AST (ÁRVORE DE SINTAXE ABSTRATA)
 * ============================================================================ */
#ifndef LEVIATA_AST_H
#define LEVIATA_AST_H

#include "leviata_types.h"
#include "leviata_lexer.h"

/* ---- Tipos de Nó da AST ---- */
typedef enum {
    /* Literais */
    AST_INT_LIT,
    AST_FLOAT_LIT,
    AST_STR_LIT,
    AST_BOOL_LIT,
    AST_NULL_LIT,
    AST_LIST_LIT,
    AST_MAP_LIT,

    /* Identificadores e acesso */
    AST_IDENT,
    AST_INDEX,          /* a[i] */
    AST_FIELD,          /* obj.campo */

    /* Expressões */
    AST_BINOP,          /* a + b, a > b, a e b ... */
    AST_UNOP,           /* -a, nao a, !a */
    AST_ASSIGN,         /* a = expr */
    AST_COMPOUND_ASSIGN, /* a += expr */
    AST_CALL,           /* f(args...) */
    AST_METHOD_CALL,    /* obj.f(args...) */

    /* Declarações */
    AST_VAR_DECL,       /* var x = expr */
    AST_CONST_DECL,     /* const X = expr */
    AST_FUNC_DECL,      /* funcao f(params) { body } */
    AST_RETURN,         /* retornar expr */

    /* Controle de fluxo */
    AST_BLOCK,          /* { stmts... } */
    AST_IF,             /* se (cond) { ... } senao { ... } */
    AST_WHILE,          /* enquanto (cond) { ... } */
    AST_FOR_RANGE,      /* para i de 0 ate 10 { ... } */
    AST_BREAK,
    AST_CONTINUE,

    /* Módulo/Programa */
    AST_PROGRAM
} AstNodeType;

/* ---- Nó da AST ---- */
typedef struct AstNode {
    AstNodeType type;
    int         line;

    union {
        /* Literais */
        struct { int64_t value; } int_lit;
        struct { double  value; } float_lit;
        struct { char    value[LV_MAX_STRING]; } str_lit;
        struct { int     value; } bool_lit; /* 0=falso 1=verdadeiro */

        /* Identificador */
        struct { char name[LV_MAX_IDENTIFIER]; } ident;

        /* Operação binária */
        struct {
            TokenType        op;
            struct AstNode  *left;
            struct AstNode  *right;
        } binop;

        /* Operação unária */
        struct {
            TokenType        op;
            struct AstNode  *operand;
        } unop;

        /* Atribuição */
        struct {
            struct AstNode  *target;  /* pode ser ident, index, field */
            struct AstNode  *value;
            TokenType        op;      /* para compound assign */
        } assign;

        /* Chamada de função */
        struct {
            struct AstNode  *callee;
            struct AstNode **args;
            int              argc;
        } call;

        /* Declaração de variável */
        struct {
            char             name[LV_MAX_IDENTIFIER];
            struct AstNode  *init;   /* pode ser NULL */
        } var_decl;

        /* Declaração de função */
        struct {
            char             name[LV_MAX_IDENTIFIER];
            char           (*params)[LV_MAX_IDENTIFIER];
            int              param_count;
            struct AstNode  *body;
        } func_decl;

        /* Retorno */
        struct {
            struct AstNode  *value;  /* pode ser NULL */
        } ret;

        /* Bloco de statements */
        struct {
            struct AstNode **stmts;
            int              count;
        } block;

        /* Se/Senao */
        struct {
            struct AstNode  *condition;
            struct AstNode  *then_branch;
            struct AstNode  *else_branch;  /* pode ser NULL */
        } if_stmt;

        /* Enquanto */
        struct {
            struct AstNode  *condition;
            struct AstNode  *body;
        } while_stmt;

        /* Para de ate */
        struct {
            char             var_name[LV_MAX_IDENTIFIER];
            struct AstNode  *from;
            struct AstNode  *to;
            struct AstNode  *body;
        } for_range;

        /* Acesso a índice */
        struct {
            struct AstNode  *object;
            struct AstNode  *index;
        } index_access;

        /* Acesso a campo */
        struct {
            struct AstNode  *object;
            char             field[LV_MAX_IDENTIFIER];
        } field_access;

        /* Lista literal [a, b, c] */
        struct {
            struct AstNode **items;
            int              count;
        } list_lit;

        /* Programa raiz */
        struct {
            struct AstNode **stmts;
            int              count;
        } program;
    } as;
} AstNode;

/* ---- Alocador de Nós AST ---- */
AstNode *ast_alloc(AstNodeType type, int line);
void     ast_free(AstNode *node);
void     ast_print(AstNode *node, int indent);

#endif /* LEVIATA_AST_H */
