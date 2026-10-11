/* ============================================================================
 * LEVIATA 3.8 - COMPILADOR DE INDEXACAO GENERICA
 *
 * Esta camada prepara a AST e delega para o compilador 3.5. O runtime 3.8
 * chama explicitamente `compiler_compile_program_v38`, sem substituir nem
 * duplicar a implementacao madura do compilador anterior.
 * ============================================================================ */
#include "leviata_compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AstNode *v38_ident(const char *nome, int linha) {
    AstNode *n = ast_alloc(AST_IDENT, linha);
    strncpy(n->as.ident.name, nome, LV_MAX_IDENTIFIER - 1);
    n->as.ident.name[LV_MAX_IDENTIFIER - 1] = '\0';
    return n;
}

static AstNode *v38_texto(const char *texto, int linha) {
    AstNode *n = ast_alloc(AST_STR_LIT, linha);
    strncpy(n->as.str_lit.value, texto, LV_MAX_STRING - 1);
    n->as.str_lit.value[LV_MAX_STRING - 1] = '\0';
    return n;
}

static void v38_vira_chamada(AstNode *n,
                             const char *nome,
                             AstNode **args,
                             int argc) {
    int linha = n->line;
    memset(&n->as, 0, sizeof(n->as));
    n->type = AST_CALL;
    n->line = linha;
    n->as.call.callee = v38_ident(nome, linha);
    n->as.call.args = args;
    n->as.call.argc = argc;
}

static const char *v38_operador(TokenType op) {
    switch (op) {
        case TOK_PLUS: return "+";
        case TOK_MINUS: return "-";
        case TOK_STAR: return "*";
        case TOK_SLASH: return "/";
        default: return NULL;
    }
}

static void v38_indices(AstNode **slot);

static void v38_indice_leitura(AstNode *n) {
    AstNode *objeto = n->as.index_access.object;
    AstNode *indice = n->as.index_access.index;

    v38_indices(&objeto);
    v38_indices(&indice);

    AstNode **args = (AstNode**)calloc(2, sizeof(AstNode*));
    if (!args) exit(EXIT_FAILURE);
    args[0] = objeto;
    args[1] = indice;
    v38_vira_chamada(n, "@indice.obter", args, 2);
}

static void v38_atribuicao_indice(AstNode *n) {
    AstNode *target = n->as.assign.target;
    AstNode *value = n->as.assign.value;
    int linha = n->line;

    AstNode *objeto = target->as.index_access.object;
    AstNode *indice = target->as.index_access.index;
    v38_indices(&objeto);
    v38_indices(&indice);

    /* O parser representa a[i] += x como:
     * assign(target=a[i], value=binop(left=target, right=x)). */
    if (value && value->type == AST_BINOP && value->as.binop.left == target) {
        const char *op = v38_operador(value->as.binop.op);
        if (op) {
            AstNode *rhs = value->as.binop.right;
            v38_indices(&rhs);

            value->as.binop.left = NULL;
            value->as.binop.right = NULL;
            free(value);

            target->as.index_access.object = NULL;
            target->as.index_access.index = NULL;
            free(target);

            AstNode **args = (AstNode**)calloc(4, sizeof(AstNode*));
            if (!args) exit(EXIT_FAILURE);
            args[0] = objeto;
            args[1] = indice;
            args[2] = v38_texto(op, linha);
            args[3] = rhs;
            v38_vira_chamada(n, "@indice.composto", args, 4);
            return;
        }
    }

    v38_indices(&value);
    target->as.index_access.object = NULL;
    target->as.index_access.index = NULL;
    free(target);

    AstNode **args = (AstNode**)calloc(3, sizeof(AstNode*));
    if (!args) exit(EXIT_FAILURE);
    args[0] = objeto;
    args[1] = indice;
    args[2] = value;
    v38_vira_chamada(n, "@indice.definir", args, 3);
}

static void v38_indices(AstNode **slot) {
    if (!slot || !*slot) return;
    AstNode *n = *slot;

    switch (n->type) {
        case AST_INDEX:
            v38_indice_leitura(n);
            break;

        case AST_ASSIGN:
        case AST_COMPOUND_ASSIGN:
            if (n->as.assign.target && n->as.assign.target->type == AST_INDEX) {
                v38_atribuicao_indice(n);
            } else {
                v38_indices(&n->as.assign.target);
                v38_indices(&n->as.assign.value);
            }
            break;

        case AST_BINOP:
            v38_indices(&n->as.binop.left);
            v38_indices(&n->as.binop.right);
            break;

        case AST_UNOP:
            v38_indices(&n->as.unop.operand);
            break;

        case AST_CALL:
        case AST_METHOD_CALL:
            v38_indices(&n->as.call.callee);
            for (int i = 0; i < n->as.call.argc; i++)
                v38_indices(&n->as.call.args[i]);
            break;

        case AST_VAR_DECL:
        case AST_CONST_DECL:
            v38_indices(&n->as.var_decl.init);
            break;

        case AST_FUNC_DECL:
            v38_indices(&n->as.func_decl.body);
            break;

        case AST_CLASS_DECL:
            for (int i = 0; i < n->as.class_decl.member_count; i++)
                v38_indices(&n->as.class_decl.members[i]);
            break;

        case AST_RETURN:
            v38_indices(&n->as.ret.value);
            break;

        case AST_BLOCK:
            for (int i = 0; i < n->as.block.count; i++)
                v38_indices(&n->as.block.stmts[i]);
            break;

        case AST_IF:
            v38_indices(&n->as.if_stmt.condition);
            v38_indices(&n->as.if_stmt.then_branch);
            v38_indices(&n->as.if_stmt.else_branch);
            break;

        case AST_WHILE:
            v38_indices(&n->as.while_stmt.condition);
            v38_indices(&n->as.while_stmt.body);
            break;

        case AST_FOR_RANGE:
            v38_indices(&n->as.for_range.from);
            v38_indices(&n->as.for_range.to);
            v38_indices(&n->as.for_range.body);
            break;

        case AST_FOR_EACH:
            v38_indices(&n->as.for_each.collection);
            v38_indices(&n->as.for_each.body);
            break;

        case AST_FIELD:
            v38_indices(&n->as.field_access.object);
            break;

        case AST_LIST_LIT:
            for (int i = 0; i < n->as.list_lit.count; i++)
                v38_indices(&n->as.list_lit.items[i]);
            break;

        case AST_PROGRAM:
            for (int i = 0; i < n->as.program.count; i++)
                v38_indices(&n->as.program.stmts[i]);
            break;

        default:
            break;
    }
}

int compiler_compile_program_v38(Compiler *c, AstNode *program, Chunk *out) {
    v38_indices(&program);
    return compiler_compile_program(c, program, out);
}
