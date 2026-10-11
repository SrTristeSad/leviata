/* ============================================================================
 * LEVIATA 3.2 - IMPLEMENTAÇÃO DO COMPILADOR (AST -> BYTECODE)
 * ============================================================================ */
#include "leviata_compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * Emissão de bytecode
 * ============================================================ */
static void emit(Compiler *c, uint8_t op, int32_t operand, int line) {
    chunk_write(c->chunk, op, operand, line);
}

static int emit_jump(Compiler *c, uint8_t op, int line) {
    emit(c, op, 0xFFFF, line);
    return c->chunk->count - 1;
}

static void patch_jump_to(Compiler *c, int idx, int target) {
    if (idx >= 0 && idx < c->chunk->count)
        c->chunk->code[idx].operand = target;
}

static void patch_jump(Compiler *c, int idx) {
    patch_jump_to(c, idx, c->chunk->count);
}

static int add_string_const(Compiler *c, const char *s) {
    ConstValue cv;
    cv.type = LV_TYPE_STRING;
    strncpy(cv.as.s_val, s, LV_MAX_STRING - 1);
    cv.as.s_val[LV_MAX_STRING - 1] = '\0';
    return chunk_add_constant(c->chunk, cv);
}

static int add_int_const(Compiler *c, int64_t v) {
    ConstValue cv;
    cv.type = LV_TYPE_INT;
    cv.as.i_val = v;
    return chunk_add_constant(c->chunk, cv);
}

static int add_float_const(Compiler *c, double v) {
    ConstValue cv;
    cv.type = LV_TYPE_FLOAT;
    cv.as.f_val = v;
    return chunk_add_constant(c->chunk, cv);
}

static void emit_load_string(Compiler *c, const char *s, int line) {
    int idx = add_string_const(c, s);
    emit(c, OP_LOAD_STRING, idx, line);
}

/* ============================================================
 * Escopos locais
 * ============================================================ */
static int resolve_local(Compiler *c, const char *name) {
    for (int i = c->local_count - 1; i >= 0; i--) {
        if (strcmp(c->locals[i].name, name) == 0)
            return c->locals[i].slot;
    }
    return -1;
}

static int declare_local(Compiler *c, const char *name) {
    if (c->local_count >= LV_LOCAL_MAX) {
        fprintf(stderr, "[LEVIATA][Compilador] Muitas variáveis locais\n");
        c->had_error = 1;
        return -1;
    }

    for (int i = c->local_count - 1; i >= 0; i--) {
        if (c->locals[i].depth < c->scope_depth) break;
        if (c->locals[i].depth == c->scope_depth &&
            strcmp(c->locals[i].name, name) == 0) {
            fprintf(stderr,
                    "[LEVIATA][Compilador] Variável local '%s' já declarada neste escopo\n",
                    name);
            c->had_error = 1;
            return -1;
        }
    }

    int slot = c->local_count;
    LocalVar *lv = &c->locals[c->local_count++];
    strncpy(lv->name, name, LV_MAX_IDENTIFIER - 1);
    lv->name[LV_MAX_IDENTIFIER - 1] = '\0';
    lv->depth = c->scope_depth;
    lv->slot = slot;
    return slot;
}

static void begin_scope(Compiler *c) {
    c->scope_depth++;
}

static void end_scope(Compiler *c, int line) {
    if (c->scope_depth <= 0) return;

    while (c->local_count > 0 &&
           c->locals[c->local_count - 1].depth >= c->scope_depth) {
        emit(c, OP_POP, 0, line);
        c->local_count--;
    }
    c->scope_depth--;
}

static void emit_unwind_to_depth(Compiler *c, int target_depth, int line) {
    for (int i = c->local_count - 1; i >= 0; i--) {
        if (c->locals[i].depth <= target_depth) break;
        emit(c, OP_POP, 0, line);
    }
}

/* ============================================================
 * Contexto de laços
 * ============================================================ */
static LoopContext *loop_push(Compiler *c, int scope_depth) {
    if (c->loop_count >= LV_LOOP_MAX) {
        fprintf(stderr, "[LEVIATA][Compilador] Laços aninhados demais\n");
        c->had_error = 1;
        return NULL;
    }

    LoopContext *loop = &c->loops[c->loop_count++];
    memset(loop, 0, sizeof(*loop));
    loop->scope_depth = scope_depth;
    return loop;
}

