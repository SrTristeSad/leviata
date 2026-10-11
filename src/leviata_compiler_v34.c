/* ============================================================================
 * LEVIATA 3.4 - CAMADA DE COMPILAÇÃO DE CONSTRUTORES
 *
 * Reaproveita integralmente o compilador 3.3. Antes da compilação:
 *   1. __leviata_novo("Pessoa", args...) vira chamada a "novo Pessoa".
 *   2. Cada classe recebe uma função construtora interna "novo Pessoa".
 *   3. A construtora cria a instância, chama iniciar(args...) quando a classe
 *      declara esse método e sempre retorna a própria instância.
 *
 * Construtores não são herdados nesta versão. Uma classe filha que deseja
 * argumentos de construção declara seu próprio iniciar(...).
 * ============================================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define compiler_compile_program compiler_compile_program_v33
#include "leviata_compiler.c"
#undef compiler_compile_program

static int v34_erro = 0;

static AstNode *v34_ident(const char *nome, int linha) {
    AstNode *n = ast_alloc(AST_IDENT, linha);
    strncpy(n->as.ident.name, nome, LV_MAX_IDENTIFIER - 1);
    n->as.ident.name[LV_MAX_IDENTIFIER - 1] = '\0';
    return n;
}

static AstNode *v34_call(AstNode *callee, int argc, int linha) {
    AstNode *n = ast_alloc(AST_CALL, linha);
    n->as.call.callee = callee;
    n->as.call.argc = argc;
    if (argc > 0) {
        n->as.call.args = (AstNode**)calloc((size_t)argc, sizeof(AstNode*));
        if (!n->as.call.args) {
            fprintf(stderr, "[LEVIATA][Compilador] Sem memória para construtor\n");
            exit(EXIT_FAILURE);
        }
    }
    return n;
}

static AstNode *v34_field(AstNode *objeto, const char *campo, int linha) {
    AstNode *n = ast_alloc(AST_FIELD, linha);
    n->as.field_access.object = objeto;
    strncpy(n->as.field_access.field, campo, LV_MAX_IDENTIFIER - 1);
    n->as.field_access.field[LV_MAX_IDENTIFIER - 1] = '\0';
    return n;
}

static AstNode *v34_cria_construtor(AstNode *classe) {
    if (!classe || classe->type != AST_CLASS_DECL) return NULL;

    AstNode *iniciar = NULL;
    for (int i = 0; i < classe->as.class_decl.member_count; i++) {
        AstNode *m = classe->as.class_decl.members[i];
        if (m && m->type == AST_FUNC_DECL &&
            strcmp(m->as.func_decl.name, "iniciar") == 0)
            iniciar = m; /* o último método com esse nome vence, como no runtime */
    }

    char nome_ctor[LV_MAX_IDENTIFIER];
    int escrito = snprintf(nome_ctor, sizeof(nome_ctor),
                           "novo %s", classe->as.class_decl.name);
    if (escrito < 0 || (size_t)escrito >= sizeof(nome_ctor)) {
        fprintf(stderr,
                "[LEVIATA][Compilador] Nome da classe '%s' é longo demais para construtor\n",
                classe->as.class_decl.name);
        v34_erro = 1;
        return NULL;
    }

    AstNode *fn = ast_alloc(AST_FUNC_DECL, classe->line);
    strncpy(fn->as.func_decl.name, nome_ctor, LV_MAX_IDENTIFIER - 1);
    fn->as.func_decl.name[LV_MAX_IDENTIFIER - 1] = '\0';

    int param_count = iniciar ? iniciar->as.func_decl.param_count : 0;
    fn->as.func_decl.param_count = param_count;
    fn->as.func_decl.params = NULL;
    if (param_count > 0) {
        fn->as.func_decl.params = (char(*)[LV_MAX_IDENTIFIER])calloc(
            (size_t)param_count, LV_MAX_IDENTIFIER);
        if (!fn->as.func_decl.params) {
            fprintf(stderr, "[LEVIATA][Compilador] Sem memória para parâmetros do construtor\n");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < param_count; i++) {
            strncpy(fn->as.func_decl.params[i],
                    iniciar->as.func_decl.params[i], LV_MAX_IDENTIFIER - 1);
            fn->as.func_decl.params[i][LV_MAX_IDENTIFIER - 1] = '\0';
        }
    }

    AstNode *bloco = ast_alloc(AST_BLOCK, classe->line);
    bloco->as.block.count = iniciar ? 3 : 2;
    bloco->as.block.stmts = (AstNode**)calloc(
        (size_t)bloco->as.block.count, sizeof(AstNode*));
    if (!bloco->as.block.stmts) {
        fprintf(stderr, "[LEVIATA][Compilador] Sem memória para corpo do construtor\n");
        exit(EXIT_FAILURE);
    }

    /* var #instancia = @classe.novo(Classe) */
    AstNode *criar = v34_call(v34_ident("@classe.novo", classe->line), 1, classe->line);
    criar->as.call.args[0] = v34_ident(classe->as.class_decl.name, classe->line);

    AstNode *var = ast_alloc(AST_VAR_DECL, classe->line);
    strcpy(var->as.var_decl.name, "#instancia");
    var->as.var_decl.init = criar;
    bloco->as.block.stmts[0] = var;

    int pos = 1;
    if (iniciar) {
        AstNode *chamada = v34_call(
            v34_field(v34_ident("#instancia", iniciar->line),
                      "iniciar", iniciar->line),
            param_count, iniciar->line);
        for (int i = 0; i < param_count; i++)
            chamada->as.call.args[i] = v34_ident(fn->as.func_decl.params[i], iniciar->line);
        bloco->as.block.stmts[pos++] = chamada;
    }

    AstNode *ret = ast_alloc(AST_RETURN, classe->line);
    ret->as.ret.value = v34_ident("#instancia", classe->line);
    bloco->as.block.stmts[pos] = ret;

    fn->as.func_decl.body = bloco;
    return fn;
}

