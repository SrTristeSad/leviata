/* ============================================================================
 * MOTOR LEVIATA - IMPLEMENTAÇÃO DA AST
 * ============================================================================ */
#include "leviata_ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

AstNode *ast_alloc(AstNodeType type, int line) {
    AstNode *n = (AstNode*)calloc(1, sizeof(AstNode));
    if (!n) {
        fprintf(stderr, "[LEVIATA] Falha de alocação de nó AST\n");
        exit(EXIT_FAILURE);
    }
    n->type = type;
    n->line = line;
    return n;
}

void ast_free(AstNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_BINOP:
            ast_free(node->as.binop.left);
            ast_free(node->as.binop.right);
            break;
        case AST_UNOP:
            ast_free(node->as.unop.operand);
            break;
        case AST_ASSIGN:
        case AST_COMPOUND_ASSIGN:
            ast_free(node->as.assign.target);
            ast_free(node->as.assign.value);
            break;
        case AST_CALL:
        case AST_METHOD_CALL:
            ast_free(node->as.call.callee);
            for (int i = 0; i < node->as.call.argc; i++)
                ast_free(node->as.call.args[i]);
            free(node->as.call.args);
            break;
        case AST_VAR_DECL:
        case AST_CONST_DECL:
            ast_free(node->as.var_decl.init);
            break;
        case AST_FUNC_DECL:
            free(node->as.func_decl.params);
            ast_free(node->as.func_decl.body);
            break;
        case AST_RETURN:
            ast_free(node->as.ret.value);
            break;
        case AST_BLOCK:
        case AST_PROGRAM:
            for (int i = 0; i < node->as.block.count; i++)
                ast_free(node->as.block.stmts[i]);
            free(node->as.block.stmts);
            break;
        case AST_IF:
            ast_free(node->as.if_stmt.condition);
            ast_free(node->as.if_stmt.then_branch);
            ast_free(node->as.if_stmt.else_branch);
            break;
        case AST_WHILE:
            ast_free(node->as.while_stmt.condition);
            ast_free(node->as.while_stmt.body);
            break;
        case AST_FOR_RANGE:
            ast_free(node->as.for_range.from);
            ast_free(node->as.for_range.to);
            ast_free(node->as.for_range.body);
            break;
        case AST_INDEX:
            ast_free(node->as.index_access.object);
            ast_free(node->as.index_access.index);
            break;
        case AST_FIELD:
            ast_free(node->as.field_access.object);
            break;
        case AST_LIST_LIT:
            for (int i = 0; i < node->as.list_lit.count; i++)
                ast_free(node->as.list_lit.items[i]);
            free(node->as.list_lit.items);
            break;
        default:
            break;
    }

    free(node);
}

static void print_indent(int n) {
    for (int i = 0; i < n; i++) printf("  ");
}

void ast_print(AstNode *node, int indent) {
    if (!node) return;
    print_indent(indent);

    switch (node->type) {
        case AST_INT_LIT:
            printf("(int %lld)\n", (long long)node->as.int_lit.value);
            break;
        case AST_FLOAT_LIT:
            printf("(float %g)\n", node->as.float_lit.value);
            break;
        case AST_STR_LIT:
            printf("(string \"%s\")\n", node->as.str_lit.value);
            break;
        case AST_BOOL_LIT:
            printf("(bool %s)\n", node->as.bool_lit.value ? "verdadeiro" : "falso");
            break;
        case AST_NULL_LIT:
            printf("(nulo)\n");
            break;
        case AST_IDENT:
            printf("(ident %s)\n", node->as.ident.name);
            break;
        case AST_BINOP:
            printf("(binop '%s')\n", token_type_name(node->as.binop.op));
            ast_print(node->as.binop.left,  indent + 1);
            ast_print(node->as.binop.right, indent + 1);
            break;
        case AST_UNOP:
            printf("(unop '%s')\n", token_type_name(node->as.unop.op));
            ast_print(node->as.unop.operand, indent + 1);
            break;
        case AST_ASSIGN:
            printf("(assign)\n");
            ast_print(node->as.assign.target, indent + 1);
            ast_print(node->as.assign.value,  indent + 1);
            break;
        case AST_VAR_DECL:
            printf("(var %s)\n", node->as.var_decl.name);
            ast_print(node->as.var_decl.init, indent + 1);
            break;
        case AST_CONST_DECL:
            printf("(const %s)\n", node->as.var_decl.name);
            ast_print(node->as.var_decl.init, indent + 1);
            break;
        case AST_FUNC_DECL:
            printf("(funcao %s(%d params))\n", node->as.func_decl.name, node->as.func_decl.param_count);
            ast_print(node->as.func_decl.body, indent + 1);
            break;
        case AST_CALL:
            printf("(chamada)\n");
            ast_print(node->as.call.callee, indent + 1);
            for (int i = 0; i < node->as.call.argc; i++)
                ast_print(node->as.call.args[i], indent + 2);
            break;
        case AST_RETURN:
            printf("(retornar)\n");
            ast_print(node->as.ret.value, indent + 1);
            break;
        case AST_BLOCK:
            printf("(bloco %d stmts)\n", node->as.block.count);
            for (int i = 0; i < node->as.block.count; i++)
                ast_print(node->as.block.stmts[i], indent + 1);
            break;
        case AST_IF:
            printf("(se)\n");
            ast_print(node->as.if_stmt.condition,   indent + 1);
            ast_print(node->as.if_stmt.then_branch,  indent + 1);
            if (node->as.if_stmt.else_branch) {
                print_indent(indent + 1); printf("(senao)\n");
                ast_print(node->as.if_stmt.else_branch, indent + 2);
            }
            break;
        case AST_WHILE:
            printf("(enquanto)\n");
            ast_print(node->as.while_stmt.condition, indent + 1);
            ast_print(node->as.while_stmt.body,      indent + 1);
            break;
        case AST_FOR_RANGE:
            printf("(para %s de ate)\n", node->as.for_range.var_name);
            ast_print(node->as.for_range.from, indent + 1);
            ast_print(node->as.for_range.to,   indent + 1);
            ast_print(node->as.for_range.body, indent + 1);
            break;
        case AST_PROGRAM:
            printf("(programa %d stmts)\n", node->as.program.count);
            for (int i = 0; i < node->as.program.count; i++)
                ast_print(node->as.program.stmts[i], indent + 1);
            break;
        default:
            printf("(nó %d)\n", node->type);
            break;
    }
}