static LoopContext *loop_current(Compiler *c) {
    if (c->loop_count <= 0) return NULL;
    return &c->loops[c->loop_count - 1];
}

static void loop_pop(Compiler *c) {
    if (c->loop_count > 0) c->loop_count--;
}

static void loop_add_break(Compiler *c, LoopContext *loop, int jump) {
    if (!loop) return;
    if (loop->break_count >= LV_LOOP_JUMP_MAX) {
        fprintf(stderr, "[LEVIATA][Compilador] Muitos 'pare' no mesmo laço\n");
        c->had_error = 1;
        return;
    }
    loop->break_jumps[loop->break_count++] = jump;
}

static void loop_add_continue(Compiler *c, LoopContext *loop, int jump) {
    if (!loop) return;
    if (loop->continue_count >= LV_LOOP_JUMP_MAX) {
        fprintf(stderr, "[LEVIATA][Compilador] Muitos 'continue' no mesmo laço\n");
        c->had_error = 1;
        return;
    }
    loop->continue_jumps[loop->continue_count++] = jump;
}

static void loop_patch_breaks(Compiler *c, LoopContext *loop, int target) {
    if (!loop) return;
    for (int i = 0; i < loop->break_count; i++)
        patch_jump_to(c, loop->break_jumps[i], target);
}

static void loop_patch_continues(Compiler *c, LoopContext *loop, int target) {
    if (!loop) return;
    for (int i = 0; i < loop->continue_count; i++)
        patch_jump_to(c, loop->continue_jumps[i], target);
}

/* ============================================================
 * Nomes qualificados da API
 * ============================================================ */
static int qualified_name(AstNode *node, char *out, size_t out_size) {
    if (!node || !out || out_size == 0) return 0;

    if (node->type == AST_IDENT) {
        size_t len = strlen(node->as.ident.name);
        if (len + 1 > out_size) return 0;
        memcpy(out, node->as.ident.name, len + 1);
        return 1;
    }

    if (node->type == AST_FIELD) {
        char base[LV_MAX_STRING];
        if (!qualified_name(node->as.field_access.object, base, sizeof(base)))
            return 0;

        int written = snprintf(out, out_size, "%s.%s",
                               base, node->as.field_access.field);
        return written >= 0 && (size_t)written < out_size;
    }

    return 0;
}

static void emit_load_global_name(Compiler *c, const char *name, int line) {
    int idx = add_string_const(c, name);
    emit(c, OP_LOAD_GLOBAL, idx, line);
}

static int field_is_static_global(Compiler *c, AstNode *field,
                                  char *name, size_t name_size) {
    if (!field || field->type != AST_FIELD) return 0;
    if (!qualified_name(field, name, name_size)) return 0;
    return hmap_has(&c->vm->globals, name);
}

static int resource_constructor_expr(AstNode *node) {
    if (!node || node->type != AST_CALL || !node->as.call.callee ||
        node->as.call.callee->type != AST_IDENT)
        return 0;

    const char *name = node->as.call.callee->as.ident.name;
    return strcmp(name, "imagem") == 0 ||
           strcmp(name, "som") == 0 ||
           strcmp(name, "arquivo") == 0;
}

static int safe_repeat_receiver(AstNode *node) {
    if (!node) return 0;
    if (node->type == AST_IDENT || node->type == AST_FIELD)
        return 1;
    return resource_constructor_expr(node);
}

static void compile_node(Compiler *c, AstNode *node);
static void compile_expr(Compiler *c, AstNode *node);

/* ============================================================
 * Funções compiladas
 * ============================================================ */
