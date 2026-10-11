/* ============================================================================
 * LEVIATA 3 - COMPILADOR (AST -> BYTECODE)
 * ============================================================================ */
#ifndef LEVIATA_COMPILER_H
#define LEVIATA_COMPILER_H

#include "leviata_ast.h"
#include "leviata_vm.h"
#include "leviata_lexer.h"

#define LV_LOOP_MAX 64
#define LV_LOOP_JUMP_MAX 256

typedef struct {
    char name[LV_MAX_IDENTIFIER];
    int depth;
    int slot;
} LocalVar;

typedef struct {
    int break_jumps[LV_LOOP_JUMP_MAX];
    int break_count;
    int continue_jumps[LV_LOOP_JUMP_MAX];
    int continue_count;
    int scope_depth;
} LoopContext;

typedef struct Compiler {
    Chunk *chunk;
    LocalVar locals[LV_LOCAL_MAX];
    int local_count;
    int scope_depth;

    LoopContext loops[LV_LOOP_MAX];
    int loop_count;

    struct Compiler *enclosing;
    LvVM *vm;
    int had_error;
} Compiler;

void compiler_init(Compiler *c, LvVM *vm, Compiler *enclosing);
void compiler_free(Compiler *c);
int compiler_compile_program(Compiler *c, AstNode *program, Chunk *out);

#endif /* LEVIATA_COMPILER_H */
