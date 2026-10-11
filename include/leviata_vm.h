/* ============================================================================
 * MOTOR LEVIATA - MÁQUINA VIRTUAL (VM + GC + HASHMAP)
 * ============================================================================ */
#ifndef LEVIATA_VM_H
#define LEVIATA_VM_H

#include "leviata_types.h"
#include "leviata_ast.h"

/* ============================================================
 * BYTECODE - Instrução da VM
 * ============================================================ */
typedef enum {
    OP_NOP = 0,

    /* Carregar constantes */
    OP_LOAD_NULL,
    OP_LOAD_TRUE,
    OP_LOAD_FALSE,
    OP_LOAD_INT,
    OP_LOAD_FLOAT,
    OP_LOAD_STRING,

    /* Variáveis */
    OP_LOAD_GLOBAL,
    OP_STORE_GLOBAL,
    OP_LOAD_LOCAL,
    OP_STORE_LOCAL,

    /* Aritméticos */
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POW,
    OP_NEG,

    /* Comparação */
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE,

    /* Lógica */
    OP_AND,
    OP_OR,
    OP_NOT,

    /* String */
    OP_STR_CONCAT,

    /* Listas */
    OP_NEW_LIST,
    OP_INDEX_GET,
    OP_INDEX_SET,
    OP_LIST_PUSH,

    /* Controle de fluxo */
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,

    /* Funções */
    OP_CALL,
    OP_RETURN,
    OP_LOAD_FUNC,

    /* Stack */
    OP_POP,
    OP_DUP,

    /* Especial */
    OP_HALT,
    OP_PRINT,
    OP_PRINTLN,

    OP_COUNT
} OpCode;

typedef struct {
    uint8_t opcode;
    int32_t operand;
} Instruction;

typedef struct {
    LvType type;
    union {
        int64_t i_val;
        double f_val;
        char s_val[LV_MAX_STRING];
    } as;
} ConstValue;

typedef struct Chunk {
    Instruction *code;
    int count;
    int capacity;
    ConstValue *constants;
    int const_count;
    int const_capacity;
    int *lines;
    int arity;
} Chunk;

/* ============================================================
 * HASHMAP - Tabela Hash de Strings para Objetos
 * ============================================================ */
typedef struct HMapEntry {
    char key[LV_MAX_IDENTIFIER];
    LvObject *value;
    int in_use;
} HMapEntry;

typedef struct {
    HMapEntry *entries;
    int capacity;
    int count;
} HashMap;

void hmap_init(HashMap *m, int capacity);
void hmap_free(HashMap *m);
void hmap_set(HashMap *m, const char *key, LvObject *value);
LvObject *hmap_get(HashMap *m, const char *key);
int hmap_has(HashMap *m, const char *key);
void hmap_delete(HashMap *m, const char *key);

/* ============================================================
 * CALL FRAME
 * ============================================================ */
typedef struct {
    Chunk *chunk;
    int ip;
    int base;
    char func_name[LV_MAX_IDENTIFIER];
} CallFrame;

/* ============================================================
 * GARBAGE COLLECTOR
 * ============================================================ */
typedef struct {
    LvObject *all_objects;
    int object_count;
    int gc_threshold;
    size_t bytes_allocated;
} GarbageCollector;

/* ============================================================
 * MÁQUINA VIRTUAL
 * ============================================================ */
struct LvVM {
    LvObject *stack[LV_STACK_MAX];
    int stack_top;

    CallFrame frames[LV_CALL_DEPTH_MAX];
    int frame_count;

    HashMap globals;

    Chunk *functions[LV_GLOBAL_MAX];
    char func_names[LV_GLOBAL_MAX][LV_MAX_IDENTIFIER];
    int func_count;

    GarbageCollector gc;

    LvObject *nil_singleton;
    LvObject *true_singleton;
    LvObject *false_singleton;

    int had_error;
    int running;
};

/* ---- API da VM ---- */
void vm_init(LvVM *vm);
void vm_free(LvVM *vm);
void vm_register_native(LvVM *vm, const char *name, LvNativeFn fn, int arity);
int vm_run_chunk(LvVM *vm, Chunk *main_chunk);

/* ---- API do GC ---- */
LvObject *gc_alloc_object(LvVM *vm, LvType type);
LvObject *gc_alloc_int(LvVM *vm, int64_t value);
LvObject *gc_alloc_float(LvVM *vm, double value);
LvObject *gc_alloc_bool(LvVM *vm, int value);
LvObject *gc_alloc_string(LvVM *vm, const char *str, size_t len);
LvObject *gc_alloc_list(LvVM *vm);
LvObject *gc_alloc_function(LvVM *vm, Chunk *chunk, const char *name, int arity);
void gc_collect(LvVM *vm);
void gc_mark_object(LvObject *obj);
void gc_sweep(LvVM *vm);

/* ---- API do Chunk ---- */
void chunk_init(Chunk *c);
void chunk_free(Chunk *c);
void chunk_write(Chunk *c, uint8_t opcode, int32_t operand, int line);
int chunk_add_constant(Chunk *c, ConstValue cv);

/* ---- Stack helpers ---- */
void vm_push(LvVM *vm, LvObject *obj);
LvObject *vm_pop(LvVM *vm);
LvObject *vm_peek(LvVM *vm, int distance);

#endif /* LEVIATA_VM_H */