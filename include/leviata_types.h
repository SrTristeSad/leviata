/* ============================================================================
 * MOTOR LEVIATA - TIPOS FUNDAMENTAIS
 * ============================================================================ */
#ifndef LEVIATA_TYPES_H
#define LEVIATA_TYPES_H

#include <stdint.h>
#include <stddef.h>

#define LV_VERSION_MAJOR  2
#define LV_VERSION_MINOR  3
#define LV_VERSION_PATCH  1

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

struct LvObject;
struct LvVM;
struct Chunk;

typedef struct LvObject LvObject;
typedef struct LvVM LvVM;
typedef LvObject* (*LvNativeFn)(LvVM *vm, int argc, LvObject **argv);

struct LvObject {
    LvType type;
    int gc_marked;
    struct LvObject *gc_next;

    union {
        int64_t  i_val;
        double   f_val;

        struct {
            char   *data;
            size_t  length;
        } string;

        struct {
            struct LvObject **items;
            int count;
            int capacity;
        } list;

        struct {
            struct LvMapEntry *buckets;
            int count;
            int capacity;
        } map;

        struct {
            char name[LV_MAX_IDENTIFIER];
            struct Chunk *chunk;
            int arity;
            int upvalue_count;
        } function;

        struct {
            char name[LV_MAX_IDENTIFIER];
            LvNativeFn fn;
            int arity;
        } native;
    } as;
};

typedef struct LvMapEntry {
    char key[LV_MAX_IDENTIFIER];
    LvObject *value;
    int occupied;
} LvMapEntry;

#endif /* LEVIATA_TYPES_H */
