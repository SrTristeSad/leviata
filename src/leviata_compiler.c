/* ============================================================================
 * MOTOR LEVIATA - IMPLEMENTAÇÃO DO COMPILADOR (AST -> BYTECODE)
 * ============================================================================ */
#include "leviata_compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Helpers de emissão de bytecode ---- */
static void emit(Compiler *c, uint8_t op, int32_t operand, int line) {
    chunk_write(c->chunk, op, operand, line);
}

static int emit_jump(Compiler *c, uint8_t op, int line) {
    emit(c, op, 0xFFFF, line);
    return c->chunk->count - 1;
}

static void patch_jump(Compiler *c, int idx) {
    c->chunk->code[idx].operand = c->chunk->count;
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

/* ---- Gerenciamento de escopo local ---- */
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

    /* Impede duas declarações com o mesmo nome no mesmo bloco,
     * mas continua permitindo shadowing em blocos aninhados. */
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

static void compile_node(Compiler *c, AstNode *node);
static void compile_expr(Compiler *c, AstNode *node);

/* ============================================================
 * EXPRESSÕES
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
            if (slot >= 0) {
                emit(c, OP_LOAD_LOCAL, slot, node->line);
            } else {
                int idx = add_string_const(c, node->as.ident.name);
                emit(c, OP_LOAD_GLOBAL, idx, node->line);
            }
            break;
        }

        case AST_BINOP: {
            /*
             * OP_JUMP_IF_FALSE/TRUE consome a condição na VM. Portanto o
             * short-circuit precisa reconstruir um booleano no caminho que
             * não avalia o operando da direita.
             */
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
                    fprintf(stderr, "[Compilador] Operador binário desconhecido\n");
                    c->had_error = 1;
                    break;
            }
            break;
        }

        case AST_UNOP:
            compile_expr(c, node->as.unop.operand);
            switch (node->as.unop.op) {
                case TOK_MINUS: emit(c, OP_NEG, 0, node->line); break;
                case TOK_NOT:   emit(c, OP_NOT, 0, node->line); break;
                default:
                    fprintf(stderr, "[Compilador] Operador unário desconhecido\n");
                    c->had_error = 1;
                    break;
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
                /* OP_INDEX_SET espera stack: objeto, índice, valor. */
                compile_expr(c, target->as.index_access.object);
                compile_expr(c, target->as.index_access.index);
                compile_expr(c, node->as.assign.value);
                emit(c, OP_INDEX_SET, 0, node->line);
            } else {
                fprintf(stderr, "[Compilador] Alvo de atribuição inválido\n");
                c->had_error = 1;
            }
            break;
        }

        case AST_CALL:
            compile_expr(c, node->as.call.callee);
            for (int i = 0; i < node->as.call.argc; i++)
                compile_expr(c, node->as.call.args[i]);
            emit(c, OP_CALL, node->as.call.argc, node->line);
            break;

        case AST_INDEX:
            compile_expr(c, node->as.index_access.object);
            compile_expr(c, node->as.index_access.index);
            emit(c, OP_INDEX_GET, 0, node->line);
            break;

        case AST_LIST_LIT:
            /*
             * Cria a lista vazia e adiciona cada item na ordem da fonte.
             * Além de preservar [a,b,c] como [a,b,c], isso preserva também
             * a ordem de avaliação de expressões com efeitos colaterais.
             */
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
 * STATEMENTS
 * ============================================================ */
static void compile_node(Compiler *c, AstNode *node) {
    if (!node || c->had_error) return;

    switch (node->type) {
        case AST_PROGRAM:
        case AST_BLOCK: {
            int is_block = (node->type == AST_BLOCK);
            if (is_block) begin_scope(c);
            for (int i = 0; i < node->as.block.count; i++)
                compile_node(c, node->as.block.stmts[i]);
            if (is_block) end_scope(c, node->line);
            break;
        }

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

        case AST_FUNC_DECL: {
            const char *fn_name = node->as.func_decl.name;
            int fn_arity = node->as.func_decl.param_count;
            LvVM *vm = c->vm;

            if (vm->func_count >= LV_GLOBAL_MAX) {
                fprintf(stderr, "[Compilador] Limite de funções compiladas atingido\n");
                c->had_error = 1;
                break;
            }

            Chunk *fn_chunk = (Chunk*)malloc(sizeof(Chunk));
            if (!fn_chunk) {
                fprintf(stderr,
                        "[Compilador] Sem memória para compilar função '%s'\n",
                        fn_name);
                c->had_error = 1;
                break;
            }
            chunk_init(fn_chunk);
            fn_chunk->arity = fn_arity;

            Compiler fn_c;
            compiler_init(&fn_c, vm, c);
            fn_c.chunk = fn_chunk;
            fn_c.scope_depth = 0;

            for (int i = 0; i < fn_arity; i++)
                declare_local(&fn_c, node->as.func_decl.params[i]);

            compile_node(&fn_c, node->as.func_decl.body);

            if (fn_c.had_error) {
                c->had_error = 1;
                chunk_free(fn_chunk);
                free(fn_chunk);
                break;
            }

            emit(&fn_c, OP_LOAD_NULL, 0, node->line);
            emit(&fn_c, OP_RETURN, 0, node->line);

            int func_idx = vm->func_count;
            vm->functions[func_idx] = fn_chunk;
            strncpy(vm->func_names[func_idx], fn_name, LV_MAX_IDENTIFIER - 1);
            vm->func_names[func_idx][LV_MAX_IDENTIFIER - 1] = '\0';
            vm->func_count++;

            int name_idx = add_string_const(c, fn_name);
            emit(c, OP_LOAD_FUNC, func_idx, node->line);
            emit(c, OP_STORE_GLOBAL, name_idx, node->line);
            emit(c, OP_POP, 0, node->line);
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
            compile_node(c, node->as.while_stmt.body);
            emit(c, OP_JUMP, loop_start, node->line);
            patch_jump(c, jmp_end);
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

            compile_node(c, node->as.for_range.body);

            emit(c, OP_LOAD_LOCAL, var_slot, node->line);
            int one = add_int_const(c, 1);
            emit(c, OP_LOAD_INT, one, node->line);
            emit(c, OP_ADD, 0, node->line);
            emit(c, OP_STORE_LOCAL, var_slot, node->line);
            emit(c, OP_POP, 0, node->line);

            emit(c, OP_JUMP, loop_start, node->line);
            patch_jump(c, jmp_end);

            end_scope(c, node->line);
            break;
        }

        case AST_ASSIGN:
        case AST_COMPOUND_ASSIGN:
        case AST_CALL:
        case AST_METHOD_CALL:
            compile_expr(c, node);
            emit(c, OP_POP, 0, node->line);
            break;

        case AST_BREAK:
        case AST_CONTINUE:
            fprintf(stderr,
                    "[Aviso] quebrar/continuar ainda não implementado no compilador\n");
            break;

        default:
            compile_expr(c, node);
            emit(c, OP_POP, 0, node->line);
            break;
    }
}

/* ============================================================
 * API PÚBLICA
 * ============================================================ */
void compiler_init(Compiler *c, LvVM *vm, Compiler *enclosing) {
    memset(c, 0, sizeof(Compiler));
    c->vm = vm;
    c->enclosing = enclosing;
    c->scope_depth = 0;
    c->local_count = 0;
    c->had_error = 0;
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