static int compile_function_global(Compiler *c,
                                   const char *global_name,
                                   const char *display_name,
                                   AstNode *fn,
                                   int implicit_this) {
    if (!fn || fn->type != AST_FUNC_DECL) return 0;

    LvVM *vm = c->vm;
    int arity = fn->as.func_decl.param_count + (implicit_this ? 1 : 0);

    if (vm->func_count >= LV_GLOBAL_MAX) {
        fprintf(stderr, "[LEVIATA][Compilador] Limite de funções atingido\n");
        c->had_error = 1;
        return 0;
    }

    Chunk *fn_chunk = (Chunk*)malloc(sizeof(Chunk));
    if (!fn_chunk) {
        fprintf(stderr, "[LEVIATA][Compilador] Sem memória para função '%s'\n",
                display_name);
        c->had_error = 1;
        return 0;
    }

    chunk_init(fn_chunk);
    fn_chunk->arity = arity;

    Compiler fn_c;
    compiler_init(&fn_c, vm, c);
    fn_c.chunk = fn_chunk;
    fn_c.scope_depth = 0;

    if (implicit_this)
        declare_local(&fn_c, "este");

    for (int i = 0; i < fn->as.func_decl.param_count; i++)
        declare_local(&fn_c, fn->as.func_decl.params[i]);

    compile_node(&fn_c, fn->as.func_decl.body);

    if (fn_c.had_error) {
        c->had_error = 1;
        chunk_free(fn_chunk);
        free(fn_chunk);
        return 0;
    }

    emit(&fn_c, OP_LOAD_NULL, 0, fn->line);
    emit(&fn_c, OP_RETURN, 0, fn->line);

    int func_idx = vm->func_count;
    vm->functions[func_idx] = fn_chunk;
    strncpy(vm->func_names[func_idx], display_name, LV_MAX_IDENTIFIER - 1);
    vm->func_names[func_idx][LV_MAX_IDENTIFIER - 1] = '\0';
    vm->func_count++;

    int name_idx = add_string_const(c, global_name);
    emit(c, OP_LOAD_FUNC, func_idx, fn->line);
    emit(c, OP_STORE_GLOBAL, name_idx, fn->line);
    emit(c, OP_POP, 0, fn->line);
    return 1;
}

/* ============================================================
 * Expressões
 * ============================================================ */
