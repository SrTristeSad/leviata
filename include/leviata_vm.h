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
    OP_LOAD_INT,       /* operand = index na const pool */
    OP_LOAD_FLOAT,     /* operand = index na const pool */
    OP_LOAD_STRING,    /* operand = index na const pool */

    /* Variáveis */
    OP_LOAD_GLOBAL,    /* operand = slot global */
    OP_STORE_GLOBAL,
    OP_LOAD_LOCAL,     /* operand = slot local */
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
    OP_NEW_LIST,       /* operand = número de itens no stack */
    OP_INDEX_GET,
    OP_INDEX_SET,
    OP_LIST_PUSH,

    /* Controle de fluxo */
    OP_JUMP,           /* operand = offset absoluto */
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,

    /* Funções */
    OP_CALL,           /* operand = argc */
    OP_RETURN,
    OP_LOAD_FUNC,      /* operand = index de função compilada */

    /* Stack */
    OP_POP,
    OP_DUP,

    /* Especial */
    OP_HALT,
    OP_PRINT,         /* instrução de debug rápido */
    OP_PRINTLN,

    OP_COUNT
} OpCode;

/* ---- Instrução Empacotada ---- */
typedef struct {
    uint8_t  opcode;
    int32_t  operand;  /* uso varia por instrução */
} Instruction;

/* ---- Constante em tempo de compilação ---- */
typedef struct {
    LvType  type;
    union {
        int64_t i_val;
        double  f_val;
        char    s_val[LV_MAX_STRING];
    } as;
} ConstValue;

/* ---- Chunk de Bytecode ---- */
typedef struct Chunk {          /* tag 'Chunk' permite forward-decl em leviata_types.h */
    Instruction *code;
    int          count;
    int          capacity;
    ConstValue  *constants;
    int          const_count;
    int          const_capacity;
    int         *lines;  /* linha fonte para cada instrução */
    int          arity;  /* parâmetros esperados (0 para chunk principal) */
} Chunk;

/* ============================================================
 * HASHMAP - Tabela Hash de Strings para Objetos
 * ============================================================ */
typedef struct HMapEntry {
    char      key[LV_MAX_IDENTIFIER];
    LvObject *value;
    int       in_use;
} HMapEntry;

typedef struct {
    HMapEntry *entries;
    int        capacity;
    int        count;
} HashMap;

void      hmap_init(HashMap *m, int capacity);
void      hmap_free(HashMap *m);
void      hmap_set(HashMap *m, const char *key, LvObject *value);
LvObject *hmap_get(HashMap *m, const char *key);
int       hmap_has(HashMap *m, const char *key);
void      hmap_delete(HashMap *m, const char *key);

/* ============================================================
 * CALL FRAME - Quadro de Chamada de Função
 * ============================================================ */
typedef struct {
    Chunk  *chunk;
    int     ip;         /* instruction pointer */
    int     base;       /* base do stack de valores desta chamada */
    char    func_name[LV_MAX_IDENTIFIER];
} CallFrame;

/* ============================================================
 * GARBAGE COLLECTOR - Mark-and-Sweep
 * ============================================================ */
typedef struct {
    LvObject *all_objects;   /* lista encadeada de todos objetos */
    int       object_count;
    int       gc_threshold;
    size_t    bytes_allocated;
} GarbageCollector;

/* ============================================================
 * MÁQUINA VIRTUAL
 * ============================================================ */
struct LvVM {
    /* Stack de valores */
    LvObject  *stack[LV_STACK_MAX];
    int        stack_top;

    /* Call stack */
    CallFrame  frames[LV_CALL_DEPTH_MAX];
    int        frame_count;

    /* Variáveis globais (HashTable para O(1) lookup) */
    HashMap    globals;

    /* Funções compiladas */
    Chunk     *functions[LV_GLOBAL_MAX];
    char       func_names[LV_GLOBAL_MAX][LV_MAX_IDENTIFIER];
    int        func_count;

    /* Garbage Collector */
    GarbageCollector gc;

    /* Constantes globais pré-interned */
    LvObject  *nil_singleton;
    LvObject  *true_singleton;
    LvObject  *false_singleton;

    /* Flags de controle */
    int        had_error;
    int        running;
};

/* ---- API da VM ---- */
void     vm_init(LvVM *vm);
void     vm_free(LvVM *vm);
void     vm_register_native(LvVM *vm, const char *name, LvNativeFn fn, int arity);
int      vm_run_chunk(LvVM *vm, Chunk *main_chunk);

/* ---- API do GC ---- */
LvObject *gc_alloc_object(LvVM *vm, LvType type);
LvObject *gc_alloc_int(LvVM *vm, int64_t value);
LvObject *gc_alloc_float(LvVM *vm, double value);
LvObject *gc_alloc_bool(LvVM *vm, int value);
LvObject *gc_alloc_string(LvVM *vm, const char *str, size_t len);
LvObject *gc_alloc_list(LvVM *vm);
LvObject *gc_alloc_function(LvVM *vm, Chunk *chunk, const char *name, int arity);
void      gc_collect(LvVM *vm);
void      gc_mark_object(LvObject *obj);
void      gc_sweep(LvVM *vm);

/* ---- API do Chunk ---- */
void chunk_init(Chunk *c);
void chunk_free(Chunk *c);
void chunk_write(Chunk *c, uint8_t opcode, int32_t operand, int line);
int  chunk_add_constant(Chunk *c, ConstValue cv);

/* ---- Stack helpers ---- */
void      vm_push(LvVM *vm, LvObject *obj);
LvObject *vm_pop(LvVM *vm);
LvObject *vm_peek(LvVM *vm, int distance);

#endif /* LEVIATA_VM_H */
