/* ============================================================================
 * LEVIATA 3.5 - CONSTRUTORES + SUPER
 *
 * Reaproveita o compilador 3.3 e mantém a transformação de construtores 3.4.
 * Antes de compilar, chamadas `super.metodo(...)` dentro de métodos são
 * transformadas em lookup explícito a partir da classe pai da classe onde o
 * método foi declarado. Isso evita recursão no override atual.
 * ============================================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define compiler_compile_program compiler_compile_program_v33
#include "leviata_compiler.c"
#undef compiler_compile_program

static int v35_erro = 0;

static AstNode *v35_ident(const char *nome, int linha) {
    AstNode *n = ast_alloc(AST_IDENT, linha);
    strncpy(n->as.ident.name, nome, LV_MAX_IDENTIFIER - 1);
    n->as.ident.name[LV_MAX_IDENTIFIER - 1] = '\0';
    return n;
}

static AstNode *v35_string(const char *valor, int linha) {
    AstNode *n = ast_alloc(AST_STR_LIT, linha);
    strncpy(n->as.str_lit.value, valor, LV_MAX_STRING - 1);
    n->as.str_lit.value[LV_MAX_STRING - 1] = '\0';
    return n;
}

static AstNode *v35_call(AstNode *callee, int argc, int linha) {
    AstNode *n = ast_alloc(AST_CALL, linha);
    n->as.call.callee = callee;
    n->as.call.argc = argc;
    n->as.call.args = NULL;
    if (argc > 0) {
        n->as.call.args = (AstNode**)calloc((size_t)argc, sizeof(AstNode*));
        if (!n->as.call.args) {
            fprintf(stderr, "[LEVIATA][Compilador] Sem memória para chamada interna\n");
            exit(EXIT_FAILURE);
        }
    }
    return n;
}

static AstNode *v35_field(AstNode *objeto, const char *campo, int linha) {
    AstNode *n = ast_alloc(AST_FIELD, linha);
    n->as.field_access.object = objeto;
    strncpy(n->as.field_access.field, campo, LV_MAX_IDENTIFIER - 1);
    n->as.field_access.field[LV_MAX_IDENTIFIER - 1] = '\0';
    return n;
}

/* --------------------------------------------------------------------------
 * SUPER
 * -------------------------------------------------------------------------- */
static int v35_e_super_ident(AstNode *n) {
    return n && n->type == AST_IDENT &&
           strcmp(n->as.ident.name, "super") == 0;
}

static void v35_super(AstNode *n,
                      const char *classe_atual,
                      int classe_tem_pai,
                      int dentro_metodo);

static void v35_super_call(AstNode *n,
                           const char *classe_atual,
                           int classe_tem_pai,
                           int dentro_metodo) {
    if (!n || (n->type != AST_CALL && n->type != AST_METHOD_CALL)) return;

    AstNode *callee = n->as.call.callee;
    if (callee && callee->type == AST_FIELD &&
        v35_e_super_ident(callee->as.field_access.object)) {
        if (!dentro_metodo || !classe_atual) {
            fprintf(stderr,
                    "[LEVIATA][Compilador] 'super' so pode ser usado dentro de metodo de classe (linha %d)\n",
                    n->line);
            v35_erro = 1;
            return;
        }
        if (!classe_tem_pai) {
            fprintf(stderr,
                    "[LEVIATA][Compilador] Classe '%s' nao possui pai para usar 'super' (linha %d)\n",
                    classe_atual, n->line);
            v35_erro = 1;
            return;
        }

        for (int i = 0; i < n->as.call.argc; i++)
            v35_super(n->as.call.args[i], classe_atual,
                      classe_tem_pai, dentro_metodo);

        char metodo[LV_MAX_IDENTIFIER];
        strncpy(metodo, callee->as.field_access.field, LV_MAX_IDENTIFIER - 1);
        metodo[LV_MAX_IDENTIFIER - 1] = '\0';

        AstNode *lookup = v35_call(
            v35_ident("@classe.super_metodo", n->line), 2, n->line);
        lookup->as.call.args[0] = v35_ident(classe_atual, n->line);
        lookup->as.call.args[1] = v35_string(metodo, n->line);

        int antigo_argc = n->as.call.argc;
        AstNode **antigos = n->as.call.args;
        AstNode **novos = (AstNode**)calloc(
            (size_t)(antigo_argc + 1), sizeof(AstNode*));
        if (!novos) {
            fprintf(stderr, "[LEVIATA][Compilador] Sem memoria para chamada de super\n");
            exit(EXIT_FAILURE);
        }
        novos[0] = v35_ident("este", n->line);
        for (int i = 0; i < antigo_argc; i++)
            novos[i + 1] = antigos[i];

        n->as.call.callee = lookup;
        n->as.call.args = novos;
        n->as.call.argc = antigo_argc + 1;

        free(antigos);
        ast_free(callee);
        return;
    }

    v35_super(callee, classe_atual, classe_tem_pai, dentro_metodo);
    for (int i = 0; i < n->as.call.argc; i++)
        v35_super(n->as.call.args[i], classe_atual,
                  classe_tem_pai, dentro_metodo);
}

