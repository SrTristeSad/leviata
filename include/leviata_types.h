/* ============================================================================
 * MOTOR LEVIATA - TIPOS FUNDAMENTAIS
 * ============================================================================ */
#ifndef LEVIATA_TYPES_H
#define LEVIATA_TYPES_H

#include <stdint.h>
#include <stddef.h>

/* ---- Configurações do Sistema ---- */
#define LV_VERSION_MAJOR  2
#define LV_VERSION_MINOR  0
#define LV_VERSION_PATCH  0

#define LV_MAX_IDENTIFIER  128
#define LV_MAX_STRING      4096
#define LV_HASHMAP_SIZE    4096
#define LV_GC_THRESHOLD    256
#define LV_STACK_MAX       1024
#define LV_CALL_DEPTH_MAX  256
#define LV_CONST_POOL_MAX  65536
#define LV_CODE_MAX        65536
#define LV_GLOBAL_MAX      4096
#define LV_LOCAL_MAX       256

/* ---- Tipos de Dado em Runtime ---- */
typedef enum {
    LV_TYPE_NULL = 0,
    LV_TYPE_BOOL,
    LV_TYPE_INT,
    LV_TYPE_FLOAT,
    LV_TYPE_STRING,
    LV_TYPE_LIST,
    LV_TYPE_MAP,
    LV_TYPE_FUNCTION,
    LV_TYPE_NATIVE,
    LV_TYPE_OBJECT
} LvType;

/* Forward declarations */
struct LvObject;
struct LvVM;
struct Chunk;          /* definido em leviata_vm.h — evita inclusão circular */

typedef struct LvObject LvObject;
typedef struct LvVM LvVM;

/* Ponteiro para função nativa (FFI/Binding) */
typedef LvObject* (*LvNativeFn)(LvVM *vm, int argc, LvObject **argv);

/* ---- Objeto Base do GC ---- */
struct LvObject {
    LvType type;
    int gc_marked;          /* Mark-and-Sweep flag */
    struct LvObject *gc_next;  /* Lista encadeada de todos objetos alocados */

    union {
        int64_t  i_val;     /* LV_TYPE_INT / LV_TYPE_BOOL */
        double   f_val;     /* LV_TYPE_FLOAT */

        struct {            /* LV_TYPE_STRING */
            char   *data;
            size_t  length;
        } string;

        struct {            /* LV_TYPE_LIST */
            struct LvObject **items;
            int count;
            int capacity;
        } list;

        struct {            /* LV_TYPE_MAP */
            struct LvMapEntry *buckets;
            int count;
            int capacity;
        } map;

        struct {            /* LV_TYPE_FUNCTION */
            char          name[LV_MAX_IDENTIFIER];
            struct Chunk *chunk;       /* bytecode compilado (propriedade de vm->functions[]) */
            int           arity;       /* número de parâmetros */
            int           upvalue_count;
        } function;

        struct {            /* LV_TYPE_NATIVE */
            char name[LV_MAX_IDENTIFIER];
            LvNativeFn fn;
            int arity;          /* -1 = variadic */
        } native;
    } as;
};

/* Entrada do HashMap */
typedef struct LvMapEntry {
    char  key[LV_MAX_IDENTIFIER];
    LvObject *value;
    int occupied;
} LvMapEntry;

#endif /* LEVIATA_TYPES_H */
