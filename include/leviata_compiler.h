/* ============================================================================
 * MOTOR LEVIATA - COMPILADOR (AST -> BYTECODE)
 * ============================================================================ */
#ifndef LEVIATA_COMPILER_H
#define LEVIATA_COMPILER_H

#include "leviata_ast.h"
#include "leviata_vm.h"
#include "leviata_lexer.h"

/* ---- Escopo de Variável Local ---- */
typedef struct {
    char name[LV_MAX_IDENTIFIER];
    int  depth;
    int  slot;
} LocalVar;

/* ---- Estado do Compilador ---- */
typedef struct Compiler {
    Chunk       *chunk;       /* chunk sendo gerado */
    LocalVar     locals[LV_LOCAL_MAX];
    int          local_count;
    int          scope_depth;
    struct Compiler *enclosing;
    LvVM        *vm;
    int          had_error;
} Compiler;

/* ---- API do Compilador ---- */
void  compiler_init(Compiler *c, LvVM *vm, Compiler *enclosing);
void  compiler_free(Compiler *c);
int   compiler_compile_program(Compiler *c, AstNode *program, Chunk *out);

#endif /* LEVIATA_COMPILER_H */