static void compile_expr(Compiler *c, AstNode *node) {
    if (!node || c->had_error) return;

    switch (node->type) {
        case AST_INT_LIT: {
            int idx = add_int_const(c, node->as.int_lit.value);
            emit(c, OP_LOAD_INT, idx, node->line);
            break;
        }

        case AST_FLOAT_LIT: {
            int idx = add_float_const(c, node->as.float_lit.value);
            emit(c, OP_LOAD_FLOAT, idx, node->line);
            break;
        }

        case AST_STR_LIT: {
            int idx = add_string_const(c, node->as.str_lit.value);
            emit(c, OP_LOAD_STRING, idx, node->line);
            break;
        }

        case AST_BOOL_LIT:
            emit(c, node->as.bool_lit.value ? OP_LOAD_TRUE : OP_LOAD_FALSE,
                 0, node->line);
            break;

        case AST_NULL_LIT:
            emit(c, OP_LOAD_NULL, 0, node->line);
            break;

        case AST_IDENT: {
            int slot = resolve_local(c, node->as.ident.name);
            if (slot >= 0)
                emit(c, OP_LOAD_LOCAL, slot, node->line);
            else
                emit_load_global_name(c, node->as.ident.name, node->line);
            break;
        }

        case AST_NEW:
            emit_load_global_name(c, "@classe.novo", node->line);
            emit_load_global_name(c, node->as.new_expr.class_name, node->line);
            emit(c, OP_CALL, 1, node->line);
            break;

        case AST_FIELD: {
            char public_name[LV_MAX_STRING];
            if (field_is_static_global(c, node, public_name, sizeof(public_name))) {
                emit_load_global_name(c, public_name, node->line);
                break;
            }

            emit_load_global_name(c, "@obj.campo", node->line);
            compile_expr(c, node->as.field_access.object);
            emit_load_string(c, node->as.field_access.field, node->line);
            emit(c, OP_CALL, 2, node->line);
            break;
        }

        case AST_BINOP: {
            if (node->as.binop.op == TOK_AND) {
                compile_expr(c, node->as.binop.left);
                int jmp_false = emit_jump(c, OP_JUMP_IF_FALSE, node->line);

                compile_expr(c, node->as.binop.right);
                emit(c, OP_NOT, 0, node->line);
                emit(c, OP_NOT, 0, node->line);
                int jmp_end = emit_jump(c, OP_JUMP, node->line);

                patch_jump(c, jmp_false);
                emit(c, OP_LOAD_FALSE, 0, node->line);
                patch_jump(c, jmp_end);
                break;
            }

            if (node->as.binop.op == TOK_OR) {
                compile_expr(c, node->as.binop.left);
                int jmp_true = emit_jump(c, OP_JUMP_IF_TRUE, node->line);

                compile_expr(c, node->as.binop.right);
                emit(c, OP_NOT, 0, node->line);
                emit(c, OP_NOT, 0, node->line);
                int jmp_end = emit_jump(c, OP_JUMP, node->line);

                patch_jump(c, jmp_true);
                emit(c, OP_LOAD_TRUE, 0, node->line);
                patch_jump(c, jmp_end);
                break;
            }

            compile_expr(c, node->as.binop.left);
            compile_expr(c, node->as.binop.right);

            switch (node->as.binop.op) {
                case TOK_PLUS:    emit(c, OP_ADD, 0, node->line); break;
                case TOK_MINUS:   emit(c, OP_SUB, 0, node->line); break;
                case TOK_STAR:    emit(c, OP_MUL, 0, node->line); break;
                case TOK_SLASH:   emit(c, OP_DIV, 0, node->line); break;
                case TOK_PERCENT: emit(c, OP_MOD, 0, node->line); break;
                case TOK_CARET:   emit(c, OP_POW, 0, node->line); break;
                case TOK_EQ:      emit(c, OP_EQ, 0, node->line); break;
                case TOK_NEQ:     emit(c, OP_NEQ, 0, node->line); break;
                case TOK_LT:      emit(c, OP_LT, 0, node->line); break;
                case TOK_GT:      emit(c, OP_GT, 0, node->line); break;
                case TOK_LTE:     emit(c, OP_LTE, 0, node->line); break;
                case TOK_GTE:     emit(c, OP_GTE, 0, node->line); break;
                default:
                    fprintf(stderr, "[LEVIATA][Compilador] Operador binário desconhecido\n");
                    c->had_error = 1;
                    break;
            }
            break;
        }

        case AST_UNOP:
            compile_expr(c, node->as.unop.operand);
            if (node->as.unop.op == TOK_MINUS)
                emit(c, OP_NEG, 0, node->line);
            else if (node->as.unop.op == TOK_NOT)
                emit(c, OP_NOT, 0, node->line);
            else {
                fprintf(stderr, "[LEVIATA][Compilador] Operador unário desconhecido\n");
                c->had_error = 1;
            }
            break;

        case AST_ASSIGN: {
            AstNode *target = node->as.assign.target;

            if (target->type == AST_IDENT) {
                compile_expr(c, node->as.assign.value);
                int slot = resolve_local(c, target->as.ident.name);
                if (slot >= 0) {
                    emit(c, OP_STORE_LOCAL, slot, node->line);
                } else {
                    int idx = add_string_const(c, target->as.ident.name);
                    emit(c, OP_STORE_GLOBAL, idx, node->line);
                }
            } else if (target->type == AST_INDEX) {
                compile_expr(c, target->as.index_access.object);
                compile_expr(c, target->as.index_access.index);
                compile_expr(c, node->as.assign.value);
                emit(c, OP_INDEX_SET, 0, node->line);
            } else if (target->type == AST_FIELD) {
                char public_name[LV_MAX_STRING];
                if (field_is_static_global(c, target,
                                           public_name, sizeof(public_name))) {
                    fprintf(stderr,
                            "[LEVIATA][Compilador] API estática '%s' não pode receber atribuição\n",
                            public_name);
                    c->had_error = 1;
                    break;
                }

                emit_load_global_name(c, "@obj.definir", node->line);
                compile_expr(c, target->as.field_access.object);
                emit_load_string(c, target->as.field_access.field, node->line);
                compile_expr(c, node->as.assign.value);
                emit(c, OP_CALL, 3, node->line);
            } else {
                fprintf(stderr, "[LEVIATA][Compilador] Alvo de atribuição inválido\n");
                c->had_error = 1;
            }
            break;
        }

        case AST_CALL:
        case AST_METHOD_CALL: {
            AstNode *callee = node->as.call.callee;

            if (callee && callee->type == AST_FIELD) {
                char public_name[LV_MAX_STRING];
                if (!field_is_static_global(c, callee,
                                            public_name, sizeof(public_name))) {
                    AstNode *receiver = callee->as.field_access.object;
                    if (!safe_repeat_receiver(receiver)) {
                        fprintf(stderr,
                                "[LEVIATA][Compilador] Guarde o receptor em 'var' antes de chamar o método '%s' (linha %d)\n",
                                callee->as.field_access.field, node->line);
                        c->had_error = 1;
                        break;
                    }

                    /* Busca o método pelo receptor. O receptor é emitido outra
                     * vez apenas em formas estáveis (identificador/campo ou
                     * construtor de recurso), pois ele será o argumento 'este'. */
                    emit_load_global_name(c, "@obj.metodo", node->line);
                    compile_expr(c, receiver);
                    emit_load_string(c, callee->as.field_access.field, node->line);
                    emit(c, OP_CALL, 2, node->line);

                    compile_expr(c, receiver);
                    for (int i = 0; i < node->as.call.argc; i++)
                        compile_expr(c, node->as.call.args[i]);
                    emit(c, OP_CALL, node->as.call.argc + 1, node->line);
                    break;
                }
            }

            compile_expr(c, callee);
            for (int i = 0; i < node->as.call.argc; i++)
                compile_expr(c, node->as.call.args[i]);
            emit(c, OP_CALL, node->as.call.argc, node->line);
            break;
        }

        case AST_INDEX:
            compile_expr(c, node->as.index_access.object);
            compile_expr(c, node->as.index_access.index);
            emit(c, OP_INDEX_GET, 0, node->line);
            break;

        case AST_LIST_LIT:
            emit(c, OP_NEW_LIST, 0, node->line);
            for (int i = 0; i < node->as.list_lit.count; i++) {
                compile_expr(c, node->as.list_lit.items[i]);
                emit(c, OP_LIST_PUSH, 0, node->line);
            }
            break;

        default:
            compile_node(c, node);
            break;
    }
}