static void v34_transforma(AstNode *n);

static void v34_transforma_chamada(AstNode *n) {
    if (!n || n->type != AST_CALL) return;

    v34_transforma(n->as.call.callee);
    for (int i = 0; i < n->as.call.argc; i++)
        v34_transforma(n->as.call.args[i]);

    AstNode *callee = n->as.call.callee;
    if (!callee || callee->type != AST_IDENT ||
        strcmp(callee->as.ident.name, "__leviata_novo") != 0)
        return;

    if (n->as.call.argc < 1 || !n->as.call.args[0] ||
        n->as.call.args[0]->type != AST_STR_LIT) {
        fprintf(stderr,
                "[LEVIATA][Compilador] Forma interna inválida de 'novo'\n");
        v34_erro = 1;
        return;
    }

    const char *classe = n->as.call.args[0]->as.str_lit.value;
    char nome_ctor[LV_MAX_IDENTIFIER];
    int escrito = snprintf(nome_ctor, sizeof(nome_ctor), "novo %s", classe);
    if (escrito < 0 || (size_t)escrito >= sizeof(nome_ctor)) {
        fprintf(stderr,
                "[LEVIATA][Compilador] Nome da classe '%s' é longo demais para construtor\n",
                classe);
        v34_erro = 1;
        return;
    }

    strncpy(callee->as.ident.name, nome_ctor, LV_MAX_IDENTIFIER - 1);
    callee->as.ident.name[LV_MAX_IDENTIFIER - 1] = '\0';

    AstNode *nome_classe = n->as.call.args[0];
    for (int i = 1; i < n->as.call.argc; i++)
        n->as.call.args[i - 1] = n->as.call.args[i];
    n->as.call.argc--;
    ast_free(nome_classe);
}