static void v35_super(AstNode *n,
                      const char *classe_atual,
                      int classe_tem_pai,
                      int dentro_metodo) {
    if (!n || v35_erro) return;

    switch (n->type) {
        case AST_CALL:
        case AST_METHOD_CALL:
            v35_super_call(n, classe_atual, classe_tem_pai, dentro_metodo);
            break;

        case AST_FIELD:
            if (v35_e_super_ident(n->as.field_access.object)) {
                fprintf(stderr,
                        "[LEVIATA][Compilador] Use 'super.metodo(...)'; acesso direto a membro de super nao e suportado (linha %d)\n",
                        n->line);
                v35_erro = 1;
            } else {
                v35_super(n->as.field_access.object, classe_atual,
                          classe_tem_pai, dentro_metodo);
            }
            break;

        case AST_BINOP:
            v35_super(n->as.binop.left, classe_atual, classe_tem_pai, dentro_metodo);
            v35_super(n->as.binop.right, classe_atual, classe_tem_pai, dentro_metodo);
            break;
        case AST_UNOP:
            v35_super(n->as.unop.operand, classe_atual, classe_tem_pai, dentro_metodo);
            break;
        case AST_ASSIGN:
        case AST_COMPOUND_ASSIGN:
            v35_super(n->as.assign.target, classe_atual, classe_tem_pai, dentro_metodo);
            v35_super(n->as.assign.value, classe_atual, classe_tem_pai, dentro_metodo);
            break;
        case AST_INDEX:
            v35_super(n->as.index_access.object, classe_atual, classe_tem_pai, dentro_metodo);
            v35_super(n->as.index_access.index, classe_atual, classe_tem_pai, dentro_metodo);
            break;
        case AST_LIST_LIT:
            for (int i = 0; i < n->as.list_lit.count; i++)
                v35_super(n->as.list_lit.items[i], classe_atual,
                          classe_tem_pai, dentro_metodo);
            break;
        case AST_VAR_DECL:
        case AST_CONST_DECL:
            v35_super(n->as.var_decl.init, classe_atual,
                      classe_tem_pai, dentro_metodo);
            break;
        case AST_FUNC_DECL:
            v35_super(n->as.func_decl.body, classe_atual,
                      classe_tem_pai, dentro_metodo);
            break;
        case AST_CLASS_DECL:
            for (int i = 0; i < n->as.class_decl.member_count; i++) {
                AstNode *m = n->as.class_decl.members[i];
                if (!m) continue;
                if (m->type == AST_FUNC_DECL)
                    v35_super(m->as.func_decl.body,
                              n->as.class_decl.name,
                              n->as.class_decl.has_parent,
                              1);
                else
                    v35_super(m, n->as.class_decl.name,
                              n->as.class_decl.has_parent,
                              0);
            }
            break;
        case AST_RETURN:
            v35_super(n->as.ret.value, classe_atual, classe_tem_pai, dentro_metodo);
            break;
        case AST_BLOCK:
            for (int i = 0; i < n->as.block.count; i++)
                v35_super(n->as.block.stmts[i], classe_atual,
                          classe_tem_pai, dentro_metodo);
            break;
        case AST_IF:
            v35_super(n->as.if_stmt.condition, classe_atual, classe_tem_pai, dentro_metodo);
            v35_super(n->as.if_stmt.then_branch, classe_atual, classe_tem_pai, dentro_metodo);
            v35_super(n->as.if_stmt.else_branch, classe_atual, classe_tem_pai, dentro_metodo);
            break;
        case AST_WHILE:
            v35_super(n->as.while_stmt.condition, classe_atual, classe_tem_pai, dentro_metodo);
            v35_super(n->as.while_stmt.body, classe_atual, classe_tem_pai, dentro_metodo);
            break;
        case AST_FOR_RANGE:
            v35_super(n->as.for_range.from, classe_atual, classe_tem_pai, dentro_metodo);
            v35_super(n->as.for_range.to, classe_atual, classe_tem_pai, dentro_metodo);
            v35_super(n->as.for_range.body, classe_atual, classe_tem_pai, dentro_metodo);
            break;
        case AST_FOR_EACH:
            v35_super(n->as.for_each.collection, classe_atual, classe_tem_pai, dentro_metodo);
            v35_super(n->as.for_each.body, classe_atual, classe_tem_pai, dentro_metodo);
            break;
        case AST_PROGRAM:
            for (int i = 0; i < n->as.program.count; i++)
                v35_super(n->as.program.stmts[i], NULL, 0, 0);
            break;
        default:
            break;
    }
}

