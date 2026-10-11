/* ============================================================================
 * LEVIATA 3 - AST (ÁRVORE DE SINTAXE ABSTRATA)
 * ============================================================================ */
#ifndef LEVIATA_AST_H
#define LEVIATA_AST_H

#include "leviata_types.h"
#include "leviata_lexer.h"

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
    AST_INDEX,
    AST_FIELD,

    /* Expressões */
    AST_BINOP,
    AST_UNOP,
    AST_ASSIGN,
    AST_COMPOUND_ASSIGN,
    AST_CALL,
    AST_METHOD_CALL,
    AST_NEW,

    /* Declarações */
    AST_VAR_DECL,
    AST_CONST_DECL,
    AST_FUNC_DECL,
    AST_CLASS_DECL,
    AST_RETURN,

    /* Controle de fluxo */
    AST_BLOCK,
    AST_IF,
    AST_WHILE,
    AST_FOR_RANGE,
    AST_FOR_EACH,
    AST_BREAK,
    AST_CONTINUE,

    AST_PROGRAM
} AstNodeType;

typedef struct AstNode {
    AstNodeType type;
    int line;

    union {
        struct { int64_t value; } int_lit;
        struct { double value; } float_lit;
        struct { char value[LV_MAX_STRING]; } str_lit;
        struct { int value; } bool_lit;

        struct { char name[LV_MAX_IDENTIFIER]; } ident;

        struct {
            TokenType op;
            struct AstNode *left;
            struct AstNode *right;
        } binop;

        struct {
            TokenType op;
            struct AstNode *operand;
        } unop;

        struct {
            struct AstNode *target;
            struct AstNode *value;
            TokenType op;
        } assign;

        struct {
            struct AstNode *callee;
            struct AstNode **args;
            int argc;
        } call;

        struct {
            char class_name[LV_MAX_IDENTIFIER];
        } new_expr;

        struct {
            char name[LV_MAX_IDENTIFIER];
            struct AstNode *init;
        } var_decl;

        struct {
            char name[LV_MAX_IDENTIFIER];
            char (*params)[LV_MAX_IDENTIFIER];
            int param_count;
            struct AstNode *body;
        } func_decl;

        struct {
            char name[LV_MAX_IDENTIFIER];
            char parent[LV_MAX_IDENTIFIER];
            int has_parent;
            struct AstNode **members;
            int member_count;
        } class_decl;

        struct { struct AstNode *value; } ret;

        struct {
            struct AstNode **stmts;
            int count;
        } block;

        struct {
            struct AstNode *condition;
            struct AstNode *then_branch;
            struct AstNode *else_branch;
        } if_stmt;

        struct {
            struct AstNode *condition;
            struct AstNode *body;
        } while_stmt;

        struct {
            char var_name[LV_MAX_IDENTIFIER];
            struct AstNode *from;
            struct AstNode *to;
            struct AstNode *body;
        } for_range;

        struct {
            char var_name[LV_MAX_IDENTIFIER];
            struct AstNode *collection;
            struct AstNode *body;
        } for_each;

        struct {
            struct AstNode *object;
            struct AstNode *index;
        } index_access;

        struct {
            struct AstNode *object;
            char field[LV_MAX_IDENTIFIER];
        } field_access;

        struct {
            struct AstNode **items;
            int count;
        } list_lit;

        struct {
            struct AstNode **stmts;
            int count;
        } program;
    } as;
} AstNode;

AstNode *ast_alloc(AstNodeType type, int line);
void ast_free(AstNode *node);
void ast_print(AstNode *node, int indent);

#endif /* LEVIATA_AST_H */