/* ============================================================
 * Statements
 * ============================================================ */
static void compile_node(Compiler *c, AstNode *node) {
    if (!node || c->had_error) return;

    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->as.program.count; i++)
                compile_node(c, node->as.program.stmts[i]);
            break;

        case AST_BLOCK:
            begin_scope(c);
            for (int i = 0; i < node->as.block.count; i++)
                compile_node(c, node->as.block.stmts[i]);
            end_scope(c, node->line);
            break;

        case AST_VAR_DECL:
        case AST_CONST_DECL: {
            if (node->as.var_decl.init)
                compile_expr(c, node->as.var_decl.init);
            else
                emit(c, OP_LOAD_NULL, 0, node->line);

            if (c->scope_depth > 0) {
                declare_local(c, node->as.var_decl.name);
            } else {
                int idx = add_string_const(c, node->as.var_decl.name);
                emit(c, OP_STORE_GLOBAL, idx, node->line);
                emit(c, OP_POP, 0, node->line);
            }
            break;
        }

        case AST_FUNC_DECL:
            compile_function_global(c,
                                    node->as.func_decl.name,
                                    node->as.func_decl.name,
                                    node,
                                    0);
            break;

        case AST_CLASS_DECL: {
            if (c->scope_depth != 0) {
                fprintf(stderr,
                        "[LEVIATA][Compilador] Classes devem ser declaradas no escopo principal nesta versão\n");
                c->had_error = 1;
                break;
            }

            /* Classe = @classe.criar("Nome", Pai|nulo). */
            emit_load_global_name(c, "@classe.criar", node->line);
            emit_load_string(c, node->as.class_decl.name, node->line);
            if (node->as.class_decl.has_parent)
                emit_load_global_name(c, node->as.class_decl.parent, node->line);
            else
                emit(c, OP_LOAD_NULL, 0, node->line);
            emit(c, OP_CALL, 2, node->line);

            int class_name_idx = add_string_const(c, node->as.class_decl.name);
            emit(c, OP_STORE_GLOBAL, class_name_idx, node->line);
            emit(c, OP_POP, 0, node->line);

            for (int i = 0; i < node->as.class_decl.member_count; i++) {
                AstNode *member = node->as.class_decl.members[i];
                if (!member) continue;

                if (member->type == AST_VAR_DECL ||
                    member->type == AST_CONST_DECL) {
                    emit_load_global_name(c, "@classe.campo", member->line);
                    emit_load_global_name(c, node->as.class_decl.name, member->line);
                    emit_load_string(c, member->as.var_decl.name, member->line);
                    if (member->as.var_decl.init)
                        compile_expr(c, member->as.var_decl.init);
                    else
                        emit(c, OP_LOAD_NULL, 0, member->line);
                    emit(c, OP_CALL, 3, member->line);
                    emit(c, OP_POP, 0, member->line);
                    continue;
                }

                if (member->type == AST_FUNC_DECL) {
                    char hidden_name[LV_MAX_IDENTIFIER];
                    char display_name[LV_MAX_IDENTIFIER];
                    int n1 = snprintf(hidden_name, sizeof(hidden_name),
                                      "@classefn.%s.%s",
                                      node->as.class_decl.name,
                                      member->as.func_decl.name);
                    int n2 = snprintf(display_name, sizeof(display_name),
                                      "%s.%s",
                                      node->as.class_decl.name,
                                      member->as.func_decl.name);
                    if (n1 < 0 || (size_t)n1 >= sizeof(hidden_name) ||
                        n2 < 0 || (size_t)n2 >= sizeof(display_name)) {
                        fprintf(stderr,
                                "[LEVIATA][Compilador] Nome de método muito longo\n");
                        c->had_error = 1;
                        break;
                    }

                    if (!compile_function_global(c,
                                                 hidden_name,
                                                 display_name,
                                                 member,
                                                 1))
                        break;

                    emit_load_global_name(c, "@classe.metodo", member->line);
                    emit_load_global_name(c, node->as.class_decl.name, member->line);
                    emit_load_string(c, member->as.func_decl.name, member->line);
                    emit_load_global_name(c, hidden_name, member->line);
                    emit(c, OP_CALL, 3, member->line);
                    emit(c, OP_POP, 0, member->line);
                }
            }
            break;
        }

        case AST_RETURN:
            if (node->as.ret.value)
                compile_expr(c, node->as.ret.value);
            else
                emit(c, OP_LOAD_NULL, 0, node->line);
            emit(c, OP_RETURN, 0, node->line);
            break;

        case AST_IF: {
            compile_expr(c, node->as.if_stmt.condition);
            int jmp_else = emit_jump(c, OP_JUMP_IF_FALSE, node->line);
            compile_node(c, node->as.if_stmt.then_branch);

            if (node->as.if_stmt.else_branch) {
                int jmp_end = emit_jump(c, OP_JUMP, node->line);
                patch_jump(c, jmp_else);
                compile_node(c, node->as.if_stmt.else_branch);
                patch_jump(c, jmp_end);
            } else {
                patch_jump(c, jmp_else);
            }
            break;
        }

        case AST_WHILE: {
            int loop_start = c->chunk->count;
            compile_expr(c, node->as.while_stmt.condition);
            int jmp_end = emit_jump(c, OP_JUMP_IF_FALSE, node->line);

            LoopContext *loop = loop_push(c, c->scope_depth);
            compile_node(c, node->as.while_stmt.body);

            loop_patch_continues(c, loop, loop_start);
            emit(c, OP_JUMP, loop_start, node->line);

            int exit_target = c->chunk->count;
            patch_jump_to(c, jmp_end, exit_target);
            loop_patch_breaks(c, loop, exit_target);
            loop_pop(c);
            break;
        }

        case AST_FOR_RANGE: {
            begin_scope(c);

            compile_expr(c, node->as.for_range.from);
            int var_slot = declare_local(c, node->as.for_range.var_name);

            int loop_start = c->chunk->count;
            emit(c, OP_LOAD_LOCAL, var_slot, node->line);
            compile_expr(c, node->as.for_range.to);
            emit(c, OP_LTE, 0, node->line);
            int jmp_end = emit_jump(c, OP_JUMP_IF_FALSE, node->line);

            LoopContext *loop = loop_push(c, c->scope_depth);
            compile_node(c, node->as.for_range.body);

            int continue_target = c->chunk->count;
            loop_patch_continues(c, loop, continue_target);

            emit(c, OP_LOAD_LOCAL, var_slot, node->line);
            int one = add_int_const(c, 1);
            emit(c, OP_LOAD_INT, one, node->line);
            emit(c, OP_ADD, 0, node->line);
            emit(c, OP_STORE_LOCAL, var_slot, node->line);
            emit(c, OP_POP, 0, node->line);
            emit(c, OP_JUMP, loop_start, node->line);

            int exit_target = c->chunk->count;
            patch_jump_to(c, jmp_end, exit_target);
            loop_patch_breaks(c, loop, exit_target);
            loop_pop(c);

            end_scope(c, node->line);
            break;
        }

        case AST_FOR_EACH: {
            begin_scope(c);

            compile_expr(c, node->as.for_each.collection);
            char hidden_collection[LV_MAX_IDENTIFIER];
            snprintf(hidden_collection, sizeof(hidden_collection),
                     "#colecao_%d_%d", node->line, c->local_count);
            int collection_slot = declare_local(c, hidden_collection);

            int zero = add_int_const(c, 0);
            emit(c, OP_LOAD_INT, zero, node->line);
            char hidden_index[LV_MAX_IDENTIFIER];
            snprintf(hidden_index, sizeof(hidden_index),
                     "#indice_%d_%d", node->line, c->local_count);
            int index_slot = declare_local(c, hidden_index);

            emit(c, OP_LOAD_NULL, 0, node->line);
            int item_slot = declare_local(c, node->as.for_each.var_name);

            int loop_start = c->chunk->count;

            emit(c, OP_LOAD_LOCAL, index_slot, node->line);
            emit_load_global_name(c, "tamanho", node->line);
            emit(c, OP_LOAD_LOCAL, collection_slot, node->line);
            emit(c, OP_CALL, 1, node->line);
            emit(c, OP_LT, 0, node->line);
            int jmp_end = emit_jump(c, OP_JUMP_IF_FALSE, node->line);

            emit(c, OP_LOAD_LOCAL, collection_slot, node->line);
            emit(c, OP_LOAD_LOCAL, index_slot, node->line);
            emit(c, OP_INDEX_GET, 0, node->line);
            emit(c, OP_STORE_LOCAL, item_slot, node->line);
            emit(c, OP_POP, 0, node->line);

            LoopContext *loop = loop_push(c, c->scope_depth);
            compile_node(c, node->as.for_each.body);

            int continue_target = c->chunk->count;
            loop_patch_continues(c, loop, continue_target);

            emit(c, OP_LOAD_LOCAL, index_slot, node->line);
            int one = add_int_const(c, 1);
            emit(c, OP_LOAD_INT, one, node->line);
            emit(c, OP_ADD, 0, node->line);
            emit(c, OP_STORE_LOCAL, index_slot, node->line);
            emit(c, OP_POP, 0, node->line);
            emit(c, OP_JUMP, loop_start, node->line);

            int exit_target = c->chunk->count;
            patch_jump_to(c, jmp_end, exit_target);
            loop_patch_breaks(c, loop, exit_target);
            loop_pop(c);

            end_scope(c, node->line);
            break;
        }

        case AST_BREAK: {
            LoopContext *loop = loop_current(c);
            if (!loop) {
                fprintf(stderr,
                        "[LEVIATA][Compilador] 'pare' só pode ser usado dentro de um laço\n");
                c->had_error = 1;
                break;
            }
            emit_unwind_to_depth(c, loop->scope_depth, node->line);
            loop_add_break(c, loop, emit_jump(c, OP_JUMP, node->line));
            break;
        }

        case AST_CONTINUE: {
            LoopContext *loop = loop_current(c);
            if (!loop) {
                fprintf(stderr,
                        "[LEVIATA][Compilador] 'continue' só pode ser usado dentro de um laço\n");
                c->had_error = 1;
                break;
            }
            emit_unwind_to_depth(c, loop->scope_depth, node->line);
            loop_add_continue(c, loop, emit_jump(c, OP_JUMP, node->line));
            break;
        }

        case AST_ASSIGN:
        case AST_COMPOUND_ASSIGN:
        case AST_CALL:
        case AST_METHOD_CALL:
            compile_expr(c, node);
            emit(c, OP_POP, 0, node->line);
            break;

        default:
            compile_expr(c, node);
            emit(c, OP_POP, 0, node->line);
            break;
    }
}

/* ============================================================
 * API pública
 * ============================================================ */
void compiler_init(Compiler *c, LvVM *vm, Compiler *enclosing) {
    memset(c, 0, sizeof(Compiler));
    c->vm = vm;
    c->enclosing = enclosing;
}

void compiler_free(Compiler *c) {
    (void)c;
}

int compiler_compile_program(Compiler *c, AstNode *program, Chunk *out) {
    c->chunk = out;
    chunk_init(out);
    compile_node(c, program);
    emit(c, OP_HALT, 0, 0);
    return !c->had_error;
}