/* --------------------------------------------------------------------------
 * CONSTRUTORES 3.4
 * -------------------------------------------------------------------------- */
static AstNode *v35_cria_construtor(AstNode *classe) {
    if (!classe || classe->type != AST_CLASS_DECL) return NULL;

    AstNode *iniciar = NULL;
    for (int i = 0; i < classe->as.class_decl.member_count; i++) {
        AstNode *m = classe->as.class_decl.members[i];
        if (m && m->type == AST_FUNC_DECL &&
            strcmp(m->as.func_decl.name, "iniciar") == 0)
            iniciar = m;
    }

    char nome_ctor[LV_MAX_IDENTIFIER];
    int escrito = snprintf(nome_ctor, sizeof(nome_ctor),
                           "novo %s", classe->as.class_decl.name);
    if (escrito < 0 || (size_t)escrito >= sizeof(nome_ctor)) {
        fprintf(stderr,
                "[LEVIATA][Compilador] Nome da classe '%s' e longo demais para construtor\n",
                classe->as.class_decl.name);
        v35_erro = 1;
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
        if (!fn->as.func_decl.params) exit(EXIT_FAILURE);
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
    if (!bloco->as.block.stmts) exit(EXIT_FAILURE);

    AstNode *criar = v35_call(v35_ident("@classe.novo", classe->line), 1, classe->line);
    criar->as.call.args[0] = v35_ident(classe->as.class_decl.name, classe->line);

    AstNode *var = ast_alloc(AST_VAR_DECL, classe->line);
    strcpy(var->as.var_decl.name, "#instancia");
    var->as.var_decl.init = criar;
    bloco->as.block.stmts[0] = var;

    int pos = 1;
    if (iniciar) {
        AstNode *chamada = v35_call(
            v35_field(v35_ident("#instancia", iniciar->line),
                      "iniciar", iniciar->line),
            param_count, iniciar->line);
        for (int i = 0; i < param_count; i++)
            chamada->as.call.args[i] = v35_ident(fn->as.func_decl.params[i], iniciar->line);
        bloco->as.block.stmts[pos++] = chamada;
    }

    AstNode *ret = ast_alloc(AST_RETURN, classe->line);
    ret->as.ret.value = v35_ident("#instancia", classe->line);
    bloco->as.block.stmts[pos] = ret;
    fn->as.func_decl.body = bloco;
    return fn;
}

static void v35_transforma_novo(AstNode *n);

static void v35_transforma_chamada_novo(AstNode *n) {
    if (!n || (n->type != AST_CALL && n->type != AST_METHOD_CALL)) return;

    v35_transforma_novo(n->as.call.callee);
    for (int i = 0; i < n->as.call.argc; i++)
        v35_transforma_novo(n->as.call.args[i]);

    AstNode *callee = n->as.call.callee;
    if (!callee || callee->type != AST_IDENT ||
        strcmp(callee->as.ident.name, "__leviata_novo") != 0)
        return;

    if (n->as.call.argc < 1 || !n->as.call.args[0] ||
        n->as.call.args[0]->type != AST_STR_LIT) {
        fprintf(stderr, "[LEVIATA][Compilador] Forma interna invalida de 'novo'\n");
        v35_erro = 1;
        return;
    }

    const char *classe = n->as.call.args[0]->as.str_lit.value;
    char nome_ctor[LV_MAX_IDENTIFIER];
    int escrito = snprintf(nome_ctor, sizeof(nome_ctor), "novo %s", classe);
    if (escrito < 0 || (size_t)escrito >= sizeof(nome_ctor)) {
        v35_erro = 1;
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

static void v35_transforma_novo(AstNode *n) {
    if (!n || v35_erro) return;

    switch (n->type) {
        case AST_CALL:
        case AST_METHOD_CALL:
            v35_transforma_chamada_novo(n);
            break;
        case AST_BINOP:
            v35_transforma_novo(n->as.binop.left);
            v35_transforma_novo(n->as.binop.right);
            break;
        case AST_UNOP:
            v35_transforma_novo(n->as.unop.operand);
            break;
        case AST_ASSIGN:
        case AST_COMPOUND_ASSIGN:
            v35_transforma_novo(n->as.assign.target);
            v35_transforma_novo(n->as.assign.value);
            break;
        case AST_INDEX:
            v35_transforma_novo(n->as.index_access.object);
            v35_transforma_novo(n->as.index_access.index);
            break;
        case AST_FIELD:
            v35_transforma_novo(n->as.field_access.object);
            break;
        case AST_LIST_LIT:
            for (int i = 0; i < n->as.list_lit.count; i++)
                v35_transforma_novo(n->as.list_lit.items[i]);
            break;
        case AST_VAR_DECL:
        case AST_CONST_DECL:
            v35_transforma_novo(n->as.var_decl.init);
            break;
        case AST_FUNC_DECL:
            v35_transforma_novo(n->as.func_decl.body);
            break;
        case AST_CLASS_DECL:
            for (int i = 0; i < n->as.class_decl.member_count; i++)
                v35_transforma_novo(n->as.class_decl.members[i]);
            break;
        case AST_RETURN:
            v35_transforma_novo(n->as.ret.value);
            break;
        case AST_BLOCK:
            for (int i = 0; i < n->as.block.count; i++)
                v35_transforma_novo(n->as.block.stmts[i]);
            break;
        case AST_IF:
            v35_transforma_novo(n->as.if_stmt.condition);
            v35_transforma_novo(n->as.if_stmt.then_branch);
            v35_transforma_novo(n->as.if_stmt.else_branch);
            break;
        case AST_WHILE:
            v35_transforma_novo(n->as.while_stmt.condition);
            v35_transforma_novo(n->as.while_stmt.body);
            break;
        case AST_FOR_RANGE:
            v35_transforma_novo(n->as.for_range.from);
            v35_transforma_novo(n->as.for_range.to);
            v35_transforma_novo(n->as.for_range.body);
            break;
        case AST_FOR_EACH:
            v35_transforma_novo(n->as.for_each.collection);
            v35_transforma_novo(n->as.for_each.body);
            break;
        case AST_PROGRAM:
            for (int i = 0; i < n->as.program.count; i++)
                v35_transforma_novo(n->as.program.stmts[i]);
            break;
        default:
            break;
    }
}

static int v35_prepara_programa(AstNode *programa) {
    if (!programa || programa->type != AST_PROGRAM) return 1;

    v35_erro = 0;
    v35_super(programa, NULL, 0, 0);
    if (v35_erro) return 0;

    for (int i = 0; i < programa->as.program.count; i++)
        v35_transforma_novo(programa->as.program.stmts[i]);
    if (v35_erro) return 0;

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
    if (!novos) return 0;

    int pos = 0;
    for (int i = 0; i < antigo_count; i++) {
        AstNode *stmt = antigos[i];
        novos[pos++] = stmt;
        if (stmt && stmt->type == AST_CLASS_DECL) {
            AstNode *ctor = v35_cria_construtor(stmt);
            if (!ctor || v35_erro) {
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
    if (!v35_prepara_programa(program)) {
        c->had_error = 1;
        chunk_init(out);
        return 0;
    }
    return compiler_compile_program_v33(c, program, out);
}