static void v34_transforma(AstNode *n) {
    if (!n) return;

    switch (n->type) {
        case AST_CALL:
        case AST_METHOD_CALL:
            v34_transforma_chamada(n);
            break;

        case AST_BINOP:
            v34_transforma(n->as.binop.left);
            v34_transforma(n->as.binop.right);
            break;
        case AST_UNOP:
            v34_transforma(n->as.unop.operand);
            break;
        case AST_ASSIGN:
        case AST_COMPOUND_ASSIGN:
            v34_transforma(n->as.assign.target);
            v34_transforma(n->as.assign.value);
            break;
        case AST_INDEX:
            v34_transforma(n->as.index_access.object);
            v34_transforma(n->as.index_access.index);
            break;
        case AST_FIELD:
            v34_transforma(n->as.field_access.object);
            break;
        case AST_LIST_LIT:
            for (int i = 0; i < n->as.list_lit.count; i++)
                v34_transforma(n->as.list_lit.items[i]);
            break;
        case AST_VAR_DECL:
        case AST_CONST_DECL:
            v34_transforma(n->as.var_decl.init);
            break;
        case AST_FUNC_DECL:
            v34_transforma(n->as.func_decl.body);
            break;
        case AST_CLASS_DECL:
            for (int i = 0; i < n->as.class_decl.member_count; i++)
                v34_transforma(n->as.class_decl.members[i]);
            break;
        case AST_RETURN:
            v34_transforma(n->as.ret.value);
            break;
        case AST_BLOCK:
            for (int i = 0; i < n->as.block.count; i++)
                v34_transforma(n->as.block.stmts[i]);
            break;
        case AST_IF:
            v34_transforma(n->as.if_stmt.condition);
            v34_transforma(n->as.if_stmt.then_branch);
            v34_transforma(n->as.if_stmt.else_branch);
            break;
        case AST_WHILE:
            v34_transforma(n->as.while_stmt.condition);
            v34_transforma(n->as.while_stmt.body);
            break;
        case AST_FOR_RANGE:
            v34_transforma(n->as.for_range.from);
            v34_transforma(n->as.for_range.to);
            v34_transforma(n->as.for_range.body);
            break;
        case AST_FOR_EACH:
            v34_transforma(n->as.for_each.collection);
            v34_transforma(n->as.for_each.body);
            break;
        case AST_PROGRAM:
            for (int i = 0; i < n->as.program.count; i++)
                v34_transforma(n->as.program.stmts[i]);
            break;
        default:
            break;
    }
}

static int v34_prepara_programa(AstNode *programa) {
    if (!programa || programa->type != AST_PROGRAM) return 1;

    v34_erro = 0;
    for (int i = 0; i < programa->as.program.count; i++)
        v34_transforma(programa->as.program.stmts[i]);
    if (v34_erro) return 0;

    int classes = 0;
    for (int i = 0; i < programa->as.program.count; i++) {
        if (programa->as.program.stmts[i] &&
            programa->as.program.stmts[i]->type == AST_CLASS_DECL)
            classes++;
    }
    if (classes == 0) return 1;

    int antigo_count = programa->as.program.count;
    AstNode **antigos = programa->as.program.stmts;
    AstNode **novos = (AstNode**)calloc(
        (size_t)(antigo_count + classes), sizeof(AstNode*));
    if (!novos) {
        fprintf(stderr, "[LEVIATA][Compilador] Sem memória ao preparar construtores\n");
        return 0;
    }

    int pos = 0;
    for (int i = 0; i < antigo_count; i++) {
        AstNode *stmt = antigos[i];
        novos[pos++] = stmt;
        if (stmt && stmt->type == AST_CLASS_DECL) {
            AstNode *ctor = v34_cria_construtor(stmt);
            if (!ctor || v34_erro) {
                free(novos);
                return 0;
            }
            novos[pos++] = ctor;
        }
    }

    free(antigos);
    programa->as.program.stmts = novos;
    programa->as.program.count = pos;
    return 1;
}

int compiler_compile_program(Compiler *c, AstNode *program, Chunk *out) {
    if (!v34_prepara_programa(program)) {
        c->had_error = 1;
        chunk_init(out);
        return 0;
    }
    return compiler_compile_program_v33(c, program, out);
}
